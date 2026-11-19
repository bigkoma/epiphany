#!/usr/bin/env python3
"""Reuse the elementary OS Flatpak recipe with a local Epiphany source tree."""

import argparse
import json
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("elementary_browser", type=Path)
    parser.add_argument("epiphany_source", type=Path)
    args = parser.parse_args()

    recipe_dir = args.elementary_browser.resolve()
    source_dir = args.epiphany_source.resolve()
    manifest = json.loads((recipe_dir / "org.gnome.epiphany.json").read_text())
    epiphany = next(module for module in manifest["modules"] if module["name"] == "epiphany")
    epiphany["sources"][0] = {
        "type": "dir",
        "path": str(source_dir),
        "skip": [".git", "build"]
    }
    epiphany["config-opts"].append("-Dunit_tests=disabled")
    manifest["finish-args"].append("--env=PRIMETAB_PANEL_BROWSER=1")
    print(json.dumps(manifest, indent=4))


if __name__ == "__main__":
    main()
