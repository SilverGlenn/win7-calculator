# Native Linux validation — in progress

This report records implementation evidence, **not final fidelity acceptance**.

## 2026-10-19: first Standard window (Task 3)

- At this milestone the headless suite comprised six CTest processes (engine, numeric, separate comma locale, controller, Standard UI, layout), all passing.
- Standard UI suite also passed with `QT_SCALE_FACTOR=1.25`, `QT_SCALE_FACTOR=1.5`, and the Fusion fallback style. Larger/missing-family font tests preserve calculator access.
- Native Wayland launch uses the installed `kvantum` style and Segoe UI 9. KWin reports `noBorder: false`, `skipTaskbar: false`, `desktopFile/resourceClass: org.win7calculator.Calculator`, `resourceName: win7-calculator`, outer geometry 228×322. Size/Maximize behavior is constrained by the fixed client size, matching the original system-menu evidence.
- Actual client screenshot: [`images/standard-client-task3.png`](images/standard-client-task3.png).
- Actual system-decorated window screenshot: [`images/standard-native-frame-task3.png`](images/standard-native-frame-task3.png). Captured via Spectacle with the calculator temporarily activated; prior focus was restored. The frame is the operator's installed SMOD decoration, not a painted application imitation.
- Embedded help/icon resources were tested after discovering static-library dead stripping: the new regression failed before explicit resource initialization and passed afterward.
- The four-mode/seven-panel VM matrix and original fixed-size system menu are saved under `docs/reference/windows7-calculator/states/` and `tests/linux/fixtures/ui-states.json`.

## 2026-10-19: Scientific mode (Task 4)

- Full current suite: seven CTest processes pass, including Scientific precedence, three angle units, inverse labels/lifetime, inverse reset versus factorial persistence, DMS/deg, Int/Frac, factorial/domain recovery, powers/roots, exponent entry, F-E and repeated mode changes.
- Direct mouse-driven VM oracle captures are under `docs/reference/windows7-calculator/scientific/`. Compact geometry was checked before entry. Earlier unverified keyboard-entry batches were discarded, not used as fixtures.
- Original Inv consumes itself only for affected functions; factorial leaves it selected. Re-entering Scientific resets the angle selector to Degrees. The controller mirrors the retained engine's existing inverse state instead of double-toggling it through modern inverse-command aliases.
- Native Wayland captures: [normal](images/scientific-client-task4.png), [Inv selected](images/scientific-inverse-client-task4.png).
- Native screenshot inspection caught a clipped Grads label from Kvantum's larger radio spacing. The native caption-size regression failed before local indicator/spacing calibration and passed afterward; no global style setting was changed.
- The earlier persistent preview process remains the Task 3 build until reopened; test-created Scientific windows validate the newer binary without forcibly closing a window the operator may be using.

## 2026-10-19: Programmer mode (Task 5)

- Full current suite: eight CTest processes pass. Direct VM captures cover all four bases/widths, NOT, highest bits 7/15/31/63, narrowing, sign extension, rotations, arithmetic right shift and the too-large shift error.
- Original Programmer re-entry resets the radix to Dec but **preserves word width**. Width-limited bit rows are hidden, not filled with gray higher bits. The model/UI follow this observed behavior.
- [Native high-bit screenshot](images/programmer-high-bit-task5.png). Native QWindow-level mouse hit testing proves the shaped bit/radio control overlay does not block numeric keys beneath it.
- ASan/UBSan found signed negation overflow in retained `i32tonum(INT32_MIN)`. A dedicated minimum-signed-integer regression reproduced the sanitizer failure; unsigned magnitude arithmetic fixed it while preserving the sign. Programmer width/shift tests subsequently passed ASan/UBSan with `halt_on_error=1`. Leak detection was disabled for this arithmetic/UI sanitizer pass; it is not a leak audit.
- The display still uses a Consolas fallback and full visual calibration is pending.

## 2026-10-19: Statistics mode (Task 6)

