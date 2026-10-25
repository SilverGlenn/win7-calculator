"""Synthetic-pixel contract tests; no Windows artwork or screenshot goldens."""
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from PIL import Image, ImageDraw


TOOL = Path(__file__).resolve().parents[2] / "tools/linux/compare-screenshots.py"


class ImageCompareTest(unittest.TestCase):
    def setUp(self):
        self.assertTrue(TOOL.is_file(), "screenshot comparison tool is missing")
        spec = importlib.util.spec_from_file_location("image_compare", TOOL)
        self.tool = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(self.tool)
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.reference = self.root / "reference.png"
        self.actual = self.root / "actual.png"
        self.output = self.root / "output"
        self.save_pair(Image.new("RGB", (80, 24), (10, 20, 30)))

    def save_pair(self, reference, actual=None):
        reference.save(self.reference)
        (reference if actual is None else actual).save(self.actual)

    def compare(self, **kwargs):
        return self.tool.compare_images(self.reference, self.actual, self.output, **kwargs)

    def cli(self, *args):
        return subprocess.run(
            [sys.executable, str(TOOL), str(self.reference), str(self.actual),
             "--output-dir", str(self.output), *map(str, args)],
            capture_output=True, text=True, env=os.environ.copy(), check=False,
        )

    def assert_invalid(self, **kwargs):
        with self.assertRaises(ValueError):
            self.compare(**kwargs)
        self.assertFalse(self.output.exists(), "invalid input must not write a report")

    def test_identical_rgb_and_opaque_rgba(self):
        for mode in ("RGB", "RGBA"):
            with self.subTest(mode=mode):
                self.save_pair(Image.new(mode, (2, 2), (10, 20, 30)))
                result = self.compare()
                self.assertEqual(result.get("version"), 1)
                self.assertEqual(result["size"], [2, 2])
                self.assertEqual(result["channels"], 3)
                self.assertTrue(result["identicalRaw"])
                self.assertTrue(result["identicalCompared"])
                self.assertEqual(result["exclusions"], [])
                self.assertEqual(result["raw"], {
                    "pixelCount": 4, "changedPixelCount": 0,
                    "meanAbsoluteError": 0, "maxAbsoluteError": 0})
                self.assertEqual(result["comparison"], result["raw"])

    def test_one_channel_error_and_unscaled_artifacts(self):
        reference = Image.new("RGB", (2, 2), (10, 20, 30))
        actual = reference.copy()
        actual.putpixel((1, 0), (16, 20, 30))
        self.save_pair(reference, actual)
        result = self.compare()
        self.assertEqual(result["raw"], {
            "pixelCount": 4, "changedPixelCount": 1,
            "meanAbsoluteError": 0.5, "maxAbsoluteError": 6})
        self.assertFalse(result["identicalRaw"])
        self.assertFalse(result["identicalCompared"])
        with Image.open(self.output / "diff.png") as diff:
            self.assertEqual(diff.size, (2, 2))
            self.assertEqual(diff.getpixel((1, 0)), (6, 0, 0))
            self.assertEqual(diff.getpixel((0, 0)), (0, 0, 0))
        with Image.open(self.output / "overlay.png") as overlay:
            self.assertEqual(overlay.size, (2, 2))
            self.assertEqual(overlay.getpixel((1, 0)), (13, 20, 30))
        self.assertEqual(json.loads((self.output / "metrics.json").read_text()), result)

    def test_shifted_key_border_is_not_aligned_away(self):
        reference = Image.new("RGB", (12, 12), "white")
        actual = reference.copy()
        ImageDraw.Draw(reference).rectangle((2, 2, 8, 8), outline="black")
        ImageDraw.Draw(actual).rectangle((3, 2, 9, 8), outline="black")
        self.save_pair(reference, actual)
        result = self.compare()
        self.assertFalse(result["identicalCompared"])
        self.assertEqual(result["raw"]["changedPixelCount"], 24)
        self.assertEqual(result["raw"]["maxAbsoluteError"], 255)

    def test_wrong_gradient_color_is_detected(self):
        reference = Image.new("RGB", (4, 4))
        actual = reference.copy()
        for y in range(4):
            for x in range(4):
                reference.putpixel((x, y), (10 + y, 20 + y, 30 + y))
                actual.putpixel((x, y), (10 + y, 22 + y, 30 + y))
        self.save_pair(reference, actual)
        result = self.compare()
        self.assertEqual(result["raw"]["changedPixelCount"], 16)
        self.assertAlmostEqual(result["raw"]["meanAbsoluteError"], 2 / 3)
        self.assertFalse(result["identicalCompared"])

    def test_size_mismatch_rejected_without_report(self):
        Image.new("RGB", (79, 24)).save(self.actual)
        self.assert_invalid()

    def test_regions_do_not_hide_global_errors(self):
        actual = Image.open(self.actual)
        self.addCleanup(actual.close)
        actual.putpixel((79, 23), (13, 20, 30))
        actual.save(self.actual)
        result = self.compare(regions={"key": [0, 19, 2, 2]})
        self.assertFalse(result["identicalCompared"])
        self.assertEqual(result["raw"]["changedPixelCount"], 1)
        region = result["regions"]["key"]
        self.assertEqual(region["rect"], [0, 19, 2, 2])
        self.assertEqual(region["raw"]["pixelCount"], 4)
        self.assertEqual(region["comparison"]["changedPixelCount"], 0)

    def test_modern_exclusion_keeps_raw_and_empty_region_visible(self):
        reference = Image.new("RGB", (80, 24))
        actual = reference.copy()
        ImageDraw.Draw(actual).rectangle((12, 0, 75, 18), fill=(3, 6, 9))
        self.save_pair(reference, actual)
        result = self.compare(modern_menu_rect=[12, 0, 64, 19],
                              regions={"modern": [12, 0, 64, 19]})
        self.assertFalse(result["identicalRaw"])
        self.assertTrue(result["identicalCompared"])
        self.assertEqual(result["raw"]["changedPixelCount"], 1216)
        self.assertEqual(result["comparison"]["pixelCount"], 704)
        self.assertEqual(result["exclusions"], [
            {"kind": "modern_menu_rect", "rect": [12, 0, 64, 19], "pixelCount": 1216}])
        self.assertEqual(result["regions"]["modern"]["comparison"], {
            "pixelCount": 0, "changedPixelCount": 0,
            "meanAbsoluteError": None, "maxAbsoluteError": None})
        with Image.open(self.output / "diff.png") as diff:
            self.assertEqual(diff.getpixel((12, 0)), (3, 6, 9))
        actual.putpixel((76, 0), (1, 0, 0))
        actual.save(self.actual)
        self.assertFalse(self.compare(modern_menu_rect=[12, 0, 64, 19])["identicalCompared"])

    def test_invalid_modern_rectangles(self):
        for rect in ([0, 0, 65, 19], [0, 1, 64, 19], [0, 19, 1, 1],
                     [0, 0, 0, 1], [-1, 0, 1, 1], [80, 0, 1, 1],
                     [0, 0, 1.0, 1], [False, 0, 1, 1], [], "0,0,1,1"):
            with self.subTest(rect=rect):
                self.assert_invalid(modern_menu_rect=rect)

    def test_no_global_pixels_left_is_rejected(self):
        self.save_pair(Image.new("RGB", (4, 4)))
        self.assert_invalid(modern_menu_rect=[0, 0, 4, 4])

    def test_invalid_regions(self):
        for regions in ({"bad": []}, {"bad": [0, 0, 0, 1]},
                        {"bad": [0, -1, 1, 1]}, {"bad": [79, 23, 2, 1]},
                        {"bad": [0, 0, 1, 1.5]}, {"bad": [0, 0, True, 1]},
                        {"": [0, 0, 1, 1]}, [], {1: [0, 0, 1, 1]}):
            with self.subTest(regions=regions):
                self.assert_invalid(regions=regions)

    def test_nonopaque_alpha_rejected_on_either_input(self):
        for path in (self.reference, self.actual):
            with self.subTest(path=path):
                self.save_pair(Image.new("RGB", (80, 24)))
                rgba = Image.new("RGBA", (80, 24), (0, 0, 0, 255))
                rgba.putpixel((2, 2), (0, 0, 0, 254))
                rgba.save(path)
                self.assert_invalid()

    def test_unreadable_and_unsupported_input(self):
        self.actual.write_bytes(b"not an image")
        self.assert_invalid()
        Image.new("L", (80, 24)).save(self.actual)
        self.assert_invalid()
        self.actual.unlink()
        self.assert_invalid()

    def test_cli_statuses_summary_dimensions_and_input_immutability(self):
        before = (self.reference.read_bytes(), self.actual.read_bytes())
        passed = self.cli()
        self.assertEqual(passed.returncode, 0, passed.stderr)
        self.assertIn("raw", passed.stdout.lower())
        self.assertIn("compared", passed.stdout.lower())
        for name in ("overlay.png", "diff.png"):
            with Image.open(self.output / name) as image:
                self.assertEqual(image.size, (80, 24))
        self.assertEqual(before, (self.reference.read_bytes(), self.actual.read_bytes()))
        actual = Image.new("RGB", (80, 24), (10, 20, 30))
        actual.putpixel((0, 0), (11, 20, 30))
        actual.save(self.actual)
        failed = self.cli()
        self.assertEqual(failed.returncode, 1, failed.stderr)
        excluded = self.cli("--modern-menu-rect", 0, 0, 1, 1)
        self.assertEqual(excluded.returncode, 0, excluded.stderr)
        self.assertIn("excluded", excluded.stdout.lower())
        for args in (("--modern-menu-rect", 0, 0, 65, 1),
                     ("--regions", "not-json"), ("--unknown",)):
            invalid = self.cli(*args)
            self.assertEqual(invalid.returncode, 2, invalid.stderr)

    def test_cli_regions_json(self):
        regions = self.root / "regions.json"
        regions.write_text(json.dumps({"display": [0, 19, 5, 5]}))
        completed = self.cli("--regions", regions)
        self.assertEqual(completed.returncode, 0, completed.stderr)
        metrics = json.loads((self.output / "metrics.json").read_text())
        self.assertEqual(metrics["regions"]["display"]["raw"]["pixelCount"], 25)

    def test_artifact_path_collisions_never_overwrite_inputs(self):
        for name in ("overlay.png", "diff.png", "metrics.json"):
            for which in ("reference", "actual"):
                with self.subTest(name=name, which=which):
                    directory = self.root / (name + which)
                    directory.mkdir()
                    collision = directory / name
                    collision.write_bytes(self.reference.read_bytes())
                    reference = collision if which == "reference" else self.reference
                    actual = collision if which == "actual" else self.actual
                    before = collision.read_bytes()
                    with self.assertRaises(ValueError):
                        self.tool.compare_images(reference, actual, directory)
                    self.assertEqual(collision.read_bytes(), before)
                    completed = subprocess.run(
                        [sys.executable, str(TOOL), str(reference), str(actual),
                         "--output-dir", str(directory)], capture_output=True, text=True)
                    self.assertEqual(completed.returncode, 2, completed.stderr)
                    self.assertEqual(collision.read_bytes(), before)

    def test_symlink_and_hardlink_artifact_collisions(self):
        self.output.mkdir()
        for link in ("symlink", "hardlink"):
            with self.subTest(link=link):
                artifact = self.output / "diff.png"
                if link == "symlink":
                    artifact.symlink_to(self.reference)
                else:
                    os.link(self.reference, artifact)
                before = self.reference.read_bytes()
                with self.assertRaises(ValueError):
                    self.compare()
                self.assertEqual(self.reference.read_bytes(), before)
                artifact.unlink()


if __name__ == "__main__":
    unittest.main()
