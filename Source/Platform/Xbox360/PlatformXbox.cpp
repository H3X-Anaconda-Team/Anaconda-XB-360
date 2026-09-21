// ============================================================
// PlatformXbox - XDK implementation
// ------------------------------------------------------------
// Uses Microsoft Xbox 360 SDK APIs. Builds in Visual Studio
// 2010 SP1 with the XDK installed.
// ============================================================

#include "Platform/Platform.h"
#include "Core/Log/Log.h"
#include "ThirdParty/miniz/miniz.h"

#include <xtl.h>
#include <xhttp.h>
#include <xui.h>
#include <string>
#include <cstring>
#include <cstdio>

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

    char lenBuf[32] = {0};
    DWORD lenSize = sizeof(lenBuf);
    HttpQueryInfoA(hUrl, HTTP_QUERY_CONTENT_LENGTH, lenBuf, &lenSize, nullptr);
    size_t total = (size_t)strtoul(lenBuf, nullptr, 10);

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
// ARCHIVE (ZIP via miniz)
// ============================================================

bool ExtractZip(const std::string& archivePath,
                const std::string& destDir) {
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));

    if (!mz_zip_reader_init_file(&zip, archivePath.c_str(), 0)) {
        Log::Error("miniz: could not open " + archivePath);
        return false;
    }

    int count = (int)mz_zip_reader_get_num_files(&zip);

    for (int i = 0; i < count; ++i) {
        mz_zip_archive_file_stat st;
        if (!mz_zip_reader_file_stat(&zip, i, &st)) continue;

        std::string fullPath = destDir;
        if (!fullPath.empty() && fullPath.back() != '\\' && fullPath.back() != '/') {
            fullPath += "\\";
        }
        fullPath += st.m_filename;

        if (mz_zip_reader_is_file_a_directory(&zip, i)) {
            CreateDir(fullPath);
            continue;
        }

        if (!mz_zip_reader_extract_to_file(&zip, i, fullPath.c_str(), 0)) {
            Log::Error("miniz: extract failed for " + fullPath);
            mz_zip_reader_end(&zip);
            return false;
        }
    }

    mz_zip_reader_end(&zip);
    return true;
}

// ============================================================
// UI
// ============================================================

void UiInit() {
    // XUI is initialised by the XDK runtime.
}

void UiShutdown() {}

void UiClear() {
    // No direct framebuffer control yet.
}

void UiText(int x, int y, const std::string& text, unsigned color) {
    (void)x; (void)y; (void)color;
    OutputDebugStringA(text.c_str());
    OutputDebugStringA("\n");
}

void UiRect(int, int, int, int, unsigned) {}

void UiPresent() {}

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
    WCHAR wTitle[128]   = {0};
    WCHAR wDefault[512] = {0};
    WCHAR wResult[512]  = {0};

    MultiByteToWideChar(CP_ACP, 0, title.c_str(),   -1, wTitle,   128);
    MultiByteToWideChar(CP_ACP, 0, initial.c_str(), -1, wDefault, 512);

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

    char narrow[512] = {0};
    WideCharToMultiByte(CP_ACP, 0, wResult, -1, narrow, 512, nullptr, nullptr);
    out = narrow;
    return true;
}

// ============================================================
// AURORA
// ============================================================

void ReloadAurora() {
    Log::Info("ReloadAurora() called");
}

// ============================================================
// TIME
// ============================================================

std::string Now() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    char buf[64];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
             st.wYear, st.wMonth, st.wDay,
             st.wHour, st.wMinute, st.wSecond);
    return buf;
}

unsigned long Ticks() {
    return GetTickCount();
}

} // namespace Platform
