# Native Windows 7 Calculator Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Deliver a functional native Linux reproduction of the complete Windows 7 Calculator, with Modern as the only added top-level menu.

**Architecture:** Add an independent CMake/Qt6 Widgets application without replacing the upstream Windows solution. Port the retained CalcManager/CEngine/RatPack subset behind a Qt-free adapter; keep legacy statistics, conversion, date and worksheet logic separately testable. Native KWin decorations surround reference-calibrated calculator widgets.

**Tech Stack:** C++20, CMake 3.25+, Qt6 Core/Gui/Widgets/Test (6.4+ APIs; validate on installed 6.10.2), Ninja, Python3/Pillow for reference tooling. No runtime network services or downloaded test frameworks.

**Spec:** `docs/superpowers/specs/2026-10-19-windows7-calculator-design.md` (operator-approved 2026-10-19).

## Global Constraints

- “The only visible addition to the original top-level UI is a menu named **Modern**.”
- “This is a reproduction, not a modern redesign or a Windows 7-inspired approximation.”
- “Use a real, server-decorated Linux window titled **Calculator**, with the installed SMOD/Aero decorations and semantic calculator icon.”
- “Preserve original menu order and wording, then append the requested menu: **View · Edit · Help · Modern**.”
- “Unit conversion, Date calculation and Worksheets were verified in Windows 7 and stay under View, **not Modern**.”
- “For the initial faithful Linux implementation, **Modern is reserved**, not a commitment to port the entire current UWP feature set.”
- Modern contains the approved disabled entry `No modern features enabled`.
- “Default 100% logical scale is the strict comparison baseline.”
- “The Linux target must not depend on files under `~/Projects/aerothemeplasma-win7` at runtime.”
- Preserve upstream Windows sources/solution and MIT notices; isolate portability changes. No Qt dependency in the arithmetic core.
- No fake frame, screenshot-backed controls, bundled Microsoft fonts/artwork, telemetry, default-app reassignment, desktop-wide shortcuts or global theme changes.
- VM is a shared resource: parent alone controls it and coordinates with the operator; children inspect saved references only.
- Original Basic-frame sizes are evidence, not Linux outer-window requirements. Compare the client area; do not emulate Basic decorations on Aero.
- Every visible enabled action works. Do not ship silent no-ops or call an intermediate slice complete.

## Review Focus

1. Pasted or typed localized numbers, Unicode minus and invalid/oversized clipboard text must not execute code, freeze the UI, or corrupt the existing calculation — tests in Tasks 1–2.
2. Repeated mode, history and panel transitions must retain/reset exactly the VM-observed state without stale focus, duplicate signal connections or use-after-free — tests in Tasks 2, 4–7.
3. Programmer negative values, highest-bit toggles and narrowing at 8/16/32/64 bits must have defined signed-width behavior without C++ signed-overflow UB — tests in Task 5.
4. Calendar boundary dates and worksheet zero rates/zero divisors must produce reference-correct results or explicit errors, not invalid dates, infinities or crashes — tests in Tasks 9–10.
5. Missing Segoe UI/Kvantum, larger fonts and fractional DPI must retain keyboard access and complete labels; font fallback must not silently count as reference fidelity — tests in Tasks 3 and 11.

## File map and ownership

