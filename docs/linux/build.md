# Native Linux calculator development

This is an in-progress native Linux target, separate from the upstream Windows UWP solution. It is **not yet the completed Windows 7 reproduction**: disabled legacy actions identify work still in progress; Modern is intentionally reserved.

## Build and test

Requires C++20, CMake 3.25+, Ninja, Python3, and Qt6 Core/Gui/Widgets/Test development packages (Qt 6.4+; validated host Qt 6.10.2).

```sh
cmake -S . -B build/linux -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/linux
QT_QPA_PLATFORM=offscreen ctest --test-dir build/linux --output-on-failure
./build/linux/src/LinuxCalculator/win7-calculator
```

The application uses a real system-decorated window and the installed Qt style/icon theme. With AeroThemePlasma installed, KWin/SMOD owns the Aero frame. No desktop settings are changed. Segoe UI and Consolas are requested when available; fallback fonts may differ from the Windows reference and are not evidence of pixel-perfect fidelity.

The engine target is Qt-free; only its test harness and the frontend link Qt. The retained library has process-global numeric constants, so arithmetic operations must be serialized on their owning application thread.

## Reversible user-local preview packaging

Standard, Scientific, Programmer and Statistics are available. History, Units,
Dates and Worksheets remain disabled. Font matching and final visual fidelity
remain pending; this tooling is not full feature/fidelity acceptance.

From any working directory, `scripts/linux/run.sh [arguments...]` starts the
repository's `build/linux/src/LinuxCalculator/win7-calculator` and forwards all
arguments. Set `WIN7CALC_BUILD_DIR` to another build directory; relative overrides
are resolved relative to the repository, not the caller's working directory.
The runner does not build or install anything.

```sh
./scripts/linux/install-user.sh --build-dir build/linux
# Or choose a separate prefix (spaces and shell metacharacters are supported):
./scripts/linux/install-user.sh --build-dir build/linux --prefix "$HOME/preview calculator"
# CMake installation uses the same checks and manifest:
cmake --install build/linux --prefix "$HOME/preview calculator"
./scripts/linux/uninstall-user.sh --prefix "$HOME/preview calculator"
```

The default prefix is `$HOME/.local`. No `sudo`, desktop-cache refresh, default
application assignment, theme change or restart is performed. Do not run these
commands against an existing manual installation without inspecting its record:
legacy/manual or modified payload is a conflict, not silently adopted. This
work does not itself update the installed desktop preview.

The layout is `PREFIX/lib/win7-calculator/win7-calculator` (a real copied ELF),
`PREFIX/bin/win7-calculator` (argument-forwarding launcher), and
`PREFIX/share/applications/org.win7calculator.Calculator.desktop`. The lib
directory also contains `LICENSE`, `NOTICE`, `README`, a self-contained
`uninstall-user.sh`, and `INSTALL.json`. The desktop entry has absolute escaped
launcher paths, semantic `accessories-calculator` icon, and the unchanged
`org.win7calculator.Calculator` identity. All runtime resources are embedded;
no source directory, build directory or AeroThemePlasma checkout is needed at
runtime. System Qt 6 Core/Gui/Widgets libraries and a suitable platform plugin
are runtime dependencies, not bundled. Python 3 is required for install/removal.

The versioned manifest enumerates the exact owned relative paths, SHA-256
checksums, and absolute prefix (its own entry has a null checksum). Install and
removal reject foreign prefixes, extra/traversal paths, symlinked parents,
conflicts and modified payload before changes. Missing owned files are tolerated
for repair/idempotent removal. Uninstall never recursively deletes a directory;
unrelated files and user state survive. Each file is staged in its destination
directory and atomically renamed, so running executables are never truncated or
stopped. A running calculator continues using its old binary until normal
close/reopen. This is per-file atomicity, not a crash-safe multi-file transaction;
an interrupted update may require inspection before another update/removal.
Do not concurrently install/remove the same prefix. `DESTDIR` staging is supported
by CMake, but absolute launch paths bind to that staged prefix: install again
with the intended final prefix rather than relocating a staged tree.

## Isolated packaging verification

```sh
python3 -m unittest discover -s tests/linux -p 'install_test.py' -v
WIN7CALC_TEST_BUILD_DIR="$(pwd)/build/linux" python3 -m unittest discover -s tests/linux -p 'install_test.py' -v
```

The default command configures/builds the application once in a temporary
build with a temporary HOME; the absolute override reuses an existing built
artifact. Every install, upgrade and removal uses a temporary HOME/prefix.
The tests launch the installed application offscreen from an unrelated cwd,
terminate only that test-owned process, and verify a live process survives an
upgrade. `desktop-file-validate` is run when available. CTest registers the same
suite with its current build directory, avoiding a nested build. CI builds the
native target, runs packaging and native tests, and checks whitespace separately
from the unchanged Windows workflow.
