# Architecture

How Anaconda XB 360 is put together.

---

## Overview

Anaconda XB 360 is a native Xbox 360 application (`.xex`) written in C++.
It reads `.ini` catalogs over HTTP, displays a menu, downloads `.7z`
packages, and extracts them to the console's HDD.

It has **no dependency on Aurora's Lua sandbox** — it is a standalone
program that Aurora can launch like any other homebrew.

---

## Layers

```text
┌──────────────────────────────────────┐
│  App/          main.cpp              │  entry point, main loop
├──────────────────────────────────────┤
│  UI/           menus, dialogs        │  draws the interface
├──────────────────────────────────────┤
│  Install/      Installer             │  download + extract
├──────────────────────────────────────┤
│  Network/      Http, Repo            │  fetch .ini, fetch .7z
├──────────────────────────────────────┤
│  Core/         Log, Ini, Config      │  parsing, settings
├──────────────────────────────────────┤
│  Platform/     Xbox360 | Desktop     │  the ONLY console-touching code
└──────────────────────────────────────┘
```

Each layer only talks to the layer directly below it.

---

## Platform layer

This is the most important design decision in the project.

**`Source/Platform/Platform.h`** declares a fixed interface:
filesystem, networking, archive extraction, UI drawing, input.

**Two implementations exist:**

| Folder | Purpose |
|---|---|
| `Platform/Desktop/` | PC stub using standard C++ — lets you run the whole app on your laptop |
| `Platform/Xbox360/` | Real console code using XDK or libxenon |

When you build for PC, the Desktop file is compiled. When you build for
Xbox 360, the Xbox360 file is compiled. **No other file in the project
changes between the two builds.**

This means you can write, test, and debug 95% of Anaconda on your laptop
before ever touching a console.

---

## Data flow

```text
1.  User launches AnacondaXB360.xex
2.  main.cpp -> Repo::LoadCategories(repoUrl)
3.  Repo fetches repo.ini over HTTP
4.  Ini::Parse turns it into structs
5.  UI::Menu shows the category list
6.  User picks a category
7.  Repo::LoadPackages(category.ini) fetches that file
8.  UI::Menu shows the package list
9.  User picks a package
10. Installer::Install(pkg)
      - Http::DownloadToFile  -> Hdd:\Anaconda\cache\package.7z
      - Platform::Extract7z   -> Hdd:\<pkg.path>\
      - Platform::ReloadAurora if pkg.reload
11. Loop back to step 5
```

---

## Folder structure

```text
Source/
├── App/
│   ├── Anaconda.h          constants: name, version, paths, URLs
│   └── main.cpp            entry point, top-level menu loop
│
├── Core/
│   ├── Log/                writes to Hdd:\Anaconda\anaconda.log
│   ├── Ini/                parses .ini text into sections
│   └── Config/             reads/writes anaconda.cfg
│
├── Network/
│   ├── Http/               thin wrapper around Platform::HttpGet
│   └── Repo/               reads catalogs, returns Category / Package
│
├── Install/
│   └── Installer.cpp       downloads, extracts, optionally reloads
│
├── UI/
│   ├── Ui.cpp              menu drawing, message boxes
│   └── UiDialog.cpp        keyboard prompt, confirm dialogs
│
└── Platform/
    ├── Platform.h          the interface — every console call is here
    ├── Platform.cpp        dispatcher, picks Desktop or Xbox360
    ├── Desktop/
    │   └── PlatformDesktop.cpp   PC test build
    └── Xbox360/
        └── PlatformXbox.cpp      real console build
```

---

## File-by-file

| File | Job |
|---|---|
| `Source/App/main.cpp` | Entry point, top-level menu loop |
| `Source/App/Anaconda.h` | Constants: name, version, paths, URLs |
| `Source/Core/Log/Log.cpp` | Writes to `Hdd:\Anaconda\anaconda.log` |
| `Source/Core/Ini/Ini.cpp` | Parses `.ini` text into sections |
| `Source/Core/Config/Config.cpp` | Reads/writes `anaconda.cfg` |
| `Source/Network/Http/Http.cpp` | Thin wrapper around `Platform::HttpGet` |
| `Source/Network/Repo/Repo.cpp` | Reads catalogs, returns `Category` / `Package` |
| `Source/Install/Installer.cpp` | Downloads, extracts, optionally reloads |
| `Source/UI/Ui.cpp` | Menu drawing, message boxes, keyboard prompt |
| `Source/Platform/Platform.h` | Declares the interface |
| `Source/Platform/Desktop/PlatformDesktop.cpp` | PC test build |
| `Source/Platform/Xbox360/PlatformXbox.cpp` | Real console build |

---

## Key data structures

Defined in `Source/App/Anaconda.h`:

```cpp
struct Package {
    std::string id;
    std::string title;
    std::string version;
    std::string author;
    std::string description;
    std::string dataurl;
    std::string path;      // e.g. "/Apps/Name/"
    bool        reload;
};

struct Category {
    std::string name;
    std::string iniurl;
    bool        isExternal;   // true if name contains "Free60" etc.
};
```

These are the only structs passed between layers.

---

## Where data lives on the console

| Data | Location |
|---|---|
| Config | `Hdd:\Anaconda\anaconda.cfg` |
| Log | `Hdd:\Anaconda\anaconda.log` |
| Download cache | `Hdd:\Anaconda\cache\` |
| Installed apps | `Hdd:\Apps\<Name>\` |
| Installed games | `Hdd:\Games\<Name>\` |
| Installed emulators | `Hdd:\Emulators\<Name>\` |
| Installed themes | `Hdd:\Themes\<Name>\` |

The catalog decides the install path — Anaconda just follows the
`path=` value from the `.ini`.

---

## The catalog hierarchy

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
See [`CATALOG_FORMAT.md`](CATALOG_FORMAT.md) for the full format reference.

---

## Error handling

| Layer | On failure |
|---|---|
| `Http` | Returns `false`, logs the HTTP status |
| `Repo` | Returns `false`, logs which URL failed |
| `Installer` | Returns `false`, shows a message box, keeps the log |
| `Platform` | Returns `false`, never throws |
| `UI` | Never fails — draws an error state instead |

No exceptions are used anywhere in the project. All errors are returned
as `bool` and logged.

---

## Threading

**None.** The app is single-threaded by design.

Downloads block the UI while they run. A progress bar is planned for
v1.1.0, and it will run on the same thread, updating between HTTP
read chunks.

This keeps the code simple and avoids the race conditions that plague
many homebrew projects.

---

## Not included

- No auto-updater (planned — see `ROADMAP.md`)
- No save-game management
- No title update handling
- No Aurora plugin hooks
- No multi-threading

---

## Why this design

| Choice | Reason |
|---|---|
| Native C++ over Lua | Full HTTP, real file I/O, no sandbox limits |
| Platform layer split | Write 95% of the app on a laptop |
| No exceptions | Smaller binary, works with XDK's limited C++ runtime |
| Single-threaded | Simpler, no race conditions |
| Config in a file | Users can edit without a PC |
| `.ini` catalog | Human-readable, editable with a text editor |
| External repo support | Pull from X-Store without forking it |
