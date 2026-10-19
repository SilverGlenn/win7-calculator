# Windows 7 Calculator reference captures

Captured directly from the operator's offline Windows 7 VM on 2026-10-19, using `~/VMs/windows7/control.py`. These are reference evidence, not application artwork or mockups. Crops exclude unrelated applications. Source guest resolution was 1440 × 900. Images are unscaled.

The VM uses Windows 7 Basic graphics: its opaque title bar is **not** an Aero Glass reference. Use the installed AeroThemePlasma/SMOD window frame on the Linux desktop, without changing desktop settings.

## Capture index

- `02`: original View menu and keyboard shortcuts.
- `03`: Scientific, normal functions.
- `04`: Programmer, decimal/Qword, bit display and disabled operations.
- `05`: Statistics, with one pre-existing sample.
- `06`: Standard with an empty expanded history area.
- `07`, `16`: unit conversion panel and category selector.
- `08`, `17`, `18`: date difference, mode selector, and add/subtract-date form.
- `09`, `10`, `11`: Edit, Help, and Worksheets menus.
- `12`–`15`: Mortgage, Vehicle lease, Fuel economy (mpg), Fuel economy (L/100 km).
- `19`: Standard, no history, zero display.
- `20`, `21`: numeric button hover and mouse-down states.
- `24`, `26`, `27`: populated history and Edit > History submenu.
- `25`: completed Standard calculation, 12 + 3 = 15.
- `28`: Scientific with Inv selected (inverse function labels).
- `29`: Standard reset, no history.

File numbering follows capture order; some intermediate captures were omitted because they were taken before the guest had finished repainting. The retained files were visually inspected; they are not all independent test cases.

## Measured baseline geometry

Coordinates below refer to the source screenshots, with the window at (125,125); saved crops begin at that point. Integer measurements include both endpoints where stated.

| State | Outer Basic-frame bounds | Outer size |
|---|---|---|
| Standard | x125..352, y125..446 | 228 × 322 |
| Scientific | x125..547, y125..446 | 423 × 322 |
| Programmer | x125..547, y125..512 | 423 × 388 |
| Statistics | x125..352, y125..546 | 228 × 422 |
| Standard + history | x125..352, y125..546 | 228 × 422 |
| Standard + conversion/date/worksheet | x125..726, y125..446 | 602 × 322 |

Standard client area excluding the Basic frame: (133,155) through (344,438), 212 × 284, including the menu bar. Menu bar height is approximately 19 px; verify exact boundaries during implementation. Standard display bounds: x144..333, y185..234 (190 × 50). Key rows begin at y240,272,304,336,368,400; nominal pitch 32 px. Main key columns begin at x144,183,222,261,300; nominal pitch 39 px. Individual widths differ by a pixel due to original dialog-unit rounding. Equals spans the final two rows; zero spans the first two columns.

Match client-area geometry rather than the VM's outer frame size: KWin owns the Linux frame. Additional theme/scaling and font calibration remains part of implementation validation; a screenshot alone does not establish exact font family or point size.
