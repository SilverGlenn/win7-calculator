# Windows 7 unit conversion reference — 2026-10-25

Captured by the parent from the operator-authorized offline VM, using only Calculator. No settings, other guest applications, clipboard viewer, network, or shared folders were used. Guest resolution:1440×900; Basic graphics. Standard+conversion outer bounds:(125,125),602×322. Native panel crops begin at(345,185),363×241. All reference images are unscaled.

## Inventory and evidence

- `00-panel.png` and `01-categories.png`: original form, placeholder and all11 category labels/order.
- `<category>-inventory.png`: complete From-unit list in each category, including original capitalization/pluralization. Longer dropdowns extend below the window; their crops retain the full popup.
- `unit-case-<category>-NN.png`: one source-unit-to-selected-base conversion for every unit, input1. **95 unit labels and95 such conversions** were captured.
- `unit-extra-*.png`:10 additional numeric/notation cases and3 input-filter/error cases.

Machine-readable inventory and all105 numeric cases: [`tests/linux/fixtures/units.json`](../../../../tests/linux/fixtures/units.json). There are121 native evidence images. Parent visually verified all dropdowns and the95 original source/result strips; host-installed OCR assisted transcription only. Three omitted decimal points were restored by reading native pixels. OCR diagnostic upscaling/contact sheets are not reference images and are not shipped here.

## Legacy definitions are the authority

Do not silently replace these measured Windows7 values with current physical constants or modern converter tables:

| Input / conversion | Original displayed result |
|---|---|
|1 British Thermal Unit → Joule|1055.056|
|1 Electron-Volts → Joule|1.60217653e-19|
|1 PICA → Meter|0.0042175176|
|1 Millimeter of mercury → Pascal|133.3|
|1 Pound per square inch (PSI) → Pascal|6894.757|
|1 Mach (at std. atm) → Meter per second|340.2933|
|3 Foot-Pound → Joule|4.0674538449942|

These are display observations, not a claim to have inspected the original implementation's internal constants. The extra Foot-Pound case distinguishes its legacy value from a more precise modern physical calculation. Repeating ratios must not be replaced by their rounded display strings when an exact ratio is known and supported by the captures.

## Form and notation behavior

- Original labels are `Select the type of unit you want to convert`, `From`, and `To`. Empty/unfocused From has italic `Enter value`; the To field is read-only. There is no Convert button; changes update the result.
- Input `1e3` is accepted and retains scientific output notation: Degree→Radian gives `1.745329251994329e+1`, while plain `1000` gives `17.45329251994329`. Identity conversion of `1e3` shows `1.e+3`.
-0 Celsius→Fahrenheit gives32;−273.15 Celsius→Fahrenheit gives−459.67. Negative Kelvin is not rejected on physical-domain grounds:−1 Kelvin→Celsius gives−274.15.
- Typing `abc` leaves the input empty. `2+3` remains visible but produces no result and the balloon `This value is not valid.`. Clearing the input clears the output. These are widget observations, not permission to evaluate expressions.

This inventory does not enable the Linux panel, establish exact font/paint parity, or complete Task8. Numeric logic, interaction/focus routing and native integration still require their regression tests. The separate unresolved history-clipboard observation is not answered by these captures.
