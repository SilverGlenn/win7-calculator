# Exact screenshot comparison (standalone tooling)

This utility supports the later Task11 screenshot gate. It does **not** establish
visual parity, calibrate the UI, complete Task11, or replace human inspection.
It uses only Python's standard library and the already available Pillow library.
Tests generate synthetic pixels, borders and gradients, not Windows artwork or
golden screenshots. No application, CMake or CI integration is added here.

## CLI

From the repository root:

```sh
PYTHONPYCACHEPREFIX=/tmp/win7-calculator-team.4QdZ3y/image-compare-pycache \
  python3 tools/linux/compare-screenshots.py reference.png actual.png \
  --output-dir /tmp/calculator-comparison
```

Optional `--regions regions.json` reads a UTF-8 JSON file containing a mapping:

```json
{
  "display": [10, 25, 190, 50],
  "key": [10, 100, 34, 27]
}
```

Rectangles are `[x, y, width, height]`, with zero-based coordinates, inclusive
origins and exclusive right/bottom edges. Coordinates must be integers (not
booleans or floats); width/height must be positive; the entire rectangle must be
in bounds. Names must be nonempty strings. An empty mapping is allowed. Named
regions add diagnostics: **the full image is always measured**, so a clean
selected region cannot hide a difference elsewhere.

The CLI prints raw and compared changed-pixel counts plus the excluded pixel
count. Status codes are:

- **0:** all global compared pixels match exactly (raw differences may remain
  inside the explicitly recorded Modern footprint).
- **1:** at least one global compared pixel differs.
- **2:** invalid input/usage or an I/O error; this is not a successful comparison.

Use a fresh output directory per comparison. Validation does not create or
rewrite artifacts on invalid input; files from an earlier run in a reused
output directory are not evidence for that invalid run. An output I/O failure
can leave partial artifacts; only a completed run and its exit status establish
that the report was written successfully.

## Exact pixels and output artifacts

Inputs must be readable RGB or fully opaque RGBA images of exactly matching
sizes. Opaque alpha is removed without changing RGB values. Nonopaque alpha,
other image modes, size mismatches and invalid rectangles are rejected rather
than silently flattened, resized or clipped. There is no resampling,
align-to-fit, threshold, fuzzy text mask or golden regeneration. RGB bytes are
compared as stored, with no ICC/gamma conversion or EXIF orientation transform.

Every valid comparison writes these files at the original dimensions:

- `overlay.png`: a 50/50 reference/actual RGB blend (Pillow integer rounding).
- `diff.png`: absolute per-channel RGB differences, **without auto-contrast**.
- `metrics.json`: the complete metrics below, even when exit status is 1.

Neither output image is masked. Raw text, borders and all other differences
remain visible even when the Modern footprint is excluded from acceptance.
Artifact paths must not overwrite either input: direct path, symlink and
existing hardlink collisions are rejected before writing any artifacts. This
is a local offline tool, not a defense against concurrent hostile filesystem
changes; keep inputs/output paths stable during the run.

## Only permitted exclusion: measured Modern menu

`--modern-menu-rect X Y W H` explicitly excludes a measured rectangle from
**comparison** metrics only. It is solely for the added **Modern** menu at the
**100% baseline**. The caller must measure the footprint; the tool never guesses
it. The rectangle must be in bounds, at most **64 pixels wide**, and entirely
inside the **top 19 pixels** (`y >= 0`, `y + height <= 19`). It must leave at
least one global compared pixel. The limits are bounds, not a recommendation to
exclude the entire allowed area: supply only the actual narrow menu footprint.

For example, append `--modern-menu-rect 150 0 50 19` only if that is the measured
footprint in both same-size baseline captures. This is **not** a workaround for
other menu, display-text, key-border, color or layout mismatches. No general
mask API exists. A single differing pixel immediately outside the supplied
rectangle still makes the global comparison fail. Do not extrapolate this
exclusion to larger scale captures.

## Python API and metrics schema

The hyphenated filename can be loaded with `importlib.util.spec_from_file_location`.
The public function is:

```python
compare_images(reference_path, actual_path, output_dir,
               regions=None, modern_menu_rect=None) -> dict
```

Paths may be strings or `pathlib.Path` objects; `regions` is the mapping itself,
not a JSON path. Rectangles may be lists or tuples. Invalid images/parameters or
input/artifact collisions raise `ValueError`; output filesystem errors can
raise `OSError`. The CLI translates these to exit 2.

Version 1 has the following top-level fields:

- `version`: `1`.
- `size`: `[width, height]`; `channels`: `3`.
- `exclusions`: `[]`, or exactly one object such as
  `{"kind": "modern_menu_rect", "rect": [150, 0, 50, 19], "pixelCount": 950}`.
- `raw`, `comparison`: metric objects for the **full image**, respectively
  including every pixel and omitting only the explicit exclusion.
- `identicalRaw`, `identicalCompared`: whether the respective global
  `changedPixelCount` is zero. Global comparison is never empty.
- `regions`: mapping from requested names to objects with `rect`, `raw` and
  `comparison`, using the same metric definitions within that region.

Each metric object contains:

| Field | Definition |
| --- | --- |
| `pixelCount` | Number of measured pixels, not channels |
| `changedPixelCount` | Pixels where **any** RGB channel differs |
| `meanAbsoluteError` | Sum of absolute RGB channel errors / pixelCount / 3 |
| `maxAbsoluteError` | Largest absolute channel error (0–255) |

For a 2×2 image with one red channel increased by 6, raw metrics are
`pixelCount: 4`, `changedPixelCount: 1`, `meanAbsoluteError: 0.5`,
`maxAbsoluteError: 6`. Identical nonempty data has mean/max zero. A region fully
covered by the Modern exclusion has comparison `pixelCount: 0`,
`changedPixelCount: 0`, and **null** mean/max; it is not a fake zero-error
acceptance. Its raw metrics remain populated.

## Synthetic verification

```sh
export PYTHONPYCACHEPREFIX=/tmp/win7-calculator-team.4QdZ3y/image-compare-pycache
python3 -m unittest discover -s tests/linux -p 'test_*.py' -v
python3 -m py_compile tools/linux/compare-screenshots.py tests/linux/test_image_compare.py
```

The focused suite checks exact channel arithmetic, unchanged/shifted borders,
wrong gradient color, region/global independence, the narrow Modern boundary,
empty-region semantics, invalid input, CLI statuses/artifacts and input
immutability. Qt/installer/application integration and real captures remain
host-owned checks after independent component review. Passing these synthetic
tests does not imply fidelity to the Windows 7 reference.