- Full current suite: nine CTest processes pass. Aggregates, population/sample deviation, large-offset stability, empty/single sample behavior, Add/CAD, memory/Exp input, dataset persistence, inline editing and validated clipboard parsing are exercised.
- Original Add resets primary to zero; C/backspace do not remove samples. Original data selection does not recall a value. Arrow controls change selection; double-click opens inline editing. Re-entry preserves data, selects its last row and resets primary.
- “Enter data to calculate” is an informational notice, not an arithmetic lockout. Singleton population deviation is zero; singleton sample deviation shows that notice.
- Native Wayland screenshots: [populated](images/statistics-task6.png), [empty](images/statistics-empty-task6.png), [notice](images/statistics-notice-task6.png). Notice/result font calibration remains visibly incomplete.
- Inline edit commits through Qt's queued delegate event; the test awaits the committed model value. A preset-Statistics constructor regression also fixed a previously hidden keypad initialization-sentinel collision.
- Follow-up VM verification resolved the context clipboard behavior: Copy/Paste acts on the **calculator value**, not the selected dataset row. Copying primary9 while dataset row2 is selected, clearing the entry, then pasting restores9 without changing Count1. The earlier selected-sample/bulk-dataset assumption was removed. A real popup-menu regression failed with clipboard2 before the fix and passed with clipboard9 afterward. The same context menu is available over the result area; Clear dataset remains the dataset-specific action.

## Known incomplete work / visual differences

- History and supplementary actions are intentionally disabled in this intermediate four-mode executable; remaining plan tasks implement them. The shared engine/controller supports later integration.
- Segoe UI resolves correctly, but **Consolas is not installed**: Qt resolves that request to DejaVu Sans Mono. Result/expression glyphs therefore differ from Windows. No Microsoft font was copied/bundled or global font configuration changed. This must remain visible in the final differences report until resolved.
- Initial native focus rectangle and menu-mnemonic visibility differ from the clean original baseline; keyboard/mouse focus-state calibration remains in Task 11.
- Gradient, glyph and popup-menu calibration is not complete. Correct dimensions and passing input tests are not a pixel-perfect claim.
- Native startup logs contain pre-existing SVG warnings from the installed Kvantum theme. The missing embedded application-icon warning was fixed; theme resources were not edited.
- An operator-requested user-local **four-mode preview** was installed on 2026-10-19; see [preview-installation.md](preview-installation.md). That is not final packaging or full-fidelity acceptance. The follow-up below did not update the installed application or change defaults.

## 2026-10-25: clipboard repairs and packaging tooling

This is a bounded follow-up, not completion of the legacy feature plan. Integrated code/tooling commit: `e9fe2c857d03689200199637a866b8f467911836` on `feature/native-win7-calculator`.

- Programmer paste now parses plain signed integer text in the active radix. Copy/Clear/Paste preserves Hex `FF` and `10`, with regression coverage for Oct/Bin and signed/unsigned 8/16/32/64-bit boundaries. Malformed input preserves the current calculation. A related typed-negative bit-view exception was reproduced and corrected without changing the engine's numeric value.
- A successful Statistics paste clears the obsolete “Enter data to calculate” notice; a rejected paste preserves it. Pasting does not alter the dataset, and Add consumes the displayed value.
- Reversible preview installation tooling now provides a copied executable, desktop entry, licenses and a validated ownership manifest. Isolated tests cover update/removal, unsafe manifests, path quoting, preservation of pre-existing empty directories, and literal template tokens in prefixes. Updates replace individual files atomically; this is **not** a crash-safe multi-file transaction, and concurrent operations on one prefix are unsupported. Legacy/manual installations are refused rather than silently adopted.
- Independent task reviews accepted the clipboard repair and the packaging correction. A fresh external Release build of the combined source passed **10/10 CTest processes**, including eight isolated installation unittest cases. The original three clipboard diagnostic assertions all passed. Five UI/layout suites passed at 125%, 150% and 200%; the full ten-process suite also passed with Fusion. Existing retained-RatPack compiler warnings remain.
- Install tests used temporary HOME/XDG locations and prefixes; no actual user-local payload was replaced and no existing calculator process was restarted. Hosted CI, Windows/MSVC compatibility, fresh live Plasma interaction and new reference-image comparisons were not verified in this follow-up. The missing legacy features and visual/font differences listed above remain acceptance blockers.
