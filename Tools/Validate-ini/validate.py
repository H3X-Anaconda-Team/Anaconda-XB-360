#!/usr/bin/env python3
"""
Anaconda XB 360 - Catalog Validator
====================================

Checks every .ini file in Catalog/ for syntax and content problems.

Usage:
    python Tools/validate-ini/validate.py
    python Tools/validate-ini/validate.py Catalog/repo.ini

Exits with code 0 if everything is fine, 1 if any errors were found.
"""

import re
import sys
import urllib.request
import urllib.error
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
CATALOG   = REPO_ROOT / "Catalog"

REQUIRED_PACKAGE_FIELDS = [
    "itemTitle", "itemVersion", "itemAuthor",
    "itemDescription", "dataurl", "path", "reload",
]

VALID_RELOAD = ("True", "False")

# ------------------------------------------------------------
# Parsing
# ------------------------------------------------------------

def parse_ini(path: Path):
    """Returns (sections, errors). Sections is {name: {key: value}}."""
    sections = {}
    errors   = []
    current  = None

    if not path.exists():
        return {}, [f"File does not exist: {path}"]

    text = path.read_text(encoding="utf-8", errors="replace")

    for lineno, raw in enumerate(text.splitlines(), start=1):
        line = raw.strip()
        if not line or line.startswith(";") or line.startswith("#"):
            continue

        m = re.match(r"^\[(.+)\]$", line)
        if m:
            name = m.group(1)
            if name in sections:
                errors.append(f"{path.name}:{lineno} duplicate section [{name}]")
            sections[name] = {}
            current = name
            continue

        if "=" not in line:
            errors.append(f"{path.name}:{lineno} not a key=value: {line!r}")
            continue

        if current is None:
            errors.append(f"{path.name}:{lineno} key/value before any [Section]")
            continue

        k, v = line.split("=", 1)
        k = k.strip()
        v = v.strip()

        if not k:
            errors.append(f"{path.name}:{lineno} empty key")
            continue

        sections[current][k] = v

    return sections, errors

# ------------------------------------------------------------
# Validation
# ------------------------------------------------------------

def check_repo_ini(path: Path):
    sections, errors = parse_ini(path)

    if not sections:
        errors.append(f"{path.name}: no sections found")

    for name, fields in sections.items():
        if "iniurl" not in fields:
            errors.append(f"{path.name} [{name}]: missing iniurl")
        elif not fields["iniurl"].startswith(("http://", "https://")):
            errors.append(f"{path.name} [{name}]: iniurl is not a URL")

    return sections, errors


def check_category_ini(path: Path):
    sections, errors = parse_ini(path)

    for name, fields in sections.items():
        for req in REQUIRED_PACKAGE_FIELDS:
            if req not in fields:
                errors.append(f"{path.name} [{name}]: missing {req}")

        if "reload" in fields and fields["reload"] not in VALID_RELOAD:
            errors.append(
                f"{path.name} [{name}]: reload={fields['reload']!r} "
                f"(must be True or False)"
            )

        if "dataurl" in fields:
            url = fields["dataurl"]
            if not url.startswith(("http://", "https://")):
                errors.append(f"{path.name} [{name}]: dataurl is not a URL")

        if "path" in fields:
            p = fields["path"]
            if not p.startswith("/") or not p.endswith("/"):
                errors.append(
                    f"{path.name} [{name}]: path must start and end with '/'"
                )

    return sections, errors


def check_url(url: str) -> bool:
    """HEAD request. Returns True for 2xx."""
    try:
        req = urllib.request.Request(url, method="HEAD")
        with urllib.request.urlopen(req, timeout=10) as resp:
            return 200 <= resp.status < 300
    except Exception:
        return False

# ------------------------------------------------------------
# Main
# ------------------------------------------------------------

def main(argv):
    if len(argv) > 1:
        target = Path(argv[1])
        if not target.is_absolute():
            target = REPO_ROOT / target
        files = [target]
    else:
        files = sorted(CATALOG.rglob("*.ini"))

    if not files:
        print("No .ini files found.")
        return 1

    total_errors = 0
    online = True

    for path in files:
        name = path.name.lower()
        print(f"\nChecking {path.relative_to(REPO_ROOT)}")

        if name == "repo.ini":
            sections, errors = check_repo_ini(path)
        else:
            sections, errors = check_category_ini(path)

        for e in errors:
            print(f"  ERROR  {e}")
        total_errors += len(errors)

        if not errors:
            print(f"  OK     {len(sections)} section(s)")

        # Optional URL reachability check
        if online:
            for sname, fields in sections.items():
                url = fields.get("iniurl") or fields.get("dataurl")
                if not url:
                    continue
                if not check_url(url):
                    print(f"  WARN   [{sname}] URL not reachable: {url}")

    print()
    if total_errors:
        print(f"Finished with {total_errors} error(s).")
        return 1

    print("All checks passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
