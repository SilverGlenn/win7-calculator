# Native Linux Windows 7 Calculator — design for approval

Status: **Design approved by the operator on 2026-10-19. Implementation plan awaiting review.**

## Intent and confirmed requirements

Create a native Linux calculator in this repository that looks exactly like Windows 7 Calculator and fits the operator's existing Windows 7-style KDE Plasma desktop. The only visible addition to the original top-level UI is a menu named **Modern**. Later-era features belong beneath that menu and can be worked on later. This is a reproduction, not a modern redesign or a Windows 7-inspired approximation.

The operator confirmed the Linux target and identified `~/Projects/aerothemeplasma-win7` as the desktop integration reference. Mockups were offered prematurely and rejected for lack of reference evidence. Direct inspection of the real VM is the design basis; no mockup is needed.

## Evidence and current state

- Repository baseline: `d125246`. This is current Microsoft Calculator: Windows UWP/XAML frontend, managed view models and C++/WinRT interop, not an existing Linux port.
- Direct VM captures and measured geometry: [`../../reference/windows7-calculator/README.md`](../../reference/windows7-calculator/README.md). Twenty-six cropped screenshots cover all four modes, menus, history, supplementary panels, worksheets and representative interaction states.
- The VM's Standard client is approximately 212 × 284 logical pixels including its menu bar at the observed baseline. Standard keys use a nominal 39 px horizontal / 32 px vertical pitch, a 190 × 50 display, a double-height equals key and double-width zero key. Do not substitute the modern Microsoft key order.
- The VM has Basic graphics, not full Aero Glass. Its titlebar must not be copied as an Aero reference.
- Installed desktop evidence: KDE global font is Segoe UI 9; layered KDE defaults select `kvantum`; Kvantum selects `Windows7Aero`; KWin selects `org.smod.wordcraftcaption`; Qt6Widgets 6.10.2 and the Qt6 Kvantum style are installed. Segoe UI font files are present. The separate theme checkout is read-only reference material.

## Scope: the original surface

### Window and menus

Use a real, server-decorated Linux window titled **Calculator**, with the installed SMOD/Aero decorations and semantic calculator icon. Do not draw a fake titlebar, introduce a frameless webview, change KWin rules, or alter global theme/font/icon settings. The original is fixed-size by mode/panel, not a stretchy modern keypad. Let the original mode/panel transitions determine the required client size; preserve native move, minimize, close and taskbar behavior. Check original maximize/resizing behavior in the VM before selecting the final Qt flags.

Preserve original menu order and wording, then append the requested menu: **View · Edit · Help · Modern**. Appending Modern leaves the original menu positions unchanged. No hamburger, mode heading, permanent history sidebar, settings button, compact-overlay button or other modern chrome.

View retains Standard (Alt+1), Scientific (Alt+2), Programmer (Alt+3), Statistics (Alt+4); History (Ctrl+H); Digit grouping; Basic (Ctrl+F4); Unit conversion (Ctrl+U); Date calculation (Ctrl+E); Worksheets submenu. Capture checked, disabled and mode-specific action states before implementing them. History is original functionality, not inherently modern.

Edit retains Copy, Paste and History submenu (Copy history, Edit, Cancel edit, Clear), preserving observed availability and keyboard behavior. Help retains View Help and About Calculator. Help opens local, truthful usage documentation rather than depending on Windows Help infrastructure. About identifies this Linux recreation and credits its sources; it must not pretend to be a Microsoft Windows executable.

### Calculator modes

- **Standard:** exact five-column legacy keypad, memory row MC/MR/MS/M+/M−, legacy operator labels, immediate evaluation, clear-entry versus clear-all, percent semantics, repeat equals, error states and expression/display behavior.
- **Scientific:** original expanded left-hand function grid, Degrees/Radians/Grads selector, Inv-selected function transformations, brackets, powers/roots, trigonometric/hyperbolic operations, factorial, logarithms, constants and exponent entry. Match the VM rather than transplanting the current UWP operator panel.
- **Programmer:** original bit display, Hex/Dec/Oct/Bin selectors, Qword/Dword/Word/Byte selectors, A–F keys, original shifts/rotates and logical operators, signed-width behavior and disabled keys.
- **Statistics:** original sample list, navigation, Count, CAD, Add, mean/mean-square/sum/sum-square and population/sample deviation controls. This mode is missing from the current navigation and needs real implementation, not a decorative replica.

History occupies the original expanded display area above the keypad, with selection, scrolling, copy, edit/cancel and clear. Respect original mode-specific availability and keep memory indication, focus rectangles and display alignment consistent.

### Supplementary panels and worksheets

Unit conversion, Date calculation and Worksheets were verified in Windows 7 and stay under View, **not Modern**. The calculator remains on the left and the original white form panel opens on the right.

- Unit conversion: original category order Angle, Area, Energy, Length, Power, Pressure, Temperature, Time, Velocity, Volume, Weight/Mass; original From/To arrangement, unit lists, entry and result behavior. Inventory exact unit labels and conversion rules from the VM during the reference-completion step.
- Date calculation: both difference-between-dates and add/subtract-date forms, date pickers, years/months/days inputs, correct leap-year/calendar behavior, original result fields and Calculate action.
- Worksheets: Mortgage, Vehicle lease, Fuel economy (mpg), Fuel economy (L/100 km), with original solve-for dropdowns, changing input labels, Calculate action, result and invalid-input behavior. Inventory every solve-for variant before implementing the forms.

### Modern boundary

