// ============================================================
// PlatformXbox - real Xbox 360 build
// ------------------------------------------------------------
// This is the file you port to the console. It uses XDK-style
// APIs. If you're using libxenon instead, swap each function's
// body for the libxenon equivalent (mostly posix).
//
// Everything else in the project stays the same.
// ============================================================

#include "Platform/Platform.h"
#include "Core/Log/Log.h"

#include <string>
#include <cstring>
#include <cstdio>

// ---- XDK headers ----
#include <xtl.h>
#include <xboxmath.h>
#include <xhttp.h>
#include <xui.h>

// If you're using libxenon, replace the block above with:
//   #include <xenon_soc/xenon_power.h>
//   #include <xenon_smc/xenon_smc.h>
//   #include <network/network.h>
//   #include <console/console.h>

namespace Platform {

// ============================================================
// FILESYSTEM
// ============================================================

bool DirExists(const std::string& path) {
    DWORD attr = GetFileAttributesA(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY);
}

bool CreateDir(const std::string& path) {
    return CreateDirectoryA(path.c_str(), nullptr) != 0 || DirExists(path);
}

bool FileExists(const std::string& path) {
    DWORD attr = GetFileAttributesA(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES) && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

bool ReadFile(const std::string& path, std::string& out) {
    HANDLE h = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                           nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;

    DWORD size = GetFileSize(h, nullptr);
    out.resize(size);

    DWORD read = 0;
    ReadFile(h, &out[0], size, &read, nullptr);
    CloseHandle(h);
    out.resize(read);
    return true;
}

bool WriteFile(const std::string& path, const std::string& data) {
    HANDLE h = CreateFileA(path.c_str(), GENERIC_WRITE, 0,
                           nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;

    DWORD written = 0;
    WriteFile(h, data.data(), (DWORD)data.size(), &written, nullptr);
    CloseHandle(h);
    return written == data.size();
}

bool AppendFile(const std::string& path, const std::string& data) {
    HANDLE h = CreateFileA(path.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ,
                           nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;

    DWORD written = 0;
    WriteFile(h, data.data(), (DWORD)data.size(), &written, nullptr);
    CloseHandle(h);
    return written == data.size();
}

bool DeleteFile(const std::string& path) {
    return DeleteFileA(path.c_str()) != 0;
}

// ============================================================
// NETWORKING
// ============================================================

// Simple blocking HTTP GET. Requires XNetStartup() to have been
// called once at program start.
bool HttpGet(const std::string& url, std::string& out) {
    HINTERNET hSession = InternetOpenA("AnacondaXB360",
                                       INTERNET_OPEN_TYPE_DIRECT,
                                       nullptr, nullptr, 0);
    if (!hSession) return false;

    HINTERNET hUrl = InternetOpenUrlA(hSession, url.c_str(),
                                      nullptr, 0,
                                      INTERNET_FLAG_RELOAD |
                                      INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hUrl) {
        InternetCloseHandle(hSession);
        return false;
    }

    out.clear();
    char buf[8192];
    DWORD read = 0;
    while (InternetReadFile(hUrl, buf, sizeof(buf), &read) && read > 0) {
        out.append(buf, read);
    }

    InternetCloseHandle(hUrl);
    InternetCloseHandle(hSession);
    return !out.empty();
}

bool HttpDownloadToFile(const std::string& url,
                        const std::string& destPath,
                        void (*progress)(size_t, size_t)) {
    HINTERNET hSession = InternetOpenA("AnacondaXB360",
                                       INTERNET_OPEN_TYPE_DIRECT,
                                       nullptr, nullptr, 0);
    if (!hSession) return false;

    HINTERNET hUrl = InternetOpenUrlA(hSession, url.c_str(),
                                      nullptr, 0,
                                      INTERNET_FLAG_RELOAD |
                                      INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hUrl) {
        InternetCloseHandle(hSession);
        return false;
    }

    // Query total size (may be 0 if unknown)
    char lenBuf[32] = {0};
    DWORD lenSize = sizeof(lenBuf);
    HttpQueryInfoA(hUrl, HTTP_QUERY_CONTENT_LENGTH,
                   lenBuf, &lenSize, nullptr);
    size_t total = (size_t)std::strtoul(lenBuf, nullptr, 10);

    HANDLE hFile = CreateFileA(destPath.c_str(), GENERIC_WRITE, 0,
                               nullptr, CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hSession);
        return false;
    }

    char buf[8192];
    DWORD read = 0;
    size_t done = 0;
    bool ok = true;

    while (InternetReadFile(hUrl, buf, sizeof(buf), &read) && read > 0) {
        DWORD written = 0;
        if (!WriteFile(hFile, buf, read, &written, nullptr) ||
            written != read) {
            ok = false;
            break;
        }
        done += read;
        if (progress) progress(done, total);
    }

    CloseHandle(hFile);
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hSession);
    return ok;
}

// ============================================================
// ARCHIVE
// ============================================================

// Requires the 7-Zip SDK to be linked in. The skeleton below
// shows the shape; fill in the real extraction loop.
bool Extract7z(const std::string& archivePath,
               const std::string& destDir) {
    Log::Info("Extract7z: " + archivePath + " -> " + destDir);

    // --- XDK 7-Zip SDK entry point (pseudo) ---
    // extern int SevenZipExtract(const wchar_t* archive,
    //                            const wchar_t* dest);
    //
    // Convert std::string paths to wide strings here, then call
    // the SDK. Return true on success.
    //
    // The 7-Zip SDK ships with the XDK under:
    //   Xbox 360 SDK\Source\Samples\XboxLive\...
    // Reference an existing homebrew that uses it (e.g. Aurora's
    // installer) for the exact function names.

    // Placeholder — replace before shipping.
    return false;
}

// ============================================================
// UI
// ============================================================

void UiInit() {
    // XUI is the standard drawing API on the XDK.
    // XuiRenderBegin / XuiRenderEnd wrap your draw calls.
    // For 720p output, set up a XUIVideoConfig first.
    XuiRenderInit();
}

void UiShutdown() {
    // Nothing to release in the basic case.
}

void UiClear() {
    // Fill the backbuffer with black.
    // XuiRenderBegin(d3dDevice, 0xFF000000);
}

void UiText(int x, int y, const std::string& text, unsigned color) {
    // Convert to wide string and draw with XuiDrawText.
    // The exact call depends on your font handle.
    (void)x; (void)y; (void)text; (void)color;
}

void UiRect(int x, int y, int w, int h, unsigned color) {
    // Draw a filled rectangle with XuiDrawRect.
    (void)x; (void)y; (void)w; (void)h; (void)color;
}

void UiPresent() {
    // XuiRenderEnd + Present.
}

// ============================================================
// INPUT
// ============================================================

Button PollInput() {
    XINPUT_STATE state;
    if (XInputGetState(0, &state) != ERROR_SUCCESS) return BTN_NONE;

    WORD b = state.Gamepad.wButtons;
    if (b & XINPUT_GAMEPAD_DPAD_UP)     return BTN_UP;
    if (b & XINPUT_GAMEPAD_DPAD_DOWN)   return BTN_DOWN;
    if (b & XINPUT_GAMEPAD_DPAD_LEFT)   return BTN_LEFT;
    if (b & XINPUT_GAMEPAD_DPAD_RIGHT)  return BTN_RIGHT;
    if (b & XINPUT_GAMEPAD_A)           return BTN_A;
    if (b & XINPUT_GAMEPAD_B)           return BTN_B;
    if (b & XINPUT_GAMEPAD_X)           return BTN_X;
    if (b & XINPUT_GAMEPAD_Y)           return BTN_Y;
    if (b & XINPUT_GAMEPAD_START)       return BTN_START;
    if (b & XINPUT_GAMEPAD_BACK)        return BTN_BACK;
    return BTN_NONE;
}

// ============================================================
// KEYBOARD
// ============================================================

bool ShowKeyboard(const std::string& title,
                  const std::string& initial,
                  std::string& out) {
    // XShowKeyboardUI is the standard on-screen keyboard.
    WCHAR wTitle[128] = {0};
    WCHAR wDefault[512] = {0};
    WCHAR wResult[512] = {0};

    // Convert `title` and `initial` to wide strings here.

    DWORD result = XShowKeyboardUI(
        0,
        VK_PAD_A,
        wDefault,
        wTitle,
        L"Enter a value",
        wResult,
        ARRAYSIZE(wResult)
    );

    if (result != ERROR_SUCCESS) return false;

    // Convert wResult back to std::string into `out`.
    return true;
}

// ============================================================
// AURORA
// ============================================================

void ReloadAurora() {
    // Aurora watches for content changes via a kernel notification.
    // The simplest way to trigger a rescan is to write to the
    // folder Aurora is watching, or call into its plugin API.
    //
    // If you don't have access to Aurora's internals, delete and
    // recreate a marker file inside Hdd:\Apps\ and Aurora will
    // rescan on next boot.
    Log::Info("ReloadAurora() called");
}

// ============================================================
// TIME
// ============================================================

std::string Now() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                  st.wYear, st.wMonth, st.wDay,
                  st.wHour, st.wMinute, st.wSecond);
    return buf;
}

unsigned long Ticks() {
    return GetTickCount();
}

} // namespace Platform
