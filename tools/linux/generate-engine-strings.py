#!/usr/bin/env python3
"""Generate a Qt-free C++ table and audit JSON from MIT-licensed engine strings."""
import argparse
import json
from pathlib import Path
import re
import xml.etree.ElementTree as ET

def generate(source, ids, header, metadata):
    values = {}
    for item in ET.parse(source).getroot().findall("data"):
        key = item.attrib["name"]
        if key in values:
            raise ValueError(f"Duplicate resource ID: {key}")
        values[key] = item.findtext("value", "")
    # The upstream UI never requests these missing labels. Define them explicitly
    # so the standalone provider has a complete table, including legacy errors.
    defaults = {"0": "±", "1": "C", "3": "Backspace", "5": "",
                "31": "sqr", "32": "cube", "33": "fact", "34": "reciproc",
                "39": "F-E", "42": "MC", "43": "MR", "44": "MS",
                "45": "M+", "46": "M-", "93": "cuberoot", "98": "Rsh",
                "102": "Infinity", "103": "-Infinity", "104": "Calculation aborted",
                "106": "Too many digits", "109": "Insufficient data"}
    defaults.update({str(50+i): str(i) if i < 10 else chr(ord('A')+i-10) for i in range(16)})
    for key, value in defaults.items():
        values.setdefault(key, value)
    required = re.findall(r'inline constexpr auto SIDS_\w+ = L"([^"]+)";', Path(ids).read_text())
    # Parser-only messages are not shipped by the upstream command engine.
    for key in required:
        if key.isdigit() and 111 <= int(key) <= 117:
            values.setdefault(key, "Invalid input")
    missing = sorted(set(required) - values.keys())
    if missing:
        raise ValueError(f"Missing engine resources: {missing}")
    def literal(s):
        return "L" + json.dumps(s, ensure_ascii=False)
    entries = ",\n".join("  {" + literal(k) + ", " + literal(v) + "}" for k,v in sorted(values.items()))
    Path(header).parent.mkdir(parents=True, exist_ok=True)
    Path(header).write_text("// Generated from Microsoft Calculator MIT resources; do not edit.\n"
                           "#pragma once\n#include <utility>\n#include <string_view>\n"
                           "inline constexpr std::pair<std::wstring_view,std::wstring_view> engineStringsData[] = {\n"
                           + entries + "\n};\n")
    Path(metadata).write_text(json.dumps(values, ensure_ascii=False, indent=2) + "\n")

if __name__ == "__main__":
    p = argparse.ArgumentParser()
    for name in ("source", "ids", "header", "metadata"):
        p.add_argument("--" + name, required=True)
    a = p.parse_args()
    generate(a.source, a.ids, a.header, a.metadata)