- Root `CMakeLists.txt`: Linux entry point, CTest and install rules; no Windows solution rewrite.
- `src/LinuxCalculator/CMakeLists.txt`: `win7calc_engine`, `win7calc_legacy`, `win7calc_widgets`, `win7-calculator` targets, with explicit source lists.
- `src/LinuxCalculator/engine/{Engine,Resources,Numeric}.{h,cpp}`: Qt-free command/display adapter, engine string provider, rational parsing/formatting/error boundary.
- `src/LinuxCalculator/model/{CalculatorState.h,CalculatorController.h,CalculatorController.cpp}`: Qt-facing mode/panel/focus state and one engine owner.
- `src/LinuxCalculator/widgets/{CalculatorWindow,CalculatorKey,CalculatorDisplay,Keypad,ScientificControls,ProgrammerControls,StatisticsControls,HistoryView}.{h,cpp}`: one focused widget responsibility per pair.
- `src/LinuxCalculator/widgets/{LegacyMetrics.h,LegacyPalette.h,KeyLayout.h,KeyLayout.cpp}`: measured geometry/paint values and data-driven key definitions, no arithmetic.
- `src/LinuxCalculator/legacy/{Statistics,Units,Worksheets}.{h,cpp}`: rational computations and typed metadata, no widgets.
- `src/LinuxCalculator/legacy/{Dates.h,Dates.cpp}`: calendar-only logic using QDate, no widgets.
- `src/LinuxCalculator/panels/{UnitPanel,DatePanel,WorksheetPanel}.{h,cpp}`: exact supplementary forms using the legacy helpers.
- `src/LinuxCalculator/{main.cpp,resources.qrc}`: native startup and embedded truthful resources. Generate `build/linux/generated/EngineStringsData.h` (C++ string table compiled into the Qt-free engine) and `build/linux/generated/engine-en-US.json` (auditable metadata) from upstream MIT `.resw` data; no Qt/JSON parser dependency in the core.
- `tests/linux/`: QtTest executables, test fixtures and shared input helpers; each task adds its named test file.
- `tests/linux/fixtures/{vm-cases.json,ui-states.json}`: observed oracle cases and exact UI inventory, with capture provenance.
- `tools/linux/{generate-engine-strings.py,compare-screenshots.py,capture-reference.py}`: deterministic resource generation, image reporting and parent-run VM capture helper.
- `scripts/linux/{run.sh,install-user.sh,uninstall-user.sh}` and `packaging/linux/org.win7calculator.Calculator.desktop`: launch/reversible user-local install.
- `docs/linux/{build.md,reference-inventory.md,validation.md}`: build, evidence and acceptance report.
- `src/CalcManager/`: edit only compilation-proven portability issues; retain `_MSC_VER` branches where Windows behavior requires them. Do not pull `UnitConverter.cpp`, `NumberFormattingUtils.cpp`, `pch.cpp` or PPL into the portable target just to get arithmetic.

All new C++ types below are in namespace `Win7Calc`. Existing `CalculationManager::Command` remains the arithmetic command vocabulary. Keep all engine/RatPack calls serialized on the owning application thread: upstream has process-global precision/constants/resource state. Tests that change locale/resource initialization run as separate CTest processes.

## Shared contracts

Define these once in Tasks 1–2 and use them unchanged:

- `enum class Mode { Standard, Scientific, Programmer, Statistics };`
- `enum class Panel { Basic, Units, Dates, Mortgage, VehicleLease, FuelMpg, FuelMetric };`
- `struct NumericLocale { wchar_t decimal; wchar_t grouping; std::wstring groupingPattern; };`
- `enum class NumericError { None, InvalidInput, Domain, DivideByZero, Overflow, InsufficientData };`
- `struct NumericResult { std::optional<CalcEngine::Rational> value; NumericError error; };`
- `struct HistoryEntry { std::wstring expression, result; std::vector<std::shared_ptr<IExpressionCommand>> commands; };`
- `struct DisplayState { std::wstring primary, expression; bool error, memory; std::vector<HistoryEntry> history; };`
- `struct CalculatorState { Mode mode; Panel panel; bool historyVisible, digitGrouping, inverse; DisplayState display; };`
- `Engine(NumericLocale)`, `void Engine::setMode(Mode)`, `void Engine::send(CalculationManager::Command)`, `bool Engine::pasteNumber(std::wstring_view)`, `void Engine::setDigitGrouping(bool)`, `const DisplayState& Engine::state() const`, `CalcEngine::Rational Engine::value() const`, `void Engine::setValue(const CalcEngine::Rational&)`, `void Engine::selectHistory(std::size_t)`, `void Engine::clearHistory()`, `bool Engine::beginHistoryEdit(std::size_t)`, `void Engine::cancelHistoryEdit()`.
- `NumericResult parseDecimal(std::wstring_view, NumericLocale)`, `std::wstring formatDecimal(const CalcEngine::Rational&, NumericLocale, int significantDigits = 32)`; use retained `StringToRat`/Rational and checked errors, never `double` parsing or `eval`.
- `CalculatorController(QObject* parent = nullptr)`, `const CalculatorState& state() const`, slots `setMode(Mode)`, `setPanel(Panel)`, `setHistoryVisible(bool)`, `setDigitGrouping(bool)`, `send(CalculationManager::Command)`, `copy()`, `paste()`, `selectHistory(int)`, `beginHistoryEdit(int)`, `cancelHistoryEdit()`, `clearHistory()`, `copyHistory()`; signal `stateChanged()`.
- `CalculatorWindow(CalculatorController&, QWidget* parent = nullptr)`; mode-specific widgets consume the same controller by reference, with their state reflected through `stateChanged()`.

`Engine::setMode(Statistics)` is not a second arithmetic evaluator: retain the last applicable arithmetic engine and let StatisticsControls manage its sample list through the shared rational helpers. Explicitly match reference mode-switch behavior from the fixture.

