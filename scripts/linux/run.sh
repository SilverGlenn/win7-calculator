#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${WIN7CALC_BUILD_DIR:-"$ROOT/build/linux"}
# Relative overrides are repository-relative, independent of the caller's cwd.
case "$BUILD" in /*) ;; *) BUILD="$ROOT/$BUILD" ;; esac
exec "$BUILD/src/LinuxCalculator/win7-calculator" "$@"
