#!/usr/bin/env python3
"""Exact RGB screenshot comparison with one explicit, bounded Modern exclusion."""
import argparse
import json
import os
from pathlib import Path
import sys

from PIL import Image, ImageChops


def _rectangle(value, size, label):
    if (not isinstance(value, (list, tuple)) or len(value) != 4
            or any(type(component) is not int for component in value)):
        raise ValueError(f"{label} must contain exactly four integers: x y width height")
    x, y, width, height = value
    if (x < 0 or y < 0 or width <= 0 or height <= 0
            or x + width > size[0] or y + height > size[1]):
        raise ValueError(f"{label} must be positive and within the image bounds")
    return list(value)


def _read_rgb(path):
    try:
        with Image.open(path) as image:
            image.load()
            if image.mode not in ("RGB", "RGBA"):
                raise ValueError(f"{path}: only RGB and opaque RGBA are supported")
            if image.mode == "RGBA" and image.getextrema()[3] != (255, 255):
                raise ValueError(f"{path}: nonopaque alpha is not supported")
            return image.convert("RGB")
    except (OSError, Image.DecompressionBombError) as error:
        raise ValueError(f"cannot read image {path}: {error}") from error


def _metrics(diff, rect, excluded=None):
    x, y, width, height = rect
    pixels = diff.load()
    count = changed = total = maximum = 0
    for row in range(y, y + height):
        for column in range(x, x + width):
            if excluded is not None:
                ex, ey, ew, eh = excluded
                if ex <= column < ex + ew and ey <= row < ey + eh:
                    continue
            channels = pixels[column, row]
            count += 1
            changed += any(channels)
            total += sum(channels)
            maximum = max(maximum, *channels)
    return {
        "pixelCount": count,
        "changedPixelCount": changed,
        "meanAbsoluteError": total / count / 3 if count else None,
        "maxAbsoluteError": maximum if count else None,
    }


def _protect_inputs(inputs, artifacts):
    for artifact in artifacts:
        for source in inputs:
            if artifact.resolve() == source.resolve():
                raise ValueError(f"output artifact would overwrite input: {source}")
            if artifact.exists() and os.path.samefile(artifact, source):
                raise ValueError(f"output artifact aliases input: {source}")


def compare_images(reference_path, actual_path, output_dir, regions=None, modern_menu_rect=None):
    """Write raw artifacts and return version-1 metrics; invalid input raises ValueError.

    Regions are a name-to-[x,y,width,height] mapping, never a global mask.
    The sole optional exclusion is a measured Modern menu footprint at 100% scale.
    """
    reference_path, actual_path = Path(reference_path), Path(actual_path)
    reference, actual = _read_rgb(reference_path), _read_rgb(actual_path)
    if reference.size != actual.size:
        raise ValueError(f"image dimensions differ: {reference.size} vs {actual.size}")
    size = reference.size
    if regions is None:
        regions = {}
    if not isinstance(regions, dict):
        raise ValueError("regions must be a mapping of names to rectangles")
    checked_regions = {}
    for name, rect in regions.items():
        if not isinstance(name, str) or not name:
            raise ValueError("region names must be nonempty strings")
        checked_regions[name] = _rectangle(rect, size, f"region {name!r}")
    excluded = None
    exclusions = []
    if modern_menu_rect is not None:
        excluded = _rectangle(modern_menu_rect, size, "modern_menu_rect")
        x, y, width, height = excluded
        if width > 64 or y + height > 19:
            raise ValueError("Modern exclusion must be at most 64px wide and within the top 19px")
        if width * height >= size[0] * size[1]:
            raise ValueError("Modern exclusion must leave global compared pixels")
        exclusions = [{"kind": "modern_menu_rect", "rect": excluded,
                       "pixelCount": width * height}]
    output_dir = Path(output_dir)
    artifacts = [output_dir / name for name in ("overlay.png", "diff.png", "metrics.json")]
    _protect_inputs((reference_path, actual_path), artifacts)
    diff = ImageChops.difference(reference, actual)
    full = [0, 0, *size]
    raw = _metrics(diff, full)
    comparison = _metrics(diff, full, excluded)
    result = {
        "version": 1,
        "size": list(size),
        "channels": 3,
        "exclusions": exclusions,
        "raw": raw,
        "comparison": comparison,
        "identicalRaw": raw["changedPixelCount"] == 0,
        "identicalCompared": comparison["changedPixelCount"] == 0,
        "regions": {
            name: {"rect": rect, "raw": _metrics(diff, rect),
                   "comparison": _metrics(diff, rect, excluded)}
            for name, rect in checked_regions.items()
        },
    }
    # Validate all inputs/collisions before creating any output. Images are always raw.
    output_dir.mkdir(parents=True, exist_ok=True)
    Image.blend(reference, actual, 0.5).save(artifacts[0])
    diff.save(artifacts[1])
    artifacts[2].write_text(json.dumps(result, indent=2, allow_nan=False) + "\n", encoding="utf-8")
    return result


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reference", type=Path)
    parser.add_argument("actual", type=Path)
    parser.add_argument("--output-dir", required=True, type=Path)
    parser.add_argument("--regions", type=Path, metavar="JSON",
                        help="UTF-8 JSON file mapping region names to [x,y,width,height]")
    parser.add_argument("--modern-menu-rect", nargs=4, type=int, metavar=("X", "Y", "W", "H"))
    args = parser.parse_args(argv)
    try:
        regions = (json.loads(args.regions.read_text(encoding="utf-8"))
                   if args.regions is not None else None)
        result = compare_images(args.reference, args.actual, args.output_dir,
                                regions=regions, modern_menu_rect=args.modern_menu_rect)
    except (ValueError, OSError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2
    raw, compared = result["raw"], result["comparison"]
    excluded_count = sum(entry["pixelCount"] for entry in result["exclusions"])
    print(f"raw: {raw['changedPixelCount']}/{raw['pixelCount']} changed; "
          f"compared: {compared['changedPixelCount']}/{compared['pixelCount']} changed; "
          f"excluded Modern pixels: {excluded_count}")
    return 0 if result["identicalCompared"] else 1


if __name__ == "__main__":
    sys.exit(main())