## Execution order and gates

**Operator-directed ownership (2026-10-19):** The parent personally implements the most complicated work: retained-engine portability and adapter (Task 1), shared state/input (Task 2), native shell and paint primitives (Task 3), Scientific and Programmer behavior (Tasks 4–5), history editing (Task 7), difficult numerical/calendar cases in Tasks 6/8–10, and full-app visual calibration/integration (Task 11). Subagents may inspect evidence, draft bounded metadata/tests, implement isolated supporting pieces such as installation tooling, and perform independent reviews. They do not own core architecture, difficult algorithms, cross-component integration or fidelity acceptance. There is one writer per checkout/worktree. This is parent-led execution, not a fresh implementation agent per task.

Tasks 1–3 establish the seam used by all UI tasks. Tasks 4–7 follow in order because they share the shell/controller and upstream state. Tasks 8–10 are independently reviewable legacy components once those contracts are stable; run sequentially by default. If delegated in parallel, give each its own worktree and exclusive legacy/panel/test files; integration into CalculatorWindow/CMake stays with the parent. Task 11 is integration/visual calibration only after all functional lanes pass. Task 12 is release validation and reversible installation, not an opportunity to hide unfinished features.

Use a failing-test → minimal implementation → passing-test cycle within every task. Capture missing VM behavior before coding its interpretation. Run task-local tests during development and the aggregate suite at integration. Commit only owned paths, after verification; do not push.

### Task 1: Build the retained engine on Linux with a reference-backed command adapter

**Files:** Create root/`src/LinuxCalculator` CMake files, `engine/*`, `tools/linux/generate-engine-strings.py`, `tests/linux/{CMakeLists.txt,engine_test.cpp,numeric_test.cpp}`, fixture `vm-cases.json`; modify only compilation-proven `src/CalcManager` portability seams.

**Consumes:** existing `CalculatorManager`, `CCalcEngine`, `ICalcDisplay`, `IResourceProvider`, Rational/RationalMath and `Command` interfaces. **Produces:** `Engine`, `Resources : IResourceProvider`, `NumericLocale`, `NumericResult`, `HistoryEntry`, `DisplayState` and all Qt-free functions in Shared contracts; runnable `engine_test`, `numeric_test`.

- [ ] Record Standard VM cases: `12+3=15`, `2+3*4=20` (immediate evaluation), `0.1+0.2=0.3`, repeat equals, `200+10%`, CE versus C, backspace, memory store/recall/add/subtract/clear, divide-by-zero and recovery. Save actual strings/command sequences/capture provenance in `vm-cases.json`; if an expected value differs from the examples, preserve the VM evidence and explain it before changing an assertion. Add resource completeness tests for every engine ID in `EngineStrings.h` and locale keys `sDecimal/sThousand/sGrouping`.
- [ ] Write failing `EngineTest::standardParity`, `singleLegacyMemory`, `errorRecovery`, `NumericTest::exactDecimal`, `rejectMalformed`, `localeIsolation` tests: command replay matches fixture strings; `parseDecimal(L"0.1") + parseDecimal(L"0.2") == parseDecimal(L"0.3")`; comma-decimal locale works in a separate process; malformed/oversized text leaves prior state unchanged. Reject inputs over 4096 code units before parsing, with explicit invalid-input feedback.
- [ ] Configure with `cmake -S . -B build/linux -G Ninja -DBUILD_TESTING=ON`; build `engine_test numeric_test`; run `ctest --test-dir build/linux -R '^(engine|numeric)' --output-on-failure`. Confirm a meaningful red result against the new requirements before adding the adapter body; do not count an unavailable dependency as a behavior-test failure.
- [ ] Build an explicit source subset: all needed `CEngine/*.cpp`, `Ratpack/*.cpp`, `CalculatorManager.cpp`, `CalculatorHistory.cpp`, `ExpressionCommand.cpp`. Use upstream portability shims and direct standard includes instead of `pch.h`. Generate/validate the embedded en-US resource map from `src/Calculator/Resources/en-US/CEngineStrings.resw` without a Windows resource loader. Implement the shared adapter/functions with one legacy memory register, original display precision calibrated to the VM, and correct exception-to-error translation. Keep modern multi-memory UI out. If `setValue` needs a new manager API, add a narrow value transfer rather than decimal round-tripping or a second evaluator.
- [ ] Run the two tests to green, plus a Release build. Run `ldd` on the engine test and verify no WinRT/UWP/PPL dependency; the `win7calc_engine` CMake target itself has no Qt link dependency. Record any Windows-source edits and their preserved MSVC branches. Commit as `feat(linux): port retained calculator engine and legacy adapter`.

