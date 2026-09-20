#!/usr/bin/env python3
"""
Anaconda XB 360 - Catalog Generator
====================================

Scans Catalog/Content/<Type>/ for .7z files and rebuilds the matching
.ini file in Catalog/Categories/.

Reads settings from config.json in this same folder.

Usage:
    python Tools/generate-catalog/generate.py

Any existing .ini entries that do NOT have a matching .7z are kept,
so hand-written entries are never lost.
"""

import json
import re
import sys
from pathlib import Path

# ------------------------------------------------------------
# Load config
# ------------------------------------------------------------

HERE = Path(__file__).resolve().parent
REPO_ROOT = HERE.parents[1]

CONFIG_PATH = HERE / "config.json"

if not CONFIG_PATH.exists():
    print(f"Error: {CONFIG_PATH} not found")
    sys.exit(1)

config = json.loads(CONFIG_PATH.read_text(encoding="utf-8"))

REPO_USER      = config["repo_user"]
REPO_NAME      = config["repo_name"]
REPO_BRANCH    = config["repo_branch"]

CONTENT_DIR    = REPO_ROOT / config["content_dir"]
CATEGORIES_DIR = REPO_ROOT / config["categories_dir"]

CATEGORY_MAP   = config["category_map"]

DEFAULTS       = config["defaults"]

RAW_BASE = (
    f"https://raw.githubusercontent.com/{REPO_USER}/{REPO_NAME}/"
    f"{REPO_BRANCH}/{config['content_dir']}"
)

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
                # Refresh the dataurl only — keep user's metadata
                url = f"{RAW_BASE}/{type_name}/{f.name}"
                if existing[section].get("dataurl") != url:
                    existing[section]["dataurl"] = url
                    changed = True
                continue

            # New file — build a fresh entry
            existing[section] = {
                "itemTitle":       section,
                "itemVersion":     DEFAULTS["itemVersion"],
                "itemAuthor":      DEFAULTS["itemAuthor"],
                "itemDescription": DEFAULTS["itemDescription"],
                "dataurl":         f"{RAW_BASE}/{type_name}/{f.name}",
                "path":            f"{base_path}{section}/",
                "reload":          DEFAULTS["reload"],
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
