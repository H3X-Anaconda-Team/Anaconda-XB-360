#!/usr/bin/env python3
"""
Anaconda XB 360 - Catalog Generator
====================================

Scans Catalog/Content/<Type>/ for .7z files and rebuilds the matching
.ini file in Catalog/Categories/.

Usage:
    python Tools/generate-catalog/generate.py

Layout expected:
    Catalog/
    ├── Content/
    │   ├── Apps/*.7z
    │   ├── Games/*.7z
    │   ├── Emulators/*.7z
    │   └── Themes/*.7z
    └── Categories/
        ├── apps.ini        <- regenerated
        ├── games.ini       <- regenerated
        ├── emulators.ini   <- regenerated
        └── themes.ini      <- regenerated

Any existing .ini entries that do NOT have a matching .7z are kept,
so hand-written entries are never lost.
"""

import os
import re
import sys
from pathlib import Path

# ------------------------------------------------------------
# Configuration
# ------------------------------------------------------------

REPO_ROOT = Path(__file__).resolve().parents[2]

CONTENT_DIR    = REPO_ROOT / "Catalog" / "Content"
CATEGORIES_DIR = REPO_ROOT / "Catalog" / "Categories"

RAW_BASE = ("https://raw.githubusercontent.com/H3X-Anaconda-Team/"
            "anaconda-xb-360/main/Catalog/Content")

# Which folder maps to which category .ini and install path
CATEGORY_MAP = {
    "Apps":      {"ini": "apps.ini",      "path": "/Apps/"},
    "Games":     {"ini": "games.ini",     "path": "/Games/"},
    "Emulators": {"ini": "emulators.ini", "path": "/Emulators/"},
    "Themes":    {"ini": "themes.ini",    "path": "/Themes/"},
}

DEFAULT_AUTHOR      = "H3X Anaconda Team"
DEFAULT_VERSION     = "1.0"
DEFAULT_DESCRIPTION = "Auto-generated entry."

# ------------------------------------------------------------
# Helpers
# ------------------------------------------------------------

def read_existing_entries(ini_path: Path) -> dict:
    """Parse an existing .ini and return {section_name: {key: value}}."""
    entries = {}
    if not ini_path.exists():
        return entries

    current = None
    for line in ini_path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith(";") or line.startswith("#"):
            continue

        m = re.match(r"^\[(.+)\]$", line)
        if m:
            current = m.group(1)
            entries[current] = {}
            continue

        if current is not None and "=" in line:
            k, v = line.split("=", 1)
            entries[current][k.strip()] = v.strip()

    return entries


def write_ini(ini_path: Path, entries: dict):
    """Write entries back out in a stable, readable format."""
    lines = []
    lines.append("; ============================================================")
    lines.append("; Anaconda XB 360 - Auto-generated catalog")
    lines.append("; Edit anything you want - hand-written entries are preserved")
    lines.append("; on the next run as long as the section name is unique.")
    lines.append("; ============================================================")
    lines.append("")

    for name in sorted(entries.keys()):
        fields = entries[name]
        lines.append(f"[{name}]")
        for key in ("itemTitle", "itemVersion", "itemAuthor",
                    "itemDescription", "dataurl", "path", "reload"):
            if key in fields:
                lines.append(f"{key}={fields[key]}")
        lines.append("")

    ini_path.write_text("\n".join(lines), encoding="utf-8")


def find_7z_files(folder: Path):
    """Return a sorted list of .7z files in a folder (non-recursive)."""
    if not folder.exists():
        return []
    return sorted([f for f in folder.iterdir()
                   if f.is_file() and f.suffix.lower() == ".7z"])


# ------------------------------------------------------------
# Main
# ------------------------------------------------------------

def main():
    if not CONTENT_DIR.exists():
        print(f"Error: {CONTENT_DIR} does not exist")
        return 1

    CATEGORIES_DIR.mkdir(parents=True, exist_ok=True)

    total_added = 0

    for type_name, info in CATEGORY_MAP.items():
        folder     = CONTENT_DIR / type_name
        ini_path   = CATEGORIES_DIR / info["ini"]
        base_path  = info["path"]

        files      = find_7z_files(folder)
        existing   = read_existing_entries(ini_path)
        changed    = False

        for f in files:
            section = f.stem  # filename without .7z

            if section in existing:
                # Already have an entry — refresh only the dataurl
                # in case the path changed, but keep user's metadata.
                url = f"{RAW_BASE}/{type_name}/{f.name}"
                if existing[section].get("dataurl") != url:
                    existing[section]["dataurl"] = url
                    changed = True
                continue

            # New file — build a fresh entry
            existing[section] = {
                "itemTitle":       section,
                "itemVersion":     DEFAULT_VERSION,
                "itemAuthor":      DEFAULT_AUTHOR,
                "itemDescription": DEFAULT_DESCRIPTION,
                "dataurl":         f"{RAW_BASE}/{type_name}/{f.name}",
                "path":            f"{base_path}{section}/",
                "reload":          "True",
            }
            changed = True
            total_added += 1

        if changed:
            write_ini(ini_path, existing)
            print(f"Updated {ini_path.relative_to(REPO_ROOT)}"
                  f" ({len(files)} file(s), {len(existing)} entries)")
        else:
            print(f"No change: {ini_path.relative_to(REPO_ROOT)}")

    print(f"\nDone. {total_added} new entry/entries added.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