**Gate:** Stop for architectural evidence if the retained engine cannot be made correct/portable; do not replace it with a low-precision toy calculator. Engine tests are headless and must finish before GUI work.

### Task 2: Add legacy state, resource initialization, clipboard and input routing

**Files:** Create `model/*`, `tests/linux/controller_test.cpp`, fixture `ui-states.json`; extend engine tests only for newly exposed state behavior.

**Consumes:** Task 1 contracts. **Produces:** `Mode`, `Panel`, `CalculatorState`, `CalculatorController` slots/signal; one controller owns one Engine; all UI command paths use that owner.

- [ ] Use the VM to record mode/panel/history retention, grouping, memory sharing, paste acceptance/error, per-mode menu enablement, history edit/cancel semantics and supported key mappings in `ui-states.json`. Capture resize/maximize behavior without changing desktop settings. Include one sequence switching each mode with each supplementary-panel choice; states unsupported by the original must not be invented.
- [ ] Write failing `ControllerTest::initialState`, `transitionMatrix`, `oneCommandPerKey`, `singleMemoryAcrossModes`, `clipboardValidation`, `historyEditRollback`, `groupingNotValueMutation`. Assert the initial Standard/Basic/zero state, exact fixture transitions and untouched value after invalid paste. Include Unicode minus, comma decimal, empty clipboard, 4097-character input and repeated mode changes.
- [ ] Run `cmake --build build/linux --target controller_test && QT_QPA_PLATFORM=offscreen ctest --test-dir build/linux -R '^controller$' --output-on-failure`; confirm red.
- [ ] Implement the shared controller API, initialize locale resources once before engine construction, and map numeric/operation keyboard input without consuming text-entry keys in supplementary fields. Clipboard uses QClipboard, no expression evaluation/shell invocation. Centralize checked/enabled menu state in the fixture-driven mode rules; preserve engine error recovery.
- [ ] Run controller/engine/numeric tests to green; commit `feat(linux): add legacy calculator state and input controller`.

### Task 3: Deliver the native Standard calculator shell and reusable painted controls

**Files:** Create `main.cpp`, `resources.qrc`, `widgets/{CalculatorWindow,CalculatorKey,CalculatorDisplay,Keypad,LegacyMetrics,LegacyPalette,KeyLayout}.*`, `tests/linux/{standard_ui_test.cpp,layout_test.cpp}` and initial `docs/linux/build.md`.

**Consumes:** controller and shared contracts. **Produces:** executable `win7-calculator`; `CalculatorKey(const KeySpec&, QWidget*) : QAbstractButton`, `CalculatorDisplay(QWidget*)`, `Keypad(CalculatorController&, QWidget*)`; `QSize clientSize(Mode, Panel, bool historyVisible, qreal logicalScale)` and `std::vector<KeySpec> keyLayout(Mode, bool inverse)` in `KeyLayout.h`. `KeySpec` contains stable ID, text, accessible name, logical QRect and optional upstream Command.

- [ ] Write failing tests for menu texts exactly `View, Edit, Help, Modern`; disabled Modern item exactly `No modern features enabled`; title `Calculator`; Standard key positions relative to the reference client; zero spanning two columns and equals two rows; correct MC/MR/MS/M+/M− order; mouse and keyboard `12+3=15`. Define named widgets/actions by stable IDs for tests, not by translated display text. Use source window (125,125), client origin (133,155), display (144,185,190,50), key origins x144/183/222/261/300 and y240/272/304/336/368/400, with measured integer widths from the capture rather than uniformly rounding every key.
- [ ] Run `cmake --build build/linux --target standard_ui_test layout_test && QT_QPA_PLATFORM=offscreen ctest --test-dir build/linux -R '^(standard_ui|layout)$' --output-on-failure`; confirm red.
- [ ] Implement true widgets, correct focus/accessibility and pressed/hover/disabled rendering, resource icon fallback and the four menus. Use the installed Qt style for menus only if calibration matches; adapt local QStyle metrics/painting when necessary, never global settings. Fix client dimensions per the observed original state; no custom frame. Original unimplemented actions are temporarily disabled during this slice, not silently enabled; Tasks 4–10 must complete them before acceptance. View Help opens bundled local instructions; About truthfully identifies the recreation and MIT sources.
- [ ] Extend layout tests for 1.0/1.25/1.5/2.0 logical scales and larger UI fonts: keys remain reachable, menus do not overlap, full labels fit, no double application of Qt DPR. Run an isolated fallback-font/style test without changing host settings; absent Segoe UI must be reported in visual-validation metadata. Run all local tests to green and launch on Wayland to check native SMOD decoration and taskbar identity.
- [ ] Save the first actual client screenshot and build/run instructions; commit `feat(linux): add native Windows 7 Standard calculator shell`.

