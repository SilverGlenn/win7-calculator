"""Real temporary-HOME packaging checks; no desktop caches or live installation.

These catch missing payload, unsafe ownership/removal, non-atomic updates and
source-dependent launchers. WIN7CALC_TEST_BUILD_DIR may reuse an absolute build.
"""
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
import unittest

ROOT = Path(__file__).resolve().parents[2]
INSTALL = ROOT / 'scripts/linux/install-user.sh'
UNINSTALL = ROOT / 'scripts/linux/uninstall-user.sh'


class InstallTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # Missing tools must be an assertion failure before spending time building.
        if not INSTALL.is_file() or not UNINSTALL.is_file():
            return
        cls.build_temp = tempfile.TemporaryDirectory(prefix='win7calc-build-')
        override = os.environ.get('WIN7CALC_TEST_BUILD_DIR')
        if override:
            if not Path(override).is_absolute():
                raise AssertionError('WIN7CALC_TEST_BUILD_DIR must be absolute')
            cls.build = Path(override)
        else:
            cls.build = Path(cls.build_temp.name) / 'build'
            build_home = Path(cls.build_temp.name) / 'home'
            build_home.mkdir()
            build_env = dict(os.environ, HOME=str(build_home),
                             XDG_CONFIG_HOME=str(build_home / '.config'),
                             XDG_CACHE_HOME=str(build_home / '.cache'),
                             XDG_DATA_HOME=str(build_home / '.local/share'))
            subprocess.run(['cmake', '-S', str(ROOT), '-B', str(cls.build), '-G', 'Ninja',
                            '-DCMAKE_BUILD_TYPE=Release', '-DBUILD_TESTING=OFF'],
                           check=True, env=build_env)
            subprocess.run(['cmake', '--build', str(cls.build), '--parallel', '2'],
                           check=True, env=build_env)

    @classmethod
    def tearDownClass(cls):
        if hasattr(cls, 'build_temp'):
            cls.build_temp.cleanup()

    def setUp(self):
        self.assertTrue(INSTALL.is_file() and UNINSTALL.is_file(),
                        'packaging install/uninstall tools are missing')
        self.temp = tempfile.TemporaryDirectory(prefix='win7calc-test-')
        self.addCleanup(self.temp.cleanup)
        self.home = Path(self.temp.name) / 'home'
        self.home.mkdir()
        self.prefix = self.home / 'prefix with spaces $cash `tick` "quote" \\back %percent'
        self.env = dict(os.environ, HOME=str(self.home), QT_QPA_PLATFORM='offscreen',
                        XDG_CONFIG_HOME=str(self.home / '.config'),
                        XDG_CACHE_HOME=str(self.home / '.cache'),
                        XDG_DATA_HOME=str(self.home / '.local/share'),
                        XDG_RUNTIME_DIR=str(self.home / 'runtime'))
        Path(self.env['XDG_RUNTIME_DIR']).mkdir(mode=0o700)

    def tool(self, script, *args, ok=True):
        result = subprocess.run([str(script), *map(str, args)], env=self.env,
                                cwd=self.temp.name, text=True, capture_output=True)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return result

    def install(self):
        self.tool(INSTALL, '--build-dir', self.build, '--prefix', self.prefix)

    def test_payload_upgrade_running_process_and_uninstall(self):
        self.install()
        lib = self.prefix / 'lib/win7-calculator'
        binary = lib / 'win7-calculator'
        self.assertFalse(binary.is_symlink())
        self.assertEqual(binary.read_bytes(), (self.build / 'src/LinuxCalculator/win7-calculator').read_bytes())
        for name in ['LICENSE', 'NOTICE', 'README', 'uninstall-user.sh', 'INSTALL.json']:
            self.assertTrue((lib / name).is_file(), name)
        self.assertIn('MIT', (lib / 'LICENSE').read_text())
        desktop = self.prefix / 'share/applications/org.win7calculator.Calculator.desktop'
        text = desktop.read_text()
        self.assertIn('Icon=accessories-calculator\n', text)
        self.assertIn('StartupWMClass=org.win7calculator.Calculator\n', text)
        self.assertIn('Exec="', text)
        self.assertIn('TryExec=', text)
        entry = dict(line.split('=', 1) for line in text.splitlines() if '=' in line)
        # Desktop Entry string unescaping, then Exec argument/percent unescaping.
        decoded = entry['Exec'].replace('\\\\', '\\')
        parsed = self.tool('/bin/sh', '-c', 'set -- ' + decoded.replace('%%', '%')
                           + '; printf "%s\\n" "$@"')
        self.assertEqual(parsed.stdout.splitlines(),
                         [str(self.prefix / 'bin/win7-calculator')])
        self.assertEqual(entry['TryExec'].replace('\\\\', '\\'),
                         str(self.prefix / 'bin/win7-calculator'))
        if shutil.which('desktop-file-validate'):
            subprocess.run(['desktop-file-validate', str(desktop)], check=True)
        # A real installed process must survive atomic replacement, not ETXTBSY.
        process = subprocess.Popen([str(self.prefix / 'bin/win7-calculator')],
                                   cwd=self.temp.name, env=self.env,
                                   stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        try:
            time.sleep(1)
            self.assertIsNone(process.poll(), 'installed offscreen application exited')
            before = binary.stat().st_ino
            unrelated = lib / 'keep.txt'
            unrelated.write_text('not ours')
            self.install()
            self.assertNotEqual(before, binary.stat().st_ino)
            self.assertIsNone(process.poll())
            self.assertEqual(unrelated.read_text(), 'not ours')
        finally:
            process.terminate()
            process.communicate(timeout=10)
        # Installed uninstaller works without a repository cwd.
        self.tool(lib / 'uninstall-user.sh', '--prefix', self.prefix)
        self.tool(UNINSTALL, '--prefix', self.prefix)
        self.assertFalse(binary.exists())
        self.assertFalse(desktop.exists())
        self.assertEqual(unrelated.read_text(), 'not ours')

    def test_invalid_build_and_conflicts_do_not_mutate(self):
        self.tool(INSTALL, '--build-dir', self.home / 'absent', '--prefix', self.prefix, ok=False)
        self.assertFalse(self.prefix.exists())
        conflict = self.prefix / 'bin/win7-calculator'
        conflict.parent.mkdir(parents=True)
        conflict.write_text('unrelated')
        self.tool(INSTALL, '--build-dir', self.build, '--prefix', self.prefix, ok=False)
        self.assertEqual(conflict.read_text(), 'unrelated')
        self.assertFalse((self.prefix / 'lib/win7-calculator').exists())

    def test_manifest_tampering_rejected_before_removal(self):
        self.install()
        manifest = self.prefix / 'lib/win7-calculator/INSTALL.json'
        original = manifest.read_text()
        sentinel = self.home / 'sentinel'
        sentinel.write_text('safe')
        for path in ['../../sentinel', str(sentinel), 'lib/win7-calculator/../foreign']:
            data = json.loads(original)
            data['files'][path] = hashlib.sha256(b'safe').hexdigest()
            manifest.write_text(json.dumps(data))
            self.tool(UNINSTALL, '--prefix', self.prefix, ok=False)
            self.assertTrue((self.prefix / 'bin/win7-calculator').exists())
            self.assertEqual(sentinel.read_text(), 'safe')
        data = json.loads(original)
        data['prefix'] = str(self.home / 'foreign')
        manifest.write_text(json.dumps(data))
        self.tool(UNINSTALL, '--prefix', self.prefix, ok=False)
        manifest.write_text(original)
        (self.prefix / 'bin/win7-calculator').write_text('modified')
        self.tool(UNINSTALL, '--prefix', self.prefix, ok=False)
        self.assertTrue((self.prefix / 'lib/win7-calculator/win7-calculator').exists())

    def test_symlink_parent_is_not_followed(self):
        target = self.home / 'target'
        target.mkdir()
        self.prefix.mkdir()
        (self.prefix / 'lib').symlink_to(target, target_is_directory=True)
        self.tool(INSTALL, '--build-dir', self.build, '--prefix', self.prefix, ok=False)
        self.assertEqual(list(target.iterdir()), [])

    def test_cmake_install_uses_the_same_owned_layout(self):
        self.tool('cmake', '--install', self.build, '--prefix', self.prefix)
        self.assertTrue((self.prefix / 'lib/win7-calculator/INSTALL.json').is_file())
        self.tool(UNINSTALL, '--prefix', self.prefix)
        self.assertFalse((self.prefix / 'bin/win7-calculator').exists())

    def test_default_prefix_and_development_runner_arguments(self):
        self.tool(INSTALL, '--build-dir', self.build)
        self.assertTrue((self.home / '.local/bin/win7-calculator').exists())
        self.tool(UNINSTALL)
        # A tiny executable in a temporary build observes quoting and argument forwarding.
        build = self.home / 'fake build'
        exe = build / 'src/LinuxCalculator/win7-calculator'
        exe.parent.mkdir(parents=True)
        exe.write_text('#!/bin/sh\nprintf "%s\\n" "$@"\n')
        exe.chmod(0o755)
        self.env['WIN7CALC_BUILD_DIR'] = str(build)
        result = self.tool(ROOT / 'scripts/linux/run.sh', 'one two', '$three')
        self.assertEqual(result.stdout.splitlines(), ['one two', '$three'])


if __name__ == '__main__':
    unittest.main()
