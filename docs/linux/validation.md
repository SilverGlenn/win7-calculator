# Native Linux validation — in progress

This report records implementation evidence, **not final fidelity acceptance**.

## 2026-10-19: first Standard window (Task 3)

- Current headless suite: six CTest processes (engine, numeric, separate comma locale, controller, Standard UI, layout), all passing.
- Standard UI suite also passed with `QT_SCALE_FACTOR=1.25`, `QT_SCALE_FACTOR=1.5`, and the Fusion fallback style. Larger/missing-family font tests preserve calculator access.
- Native Wayland launch uses the installed `kvantum` style and Segoe UI 9. KWin reports `noBorder: false`, `skipTaskbar: false`, `desktopFile/resourceClass: org.win7calculator.Calculator`, `resourceName: win7-calculator`, outer geometry 228×322. Size/Maximize behavior is constrained by the fixed client size, matching the original system-menu evidence.
- Actual client screenshot: [`images/standard-client-task3.png`](images/standard-client-task3.png).
- Actual system-decorated window screenshot: [`images/standard-native-frame-task3.png`](images/standard-native-frame-task3.png). Captured via Spectacle with the calculator temporarily activated; prior focus was restored. The frame is the operator's installed SMOD decoration, not a painted application imitation.
- Embedded help/icon resources were tested after discovering static-library dead stripping: the new regression failed before explicit resource initialization and passed afterward.
- The four-mode/seven-panel VM matrix and original fixed-size system menu are saved under `docs/reference/windows7-calculator/states/` and `tests/linux/fixtures/ui-states.json`.

## Known incomplete work / visual differences

- Scientific, Programmer, Statistics, history and supplementary actions are intentionally disabled in this intermediate Standard-only executable; remaining plan tasks implement them. The shared engine/controller supports later integration.
- Segoe UI resolves correctly, but **Consolas is not installed**: Qt resolves that request to DejaVu Sans Mono. Result/expression glyphs therefore differ from Windows. No Microsoft font was copied/bundled or global font configuration changed. This must remain visible in the final differences report until resolved.
- Initial native focus rectangle and menu-mnemonic visibility differ from the clean original baseline; keyboard/mouse focus-state calibration remains in Task 11.
- Gradient, glyph and popup-menu calibration is not complete. Correct dimensions and passing input tests are not a pixel-perfect claim.
- Native startup logs contain pre-existing SVG warnings from the installed Kvantum theme. The missing embedded application-icon warning was fixed; theme resources were not edited.
- No user-local installation, default-app change or final whole-branch review has happened yet.