### Task 4: Implement the Scientific mode and inverse function states

**Files:** Create `widgets/ScientificControls.*`, `tests/linux/scientific_test.cpp`; extend `KeyLayout`, controller mode rules, shell and oracle fixtures.

**Consumes:** Engine commands/Mode, reusable keys/display, layout API. **Produces:** `ScientificControls(CalculatorController&, QWidget*)`; complete Scientific normal/inverse layouts and function commands.

- [ ] Record VM values for degree/radian/grad trigonometry, Inv transformations, hyperbolic inverse, factorial/domain errors, dms/deg, powers/roots, Int/Frac, Exp and F-E. Record exact visible labels and post-operation Inv persistence. Use captures 03/28 as the layout authority.
- [ ] Write failing `ScientificTest::precedence` (`2+3*4=14`), `angleUnits` (sin 30 degrees = 0.5), `inverseLabels`, `inverseParity`, `domainRecovery`, `repeatedModeTransitions`; every normal/inverse key must map to an implemented command and reference label, including disabled blank cells.
- [ ] Run `cmake --build build/linux --target scientific_test && QT_QPA_PLATFORM=offscreen ctest --test-dir build/linux -R '^scientific$' --output-on-failure`; confirm red.
- [ ] Implement the original left-side grid and selectors, using upstream commands and the existing controller. Extend only necessary rational/engine behavior; do not expose newer sec/csc/random functions absent from the reference UI.
- [ ] Run scientific, controller and layout tests to green and save normal/inverse screenshots; commit `feat(linux): reproduce Scientific calculator mode`.

### Task 5: Implement Programmer mode, word widths and the original bit display

**Files:** Create `widgets/ProgrammerControls.*`, `tests/linux/programmer_test.cpp`; extend `KeyLayout`, shell, controller and fixtures.

**Consumes:** retained programmer commands, state and layout contracts. **Produces:** `ProgrammerControls(CalculatorController&, QWidget*)`; synchronized radix, width and bit-toggle UI.

- [ ] Record VM cases for all bases and widths, negative decimal versus unsigned hex display, highest-bit toggling, narrowing, rotations/shifts and disabled decimal/scientific keys. Determine shift counts and overflow handling from the actual original, not host integer conventions.
- [ ] Write failing `ProgrammerTest::radixAndDisabledKeys`, `signedWidths`, `highestBitToggle`, `narrowing`, `shiftRotateParity`, `modeSwitchState`. Include Byte NOT 0 → decimal −1 / hex FF and bit positions 7/15/31/63, with actual VM strings stored in the fixture.
- [ ] Run `cmake --build build/linux --target programmer_test && QT_QPA_PLATFORM=offscreen ctest --test-dir build/linux -R '^programmer$' --output-on-failure`; confirm red.
- [ ] Implement the original two-row grouped bit display and side selectors, A–F and original operation grid. Use checked unsigned bit operations/retained engine commands rather than signed shifts or `1 << 63`.
- [ ] Run programmer/controller/layout tests to green and a sanitizer build for width/shift cases; save a screenshot with a nonzero high bit; commit `feat(linux): reproduce Programmer calculator mode`.

### Task 6: Implement Statistics samples and original calculations

**Files:** Create `legacy/Statistics.*`, `widgets/StatisticsControls.*`, `tests/linux/statistics_test.cpp`; integrate shell/controller/layout and fixtures.

**Consumes:** Rational, NumericResult, Engine value transfer, numeric formatting. **Produces:** `enum class Statistic { Mean, MeanSquare, Sum, SumSquare, PopulationDeviation, SampleDeviation }; NumericResult calculateStatistic(std::span<const CalcEngine::Rational>, Statistic)` and `StatisticsControls(CalculatorController&, QWidget*)`.