For the initial faithful Linux implementation, **Modern is reserved**, not a commitment to port the entire current UWP feature set. Builder call for approval: it contains a disabled `No modern features enabled` entry until a real later-era feature is implemented. Do not add nonfunctional feature buttons or pretend that graphing works. Existing UWP sources remain preserved in the repository; they are not Linux runtime features.

Graphing, currency/network conversion, always-on-top/compact overlay, modern settings and any later additions may only appear under Modern if implemented in future work. The checked-in upstream graphing engine is a mock. Do not fetch exchange rates, add telemetry, change the default calculator or assign desktop-wide shortcuts in this work.

## Appearance contract

The authority is the captured original at the baseline size, not a generic Kvantum button theme. Reproduce calculator client backgrounds, gradients, borders, key dimensions, individual dialog-unit rounding, glyph placement, blue/black/disabled text, expression/result alignment, memory markers, hover, pressed, checked and focus states. Standard and inverse Scientific captures demonstrate that mathematical labels and state transformations matter.

Use the installed theme for native menu/control primitives where their metrics and appearance match; custom-paint calculator-specific displays and keys when the generic style diverges. Retain widget focus, hit targets, keyboard activation and accessible names when painting. Do not ship screenshot-backed clickable images or paint text into raster assets.

Use the existing Segoe UI installation; calibrate number/expression and special-function fonts against the captures instead of asserting an unverified font from appearance. Do not bundle Microsoft font files or extract Windows artwork as shipped assets. Prefer the installed semantic icon `accessories-calculator` with an appropriate fallback.

Default 100% logical scale is the strict comparison baseline. Preserve the original geometry proportionally at larger DPI/font sizes; do not force physical pixel sizing that makes the application unusable at another scale. Distinguish compositor/text-rasterization differences from layout or paint errors. No claim of literal pixel-perfect parity until measured comparisons and human inspection support it.

## Implementation architecture — builder recommendation for approval

Use **C++20, CMake and Qt6 Widgets**, with native Wayland support and no UWP/WinRT dependency in the Linux target. These are proposed builder choices, not requirements the operator specified.

Keep the upstream Windows solution and source layout intact. Add a Linux target with a narrow frontend/engine boundary. Reuse `src/CalcManager` arithmetic/CEngine/RatPack and applicable memory/history behavior rather than replacing arithmetic with binary floating-point or `eval`. The source already contains portability shims but still includes Windows/PPL dependencies; isolate or replace those at the build boundary without breaking the upstream Windows target. The current UWP views, view models and WinRT bridge are not reused as the Linux UI.

Keep Qt out of the arithmetic core. A small adapter maps keys/commands and display callbacks. Legacy statistics, conversion, dates and worksheet calculations use independently testable logic. Prefer a single native engine/library plus focused legacy helpers, not a second competing evaluator. Precision, formatting, locale, sign and overflow behavior must be covered by tests and compared against the VM.

The builder can decide internal file layout, test framework, engine-adapter API and painting decomposition. A portability spike is the first implementation gate after approval: build and exercise the retained engine on Linux before broad UI work. If the engine requires a fundamentally different architecture or any legacy mode cannot be delivered truthfully, stop and bring back the evidence rather than silently substituting a limited calculator.

The Linux target must not depend on files under `~/Projects/aerothemeplasma-win7` at runtime. Use installed desktop integration with sensible fallback behavior on other Qt desktops. Treat the theme checkout as read-only and do not copy its licensed assets into this MIT repository without a separate license decision.

## Verification and acceptance

1. Finish the reference inventory: remaining selector variants, exact font/paint calibration, disabled menu states, window sizing flags, keyboard mappings, error cases and sample calculations. Record evidence before implementation of each relevant area; never guess a missing screen.
2. Automated arithmetic, memory/history, programmer-width, statistics, conversions, calendar and worksheet tests, including failure cases and representative VM parity cases.
3. Qt interaction tests for every mode/panel, menus/shortcuts, state transitions, focus, button labels, clipboard and the Modern boundary. No visible action may be a silent no-op.
4. Deterministic client-area screenshots at the same logical size for all baseline states. Compare geometry and pixel differences/overlays against VM crops, treating native frame and documented platform text rendering separately. Examine error maps, not just scaled thumbnails.
5. Live verification on the existing Plasma Wayland desktop with the installed theme: native decoration, taskbar identity, menus, input, clipboard, focus and 100%/larger scale. Offscreen-only tests do not establish desktop fidelity.
6. Provide actual application screenshots beside the original captures and an explicit remaining-differences list for operator inspection. This replaces premature mockups.
7. Provide build/run instructions and a reversible user-local installation/desktop-entry path. Do not require system-wide installation or change system defaults. Do not claim the installed calculator was updated unless that path was actually exercised and verified.

Acceptance target is the full original UI and functional legacy calculator described above, with Modern as the only added top-level menu. Intermediate implementation slices may be reviewed, but cannot be described as the completed faithful reproduction.

## Non-goals and guardrails

No redesign, browser wrapper, fake Windows frame, desktop theme edits, telemetry, online service integration, graphing-engine replacement, new modern features, or default-app reassignment. No rewriting unrelated Microsoft sources. Do not conflate a build/test pass with visual parity. Do not promise full Aero Glass equivalence based on the Basic-graphics VM.

## Approval record

The operator approved this description and the proposed C++/Qt6/retained-engine approach with “Yup sounds good.” on 2026-10-19, after the builder check identified only the pending architecture approval. This includes reserving Modern for later instead of porting modern UWP features now; original Windows 7 features remain fully in scope. The implementation plan is the next review gate.
