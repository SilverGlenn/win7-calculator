#!/bin/sh
# Self-contained: the installed copy needs only Python 3, not the source tree.
exec python3 - "$@" <<'PY'
import argparse
import hashlib
import json
import os
from pathlib import Path
import sys

PAYLOAD = {
    'lib/win7-calculator/win7-calculator',
    'lib/win7-calculator/LICENSE', 'lib/win7-calculator/NOTICE',
    'lib/win7-calculator/README', 'lib/win7-calculator/uninstall-user.sh',
    'bin/win7-calculator',
    'share/applications/org.win7calculator.Calculator.desktop',
}
MANIFEST = 'lib/win7-calculator/INSTALL.json'

def safe_path(path):
    for part in (path, *path.parents):
        if part.is_symlink():
            raise ValueError('symlink in installation path: ' + str(part))

def main():
    parser = argparse.ArgumentParser(description='Remove only verified calculator payload')
    parser.add_argument('--prefix', default=str(Path.home() / '.local'))
    parser.add_argument('--check', action='store_true', help='validate ownership without removing files')
    args = parser.parse_args()
    prefix = Path(os.path.abspath(args.prefix))
    safe_path(prefix)
    manifest = prefix / MANIFEST
    safe_path(manifest)
    if not manifest.exists():
        return
    data = json.loads(manifest.read_text())
    if (data.get('version') != 1 or data.get('prefix') != str(prefix)
            or not isinstance(data.get('files'), dict)
            or set(data['files']) != PAYLOAD | {MANIFEST}
            or data['files'][MANIFEST] is not None):
        raise ValueError('unsafe or foreign installation manifest')
    # Validate the entire manifest and every extant file BEFORE deleting anything.
    for relative in PAYLOAD:
        path = prefix / relative
        safe_path(path)
        digest = data['files'][relative]
        if not isinstance(digest, str) or len(digest) != 64:
            raise ValueError('invalid payload checksum')
        if path.exists() and (not path.is_file() or
                hashlib.sha256(path.read_bytes()).hexdigest() != digest):
            raise ValueError('modified/unowned payload: ' + str(path))
    if args.check:
        return
    for relative in sorted(PAYLOAD):
        (prefix / relative).unlink(missing_ok=True)
    manifest.unlink()
    # Never recurse: unrelated files, directories and user state survive.
    for relative in ['lib/win7-calculator', 'share/applications', 'bin', 'lib', 'share']:
        try:
            (prefix / relative).rmdir()
        except OSError:
            pass

try:
    main()
except (OSError, ValueError, TypeError) as error:
    print('uninstall-user: ' + str(error), file=sys.stderr)
    sys.exit(1)
PY