- [ ] Record empty/single/multiple sample behavior, Add/CAD/backspace/C semantics, list selection/navigation, sample deletion and negative/decimal entry. Verify whether Inv/F-E/Exp or memory alter sample entry and record exact Count/result placement.
- [ ] Write failing `StatisticsTest::exactAggregates`, `deviations`, `insufficientData`, `sampleLifecycle`, `sampleEntryParity`. For samples 1,2,3 assert sum 6, sum-square 14, mean 2, mean-square 14/3, sample deviation 1; population deviation equals retained rational root(2/3) at reference display precision. Empty sample results follow the VM fixture rather than invented zero values.
- [ ] Run `cmake --build build/linux --target statistics_test && QT_QPA_PLATFORM=offscreen ctest --test-dir build/linux -R '^statistics$' --output-on-failure`; confirm red.
- [ ] Implement rational aggregation/deviation with explicit insufficient-data errors, then the original list/Count/key layout and memory integration; do not reinterpret CAD or Add as unrelated modern commands.
- [ ] Run statistics, transition and layout tests to green; save empty/populated/error screenshots; commit `feat(linux): restore Windows 7 Statistics mode`.

### Task 7: Complete original history display, selection and editing

**Files:** Create `widgets/HistoryView.*`, `tests/linux/history_test.cpp`; extend engine history adapter, display, controller and menu states.

**Consumes:** HistoryEntry/Engine history methods and controller slots. **Produces:** `HistoryView(CalculatorController&, QWidget*)` and full original history UI behavior, without a modern sidebar.

- [ ] Record additional VM cases for history capacity/scrolling, selecting an earlier entry, editing an operand and recalculating, cancel edit, clear shortcut and copy-history text. Record history availability in Programmer/Statistics and behavior with supplementary panels.
- [ ] Write failing `HistoryTest::displayExpansion`, `selectionAndReplay`, `editCancelRestores`, `editChangesOnlySelectedCalculation`, `copyAndClear`, `availabilityByMode`, `repeatedToggleNoDuplicateCommands`. Standard expanded outer VM height is 422 versus 322, a 100 px client increase; keypad moves down, it does not shrink.
- [ ] Run `cmake --build build/linux --target history_test && QT_QPA_PLATFORM=offscreen ctest --test-dir build/linux -R '^history$' --output-on-failure`; confirm red.
- [ ] Implement the embedded history region, arrows, selected-row highlight and original Edit > History actions. Replay retained expression command snapshots rather than parsing formatted strings; take an immutable edit snapshot for cancel. Do not silently keep upstream's 20-item limit if the VM oracle demonstrates different capacity.
- [ ] Run history/controller/engine/layout tests to green; save selected/editing screenshots; commit `feat(linux): reproduce legacy calculation history and editing`.

### Task 8: Implement original unit conversion metadata, arithmetic and form

**Files:** Create `legacy/Units.*`, `panels/UnitPanel.*`, `tests/linux/units_test.cpp`, `tests/linux/fixtures/units.json`; integrate shell after component tests pass.

**Consumes:** NumericResult, parseDecimal/formatDecimal, Rational. **Produces:** `struct UnitCategory { std::string id; std::wstring label; std::vector<std::string> units; }; const std::vector<UnitCategory>& unitCategories(); std::wstring unitLabel(std::string_view); NumericResult convertUnit(const CalcEngine::Rational&, std::string_view fromId, std::string_view toId); UnitPanel(CalculatorController&, QWidget*)`.

- [ ] Parent inventories every original unit label/order in all 11 categories from the VM and records them in `units.json`, including temperature offsets and any non-SI definitions. Categories must be exactly Angle, Area, Energy, Length, Power, Pressure, Temperature, Time, Velocity, Volume, Weight/Mass. No Currency/Data or modern category names.
- [ ] Write failing `UnitsTest::categoryAndUnitInventory`, `exactFactors`, `temperatureAffine`, `roundTrips`, `invalidInputs`, `panelKeyboardRouting`. Assert 1 inch = 0.0254 meters, 0 Celsius = 32 Fahrenheit, and oracle display strings for representative units in every category; invalid IDs and malformed input return errors without mutating calculator memory/history.
- [ ] Run `cmake --build build/linux --target units_test && QT_QPA_PLATFORM=offscreen ctest --test-dir build/linux -R '^units$' --output-on-failure`; confirm red.
- [ ] Implement exact rational scale/offset tables with stable IDs and the original form labels/placeholder/dropdowns/disabled left display. The panel updates as the VM does, without a new Convert button. Register this working panel through the original View action.
- [ ] Run units/controller/layout tests to green; compare the actual unit-panel screenshot to capture 07; commit `feat(linux): reproduce Windows 7 unit conversion panel`.

### Task 9: Implement calendar arithmetic and both original Date forms

**Files:** Create `legacy/Dates.*`, `panels/DatePanel.*`, `tests/linux/dates_test.cpp`; integrate shell and oracle fixtures.

**Consumes:** Panel/controller and Qt QDate. **Produces:** `struct DateDifference { int years, months, weeks, days; qint64 totalDays; }; std::optional<DateDifference> dateDifference(QDate from, QDate to); std::optional<QDate> shiftDate(QDate from, int years, int months, int days, bool subtract); DatePanel(CalculatorController&, QWidget*)`.

