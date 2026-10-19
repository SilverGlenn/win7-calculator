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
