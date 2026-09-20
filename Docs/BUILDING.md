# Building Anaconda XB 360

How to turn the source code into a runnable `.xex` for Xbox 360.

---

## Quick decision

| If you want to... | Use |
|---|---|
| Legally distribute the build | **libxenon** |
| Match existing homebrew projects | **XDK + Visual Studio 2010** |
| Test the logic on your laptop first | **Desktop build** (below) |

---

## 1. Desktop build (test on your PC)

Before touching a console, build the Desktop variant. It runs the exact
same app logic — just draws to a terminal instead of the TV.

### Windows

```bash
g++ -std=c++11 -I Include -o AnacondaDesktop.exe ^
    Source/App/main.cpp ^
    Source/Core/Log/Log.cpp ^
    Source/Core/Ini/Ini.cpp ^
    Source/Core/Config/Config.cpp ^
    Source/Network/Http/Http.cpp ^
    Source/Network/Repo/Repo.cpp ^
    Source/Install/Installer.cpp ^
    Source/UI/Ui.cpp ^
    Source/UI/UiDialog.cpp ^
    Source/Platform/Desktop/PlatformDesktop.cpp ^
    -lcurl
```

### Linux / macOS

```bash
g++ -std=c++11 -I Include -o AnacondaDesktop \
    Source/App/main.cpp \
    Source/Core/Log/Log.cpp \
    Source/Core/Ini/Ini.cpp \
    Source/Core/Config/Config.cpp \
    Source/Network/Http/Http.cpp \
    Source/Network/Repo/Repo.cpp \
    Source/Install/Installer.cpp \
    Source/UI/Ui.cpp \
    Source/UI/UiDialog.cpp \
    Source/Platform/Desktop/PlatformDesktop.cpp \
    -lcurl
```

Run it. It reads `repo.ini` from GitHub, lists your categories, and lets
you pick one. Downloads work too — they just land in a local folder.

---

## 2. Xbox 360 build — XDK path

### Requirements

- Windows 7 SP1 (recommended for compatibility) or a VM
- Visual Studio 2010 SP1
- Xbox 360 SDK (XDK)

### Steps

1. Open `AnacondaXB360.sln` in Visual Studio.
2. Set configuration to **Release** and platform to **Xbox 360**.
3. Make sure the XDK include and library paths are set in
   `Tools > Options > Projects and Solutions > VC++ Directories`.
4. Build → Build Solution.

The resulting `AnacondaXB360.xex` appears in `Build/`.

### Using a newer Visual Studio

Newer VS versions don't support the XDK natively. Workaround:

1. Create a batch file that calls the VS2010 compiler environment.
2. Invoke `MSBuild` on your `.vcxproj` from inside that batch.

This is fragile but works if you don't want a Win7 VM.

---

## 3. Xbox 360 build — libxenon path

### Requirements

- libxenon toolchain installed
- `DEVKITXENON` environment variable set

### Setup

```bash
export DEVKITXENON=/usr/local/xenon
export PATH=$DEVKITXENON/bin:$PATH
```

### Build

```bash
make -f build/Makefile
```

This produces `AnacondaXB360.elf`.

### Convert to .xex

```bash
xenon-elf2xex AnacondaXB360.elf AnacondaXB360.xex
```

### Docker alternative

If you don't want to install the toolchain locally:

```bash
docker run -it -v $PWD:/app free60/libxenon:latest
# inside the container:
cd /app && make
```

---

## 4. Deploying to the console

1. Connect to the console via FTP.
2. Copy `AnacondaXB360.xex` to:

   ```text
   Hdd:\Apps\AnacondaXB360\AnacondaXB360.xex
   ```

3. Launch Aurora.
4. Find Anaconda XB 360 in your apps list.

Alternatively, use a USB drive and copy the file manually.

---

## 5. Common build errors

| Error | Cause | Fix |
|---|---|---|
| `cannot open source file "xbox.h"` | XDK not installed or include path wrong | Re-check VC++ Directories |
| `undefined reference to curl_easy_init` | Missing libcurl | Add `-lcurl` on Linux; install libcurl dev |
| `xenon-elf2xex: command not found` | libxenon bin not in PATH | `export PATH=$DEVKITXENON/bin:$PATH` |
| `.xex` builds but won't launch | Missing XexInfo or wrong title ID | Check your `.vcxproj` output settings |

---

## 6. Reproducing a clean build

```bash
# clean everything
rm -rf Build/*
find Source -name "*.o" -delete

# rebuild
make -f build/Makefile
xenon-elf2xex Build/AnacondaXB360.elf Build/AnacondaXB360.xex
```

---

## 7. Version stamping

Before every release, update these in `Source/App/Anaconda.h`:

```c
#define ANACONDA_VERSION "1.0.0"
```

And add an entry to `Docs/CHANGELOG.md`.

---

## 8. Build matrix

| Target | Compiler | Output | Use |
|---|---|---|---|
| Desktop (Windows) | MSVC or MinGW g++ | `AnacondaDesktop.exe` | Local testing |
| Desktop (Linux/macOS) | g++ / clang++ | `AnacondaDesktop` | Local testing |
| Xbox 360 (XDK) | VS2010 + XDK | `AnacondaXB360.xex` | Real console |
| Xbox 360 (libxenon) | xenon-g++ | `AnacondaXB360.xex` | Real console, legal |

---

## 9. CI builds

GitHub Actions cannot legally install the XDK, so the included
`.github/workflows/build.yml` is a **template**. To use it in CI:

- Switch to libxenon and use a self-hosted runner with the toolchain, or
- Build locally and upload the `.xex` to a Release manually

The workflow file shows the structure; it just won't succeed on
GitHub's hosted runners without the XDK.
