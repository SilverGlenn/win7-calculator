#!/bin/sh
# Keep all path handling and manifest validation out of shell interpolation.
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd) || exit 1
exec python3 - "$SCRIPT_DIR" "$@" <<'PY'
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import sys
import tempfile

scripts = Path(sys.argv.pop(1))
root = scripts.parents[1]
MANIFEST = 'lib/win7-calculator/INSTALL.json'

def safe_path(path):
    for part in (path, *path.parents):
        if part.is_symlink():
            raise ValueError('symlink in installation path: ' + str(part))

def desktop_value(value):
    return value.replace('\\', '\\\\').replace('\n', '\\n').replace('\r', '\\r').replace('\t', '\\t')

def desktop_exec(value):
    # Exec has a second quoting layer beyond Desktop Entry string escaping.
    quoted = ''.join('\\' + c if c in '\\"`$' else c for c in value)
    return desktop_value('"' + quoted.replace('%', '%%') + '"')

def atomic_write(path, content, mode):
    fd, temporary = tempfile.mkstemp(prefix='.' + path.name + '-', dir=path.parent)
    try:
        with os.fdopen(fd, 'wb') as stream:
            stream.write(content)
            stream.flush()
            os.fsync(stream.fileno())
            os.fchmod(stream.fileno(), mode)
        os.replace(temporary, path)
    finally:
        if os.path.exists(temporary):
            os.unlink(temporary)

def main():
    parser = argparse.ArgumentParser(description='Install the native calculator preview (no desktop settings changed)')
    parser.add_argument('--build-dir', required=True)
    parser.add_argument('--prefix', default=str(Path.home() / '.local'))
    args = parser.parse_args()
    binary = Path(args.build_dir).absolute() / 'src/LinuxCalculator/win7-calculator'
    # Validate before even creating the prefix. Never install a build-tree symlink.
    if not binary.is_file() or not os.access(binary, os.X_OK):
        raise ValueError('built executable missing or not executable: ' + str(binary))
    executable = binary.read_bytes()
    if not executable.startswith(b'\x7fELF'):
        raise ValueError('built executable is not an ELF binary')
    prefix = Path(os.path.abspath(args.prefix))
    if any(ord(c) < 32 or ord(c) == 127 for c in str(prefix)):
        raise ValueError('control characters are not supported in prefixes')
    safe_path(prefix)
    launcher = str(prefix / 'bin/win7-calculator')
    desktop = (root / 'packaging/linux/org.win7calculator.Calculator.desktop.in').read_text()
    substitutions = {'@EXEC@': desktop_exec(launcher), '@TRYEXEC@': desktop_value(launcher)}
    # Match only the original template; token-like text in prefixes is data.
    desktop = re.sub(r'@(?:EXEC|TRYEXEC)@', lambda match: substitutions[match.group()], desktop)
    readme = '''Native Linux Calculator preview
Standard, Scientific, Programmer and Statistics are available. History, Units,
Dates and Worksheets are disabled. Numeric fonts and final fidelity are pending.
Resources are embedded; system Qt 6 Core/Gui/Widgets is required (not bundled).
No desktop settings or application caches were changed.
To remove: run ./uninstall-user.sh --prefix PREFIX from this directory, replacing
PREFIX with the absolute prefix recorded in INSTALL.json (default: $HOME/.local).
Only unchanged manifest-owned files are removed; unrelated files and user state
are preserved. Modified payload or legacy manual installations require explicit
inspection, not forced deletion. Python 3 is required for installation/removal.
Updates replace files atomically without stopping an existing calculator; close
and reopen normally to use an updated binary. This is not a full fidelity release.
'''
    payload = {
        'lib/win7-calculator/win7-calculator': (executable, 0o755),
        'lib/win7-calculator/LICENSE': ((root / 'LICENSE').read_bytes(), 0o644),
        'lib/win7-calculator/NOTICE': ((root / 'NOTICE.txt').read_bytes(), 0o644),
        'lib/win7-calculator/README': (readme.encode(), 0o644),
        'lib/win7-calculator/uninstall-user.sh': ((scripts / 'uninstall-user.sh').read_bytes(), 0o755),
        'bin/win7-calculator': (('#!/bin/sh\nexec ' + shlex.quote(str(prefix / 'lib/win7-calculator/win7-calculator')) + ' "$@"\n').encode(), 0o755),
        'share/applications/org.win7calculator.Calculator.desktop': (desktop.encode(), 0o644),
    }
    manifest = prefix / MANIFEST
    safe_path(manifest)
    previous = set()
    if manifest.exists():
        subprocess.run([str(scripts / 'uninstall-user.sh'), '--prefix', str(prefix), '--check'], check=True)
        previous = set(json.loads(manifest.read_text())['files'])
    # Preflight every conflict before any filesystem mutation.
    for relative in set(payload) | {MANIFEST}:
        path = prefix / relative
        safe_path(path)
        if path.exists() and (relative not in previous or not path.is_file()):
            raise ValueError('refusing to overwrite unowned path: ' + str(path))
        for parent in path.parents:
            if parent.exists() and not parent.is_dir():
                raise ValueError('non-directory installation parent: ' + str(parent))
    checksums = {name: hashlib.sha256(content).hexdigest() for name, (content, _) in payload.items()}
    checksums[MANIFEST] = None  # The manifest owns itself, without a recursive hash.
    record = json.dumps({'version': 1, 'prefix': str(prefix), 'files': checksums}, indent=2, sort_keys=True) + '\n'
    for relative, (content, mode) in payload.items():
        path = prefix / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        atomic_write(path, content, mode)
    atomic_write(manifest, record.encode(), 0o644)
    print('Installed preview at ' + str(prefix))

try:
    main()
except (OSError, ValueError, TypeError, subprocess.CalledProcessError) as error:
    print('install-user: ' + str(error), file=sys.stderr)
    sys.exit(1)
PY
