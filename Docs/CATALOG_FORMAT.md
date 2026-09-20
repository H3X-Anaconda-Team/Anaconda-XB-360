# Catalog Format

Complete reference for every `.ini` file Anaconda XB 360 reads.

---

## Overview

Anaconda XB 360 reads a hierarchy of `.ini` files:

```text
repo.ini
├── Categories/apps.ini
├── Categories/games.ini
├── Categories/emulators.ini
├── Categories/themes.ini
├── External/free60-apps.ini
├── External/free60-games.ini
└── External/free60-emulators.ini
```

`repo.ini` is the entry point. Every other `.ini` is listed from inside it.
The store loads `repo.ini` first, then loads whichever category the user
selects.

---

## `repo.ini` — the main index

Lists categories. One section per category.

### Format

```ini
[Category Display Name]
iniurl=<URL to that category's .ini file>
```

### Example

```ini
[Apps]
iniurl=https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/Catalog/Categories/apps.ini

[Games]
iniurl=https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/Catalog/Categories/games.ini

[Emulators]
iniurl=https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/Catalog/Categories/emulators.ini

[Themes]
iniurl=https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/Catalog/Categories/themes.ini

[Free60 Apps]
iniurl=https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/Catalog/External/free60-apps.ini

[Free60 Games]
iniurl=https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/Catalog/External/free60-games.ini

[Free60 Emulators]
iniurl=https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/Catalog/External/free60-emulators.ini
```

### Notes

- The section name is what the user sees in the menu.
- If the name contains `Free60`, `X-Store`, or `External`, the store
  groups it under a `[Free60]` prefix in the menu.
- Any number of categories is allowed.
- Order matters — the store shows them top to bottom, locals first,
  then externals.

---

## Category files

Files like `apps.ini`, `games.ini`, `emulators.ini`, `themes.ini`.
Each one lists packages. One section per package.

### Format

```ini
[UniquePackageID]
itemTitle=Display name
itemVersion=1.0
itemAuthor=Author name
itemDescription=Short description
dataurl=<direct link to the .7z file>
path=/Install/Folder/On/Console/
reload=True
```

### Field reference

| Field | Required | Purpose |
|---|---|---|
| `[SectionName]` | ✅ | Unique ID — no spaces, no special characters |
| `itemTitle` | ✅ | What the user sees in the list |
| `itemVersion` | ✅ | Shown next to the title |
| `itemAuthor` | ✅ | Shown next to the title |
| `itemDescription` | ✅ | Shown when the item is highlighted |
| `dataurl` | ✅ | Full URL to the `.7z` file |
| `path` | ✅ | Install location on the console |
| `reload` | ❌ | `True` or `False`. Reload Aurora after install |
| `dataurlpart2` | ❌ | Second part of a split archive |
| `dataurlpart3` | ❌ | Third part of a split archive |

### Example

```ini
[AnacondaFileManager]
itemTitle=Anaconda File Manager
itemVersion=1.0
itemAuthor=H3X Anaconda Team
itemDescription=A simple file manager for Xbox 360.
dataurl=https://github.com/H3X-Anaconda-Team/anaconda-xb-360/releases/download/v1.0/AnacondaFileManager.7z
path=/Apps/AnacondaFileManager/
reload=True
```

---

## Install paths

| Path | Installs to |
|---|---|
| `/Apps/Name/` | `Hdd:\Apps\Name\` |
| `/Games/Name/` | `Hdd:\Games\Name\` |
| `/Emulators/Name/` | `Hdd:\Emulators\Name\` |
| `/Themes/Name/` | `Hdd:\Themes\Name\` |
| `/Content/Name/` | `Hdd:\Content\Name\` |

Any custom path works too — the store does not validate it. The only
requirement is that it starts with `/` and ends with `/`.

---

## Hosting rules

### Files under 100 MiB

Can be committed directly to `Catalog/Content/...` in the repo.
Reference them with a raw GitHub URL:

```ini
dataurl=https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/Catalog/Content/Themes/NeonGreen.7z
```

### Files over 100 MiB

**Must go into GitHub Releases.** GitHub blocks any file over 100 MiB
in a normal repo. Releases have no such limit.

```ini
dataurl=https://github.com/H3X-Anaconda-Team/anaconda-xb-360/releases/download/themes-v1.0/BigTheme.7z
```

### Files over 2 GiB

Split them and use `dataurlpart2`, `dataurlpart3`, etc. The store
concatenates them in order before extracting.

```ini
[BigGame]
itemTitle=Big Game
itemVersion=1.0
itemAuthor=Some Author
itemDescription=A large homebrew game.
dataurl=https://example.com/biggame.part1.7z
dataurlpart2=https://example.com/biggame.part2.7z
dataurlpart3=https://example.com/biggame.part3.7z
path=/Games/BigGame/
reload=True
```

---

## Syntax rules

| Rule | Detail |
|---|---|
| Section names | Case-sensitive |
| Field names | Case-sensitive |
| Values | Everything after the first `=` |
| Comments | Start with `;` or `#` |
| Blank lines | Ignored |
| Whitespace | Trimmed around keys and values |
| Encoding | UTF-8 without BOM |
| Line endings | LF preferred, CRLF tolerated |

