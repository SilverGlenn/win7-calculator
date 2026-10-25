# Windows 7 history reference follow-up — 2026-10-25

Captured by the parent from the operator-authorized offline Windows 7 VM using its unchanged local QMP controller. Only Calculator and the Run dialog used to launch it were operated. No settings or other applications were changed. Guest resolution:1440×900; Basic graphics. Window origin:(125,125). Crops are unscaled; popup captures include their full visible extent. These are evidence, not shipped application artwork.

## Verified behavior

- History rows contain expressions; selecting a row recalls its result without adding a calculation.
- F2 opens an inline whole-expression editor. Enter recalculates and replaces only the selected row. Esc restores the committed entry/result. Keypad clicks insert into the draft.
- Syntax and domain failures keep the draft open and retain the previous committed result: `Unexpected end of expression`, `Cannot divide by zero`, `Unknown function`.
- Ctrl+Shift+D clears history without clearing the current nonzero result.
- Standard and Scientific have separate history stores. Returning to Standard retains rows, selects the newest, and resets the primary display to0.
- Home/End navigate to the earliest/latest history entry. All35 generated unique entries remained accessible; **35 is a lower bound, not an established maximum**. A20-item or32-item cap would contradict these observations.
- Standard edited `2+3*4` evaluates immediately to20. Scientific angle-specific history spelling is `sind(30)`; `sin(60)` is rejected while `sind(60)` and `sqrt(9)` commit successfully.

## Capture index

| Capture | Evidence |
|---|---|
| `01-selected` | Earlier Standard entry recalled as15 |
| `02-menu` | History actions and original shortcuts |
| `03-editor`, `04-selected-text` | Inline whole-expression editing |
| `05-draft`, `06-committed` | `12+3` changed to `9+2`, result11 |
| `07-cancel-draft`, `08-cancelled` | Esc restores committed state |
| `09-incomplete`, `10-divide-zero` | Rejected draft with original tooltip |
| `11-keypad-editor` | Keypad entry routed to the draft |
| `12-scientific`, `13-standard-return` | Separate stores and re-entry display reset |
| `14-thirty-five`, `15-earliest`, `16-latest` | Scrolling/navigation and retained lower bound35 |
| `17-clear` | Clear preserves primary1035 |
| `18-standard-precedence` | Edited expression evaluates to20 |
| `19-scientific-sine`, `20-scientific-editor` | Original `sind(30)` notation |
| `21-unknown-function`, `22-sind60`, `23-sqrt` | Scientific edit grammar and result examples |

Machine-readable observations: [`tests/linux/fixtures/history-oracle.json`](../../../../tests/linux/fixtures/history-oracle.json).

## Remaining evidence gaps

Exact Copy history serialization was **not** established. Pasting its output into Calculator's own single-line history editor yielded a blank draft, while an ordinary numeric Copy/Paste positive control worked. That does not distinguish leading line breaks from other single-line behavior. No external guest clipboard viewer/helper was launched. Ultimate capacity above35 and the complete Scientific function-name inventory remain unmeasured. These gaps must not be represented as verified parity.

A timed-out screenshot-settling batch was inspected in place; only visually verified completed captures are retained here. No interrupted input sequence was blindly repeated.