- [ ] Record VM date range, equal/reversed dates, leap day, month-end shifts, component application order and date-format behavior. The output decomposition and invalid-date policy come from these cases; do not assume that naïve 365/30-day division or QDate defaults match.
- [ ] Write failing `DatesTest::differenceDecomposition`, `equalAndReversed`, `leapAndMonthEnd`, `boundaryDates`, `formsAndFocus`. Assert 2024-02-28 to 2024-03-01 is 2 total days; use observed strings/components for 2024-02-29 +1 year and January 31 +1 month. Invalid/out-of-range results return nullopt and visible feedback, never wrap.
- [ ] Run `cmake --build build/linux --target dates_test && QT_QPA_PLATFORM=offscreen ctest --test-dir build/linux -R '^dates$' --output-on-failure`; confirm red.
- [ ] Implement calendar-component arithmetic plus the exact difference and add/subtract forms, original Calculate action, original date-picker layout and localized result copy. Text-field arrow/navigation input must not become calculator commands.
- [ ] Run dates/controller/layout tests to green; save both forms/calendar-popup screenshots; commit `feat(linux): reproduce Windows 7 date calculation`.

### Task 10: Implement all worksheet solve-for variants and original forms

**Files:** Create `legacy/Worksheets.*`, `panels/WorksheetPanel.*`, `tests/linux/worksheets_test.cpp`, `tests/linux/fixtures/worksheets.json`; integrate original submenu actions.

**Consumes:** Rational math, NumericResult, Panel and controller. **Produces:** `struct WorksheetSpec { std::string id; std::wstring label; std::vector<std::string> inputIds; }; const std::vector<WorksheetSpec>& worksheetSpecs(Panel); NumericResult solveWorksheet(std::string_view solveId, const std::map<std::string, CalcEngine::Rational>& inputs); WorksheetPanel(Panel, CalculatorController&, QWidget*)`.

- [ ] Inventory every solve-for dropdown for Mortgage, Vehicle lease, Fuel economy (mpg), Fuel economy (L/100 km), including changing labels and invalid-input behavior. Record one successful oracle case per solve ID plus zero-rate, zero-distance/fuel, negative term and unsolvable/log-domain cases.
- [ ] Write failing `WorksheetsTest::completeSolveInventory`, `mortgageAllVariants`, `leaseAllVariants`, `fuelBothUnits`, `zeroAndDomainBoundaries`, `formFieldTransitions`. Assert 100 miles / 4 gallons = 25 mpg; 8 liters / 100 km = 8 L/100 km; zero-interest mortgage payment equals (price−down payment)/(years×12), subject to the observed validation policy. Every solve ID in the fixture must execute a tested formula.
- [ ] Run `cmake --build build/linux --target worksheets_test && QT_QPA_PLATFORM=offscreen ctest --test-dir build/linux -R '^worksheets$' --output-on-failure`; confirm red.
- [ ] Implement retained-rational algebra/powers/logarithms and precise error handling, including zero-rate limits; do not invent loan-rate solvers absent from the VM dropdown. Implement original dynamic form layout, placeholder and result fields with no new controls.
- [ ] Run worksheets/controller/layout tests to green; save one screenshot per solve variant; commit `feat(linux): reproduce calculator worksheets`.

### Task 11: Calibrate full appearance and verify all interaction states on Plasma

**Files:** Create `tools/linux/{capture-reference.py,compare-screenshots.py}`, `tests/linux/{visual_test.cpp,test_image_compare.py}`, `docs/linux/{reference-inventory.md,validation.md}`; adjust only measured widget/layout/paint differences and their regression tests.

**Consumes:** complete working legacy application, original reference captures and fixture state IDs. **Produces:** deterministic client captures, overlay/diff images, numeric geometry/color metrics and a truthful per-state remaining-differences report.