---

## Full working example

### `Catalog/repo.ini`

```ini
; ============================================================
; Anaconda XB 360 - Main Repository Index
; ============================================================

[Apps]
iniurl=https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/Catalog/Categories/apps.ini

[Games]
iniurl=https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/Catalog/Categories/games.ini

[Emulators]
iniurl=https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/Catalog/Categories/emulators.ini

[Themes]
iniurl=https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/Catalog/Categories/themes.ini

[Free60 Apps]
iniurl=https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/Catalog/External/free60-apps.ini

[Free60 Games]
iniurl=https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/Catalog/External/free60-games.ini

[Free60 Emulators]
iniurl=https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/Catalog/External/free60-emulators.ini
```

### `Catalog/Categories/apps.ini`

```ini
[AnacondaFileManager]
itemTitle=Anaconda File Manager
itemVersion=1.0
itemAuthor=H3X Anaconda Team
itemDescription=Simple file manager for Xbox 360.
dataurl=https://github.com/H3X-Anaconda-Team/anaconda-xb-360/releases/download/v1.0/AnacondaFileManager.7z
path=/Apps/AnacondaFileManager/
reload=True

[AnacondaFTP]
itemTitle=Anaconda FTP
itemVersion=1.0
itemAuthor=H3X Anaconda Team
itemDescription=Quick FTP server for transferring files.
dataurl=https://github.com/H3X-Anaconda-Team/anaconda-xb-360/releases/download/v1.0/AnacondaFTP.7z
path=/Apps/AnacondaFTP/
reload=True
```

### `Catalog/Categories/themes.ini`

```ini
[AnacondaDark]
itemTitle=Anaconda Dark
itemVersion=1.0
itemAuthor=H3X Anaconda Team
itemDescription=Dark green theme matching Anaconda branding.
dataurl=https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/Catalog/Content/Themes/AnacondaDark.7z
path=/Themes/AnacondaDark/
reload=True

[NeonGreen]
itemTitle=Neon Green
itemVersion=1.0
itemAuthor=H3X Anaconda Team
itemDescription=High-contrast neon green Aurora theme.
dataurl=https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/Catalog/Content/Themes/NeonGreen.7z
path=/Themes/NeonGreen/
reload=True
```

### `Catalog/External/free60-games.ini`

```ini
[Free60ExampleGame]
itemTitle=Example Game (Free60)
itemVersion=1.0
itemAuthor=951261
itemDescription=Placeholder — replace with an actual game from X-Store.
dataurl=https://github.com/951261/X-Store/releases/download/v0.2.12/ExampleGame.7z
path=/Games/ExampleGame/
reload=True
```

---

## Validating a catalog

Use the validator in `Tools/validate-ini/`:

```bash
python Tools/validate-ini/validate.py Catalog/repo.ini
```

It checks:

- Every `iniurl` is reachable
- Every category file parses correctly
- Every package has all required fields
- Every `dataurl` responds with HTTP 200
- No section names are duplicated

---

## Common mistakes

| Mistake | Result | Fix |
|---|---|---|
| Missing `[Section]` header | Values ignored | Add a section name above the fields |
| Spaces in section name | Store can't match it | Use `CamelCase` or `snake_case` |
| `dataurl` points to GitHub HTML page | Download fails | Use `raw.githubusercontent.com` or a Release asset URL |
| `path` doesn't end with `/` | Files scatter | Always end with `/` |
| `.7z` over 100 MiB committed to repo | Push rejected | Move it to Releases |
| Section name reused across files | Only one shows | Keep IDs unique across the whole catalog |
| `reload=Yes` | Ignored (case-sensitive) | Use exactly `True` or `False` |
| Comment starts with `//` | Treated as a value | Use `;` or `#` |

---

## Adding a new package

1. Upload the `.7z` — either to `Catalog/Content/<Type>/` (under 100 MiB)
   or to a GitHub Release (any size).
2. Open the matching category file in `Catalog/Categories/`.
3. Add a new section using the template above.
4. Commit. The store picks it up on next launch.

---

## Adding a new category

1. Create `Catalog/Categories/<newcategory>.ini`.
2. Add package sections to it.
3. Open `Catalog/repo.ini`.
4. Add a section pointing at the new file:

   ```ini
   [New Category]
   iniurl=https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/Catalog/Categories/newcategory.ini
   ```

5. Commit. The category appears in the store's main menu.

---

## Adding an external repo

If another GitHub project publishes a compatible `.ini` catalog, point
at it directly:

```ini
[Some External Store]
iniurl=https://raw.githubusercontent.com/SOMEUSER/SOMEREPO/main/repo.ini
```

If the section name contains `External`, `Free60`, or `X-Store`, the
store groups it under the Free60 heading. Otherwise it shows as its
own top-level category.

---

## File naming convention

| Type | Folder | Example |
|---|---|---|
| Main index | `Catalog/` | `repo.ini` |
| Category list | `Catalog/Categories/` | `apps.ini` |
| External list | `Catalog/External/` | `free60-games.ini` |
| Content under 100 MiB | `Catalog/Content/<Type>/` | `NeonGreen.7z` |
| Content over 100 MiB | GitHub Releases | `BigTheme.7z` |