- [ ] Write failing `VisualTest::referenceStateGeometry`, `allEnabledActionsImplemented`, `dpiAndFontFallback`, `buttonStateRendering` and Python comparison-tool tests. The image tool must detect a 1 px shifted key and a deliberately wrong gradient color, and must never hide all text/key borders with broad masks. Test a precise exclusion for the added Modern menu footprint only when comparing original menu-bar content.
- [ ] Run `cmake --build build/linux --target visual_test && QT_QPA_PLATFORM=offscreen ctest --test-dir build/linux -R '^(visual|image_compare)$' --output-on-failure`; confirm red.
- [ ] Add `win7-calculator --capture-state <fixture-id> --capture-output <path>` for deterministic test capture, without adding visible UI controls. Client capture excludes native decorations; standard baseline client is 212×284 before scale adjustment. Generate overlays, absolute channel-error heatmaps, per-region metrics and explicit text-rendering notes from identical logical bounds. No resampling screenshots to conceal size errors or auto-accepting regenerated goldens.
- [ ] Calibrate fonts (including numeric/expression/special-function faces), colors, gradients, borders, menu metrics, mathematical glyphs, selected/disabled/focus/error states and every panel/solve variant against the VM. For each mismatch add or tighten a geometry/color/state regression before the paint fix. Check content pixel differences exactly; keep renderer-dependent text differences visible in the report. Geometry must match reference at default scale, with zero tolerance for key/field placement errors.
- [ ] Run the entire suite in offscreen mode, then launch using the current Plasma Wayland session and installed Kvantum/SMOD. Manually verify native frame/taskbar identity, menu activation/mnemonics, clipboard, focus traversal, mouse-down/release outside a key, history editing, larger fonts and 100/125/150% logical scale. Record backend/style/fonts/DPR and actual evidence. Commit `test(linux): calibrate and verify Windows 7 calculator fidelity` only after reporting unresolved differences rather than claiming automatic parity.

### Task 12: Package, review and validate the runnable installed application

**Files:** Create `scripts/linux/{run.sh,install-user.sh,uninstall-user.sh}`, `packaging/linux/org.win7calculator.Calculator.desktop`, `tests/linux/install_test.py`; finish `docs/linux/{build.md,validation.md}`; add a concise Linux section to root `README.md` and a Linux build/test workflow under `.github/workflows/` without altering existing Windows CI.

**Consumes:** fully verified `win7-calculator` target and visual report. **Produces:** clean Release build, reversible user-local install, truthful documented acceptance state.

- [ ] Write failing isolated-HOME install tests asserting executable/library/resource/desktop-entry presence, correct `Exec`, `Icon=accessories-calculator`, `StartupWMClass`/Wayland app ID consistency, idempotent upgrade/removal and preservation of unrelated files. Do not modify KDE default apps or theme configuration. Record a manifest of only installed owned files.
- [ ] Run `python3 -m unittest discover -s tests/linux -p 'install_test.py' -v`; confirm red.
- [ ] Implement CMake install rules and scripts defaulting to `$HOME/.local`, with explicit prefix override and safe path quoting. Add build/run/update/uninstall instructions, license attribution and CI commands for all portable tests. Avoid depending on the source directory or a local theme checkout after installation.
- [ ] From a clean execution worktree, configure/build Release and run `QT_QPA_PLATFORM=offscreen ctest --test-dir build/linux --output-on-failure`, image-tool tests, isolated installation tests and `git diff --check`. Use an authorized fresh Luna reviewer for the entire diff, legacy-surface coverage, portability changes and evidence claims; parent addresses complex findings and reruns affected plus aggregate tests. Use exact model `openai/gpt-6-luna:xhigh` for Luna children and, if useful for bounded support, `openai/gpt-6.1-sol:low`, with no more than three concurrent Sol children.
- [ ] Install user-locally only through the provided reversible script, then launch the installed binary on the real desktop and record its executable path/version/hash and reference screenshots. Present original-versus-actual screenshots plus any remaining differences to the operator. Commit `build(linux): package native Windows 7 calculator` after verification. Do not equate passing tests with operator acceptance or assert pixel-perfect parity while documented differences remain.

## Plan self-review / coverage map

- Intent/native desktop/Modern boundary: Tasks 2–3, 11–12.
- Original menus, keyboard, clipboard, Help/About: Tasks 2–3, 7; later feature actions completed in Tasks 4–10.
- Arithmetic/memory/error behavior and portability: Tasks 1–2; Scientific 4; Programmer 5; Statistics 6.
- Original history and edits: Tasks 2 and 7.
- Conversion/date/all worksheets: Tasks 8–10, with inventory gates forbidding guessed variants.
- Exact client appearance, interaction states, accessibility and scale: Tasks 3–11; measured full-app verification in 11.
- No theme edits/Windows breakage/network/modern feature creep: Global Constraints plus final whole-diff review.
- Build/install/documentation/truthful evidence: Tasks 1, 3, 11–12.

The plan's shared types/signatures are the handoff contract; implementations may split private helpers but must coordinate any public-interface change before dependent tasks start. Remaining VM inventory/calibration is explicitly assigned to tested task gates, not left as permission to invent UI.
