// ============================================================
// PlatformXbox - XDK implementation with XUI drawing
// ============================================================

#include "Platform/Platform.h"
#include "Core/Log/Log.h"
#include "ThirdParty/miniz/miniz.h"

#include <xtl.h>
#include <xhttp.h>
#include <xui.h>
#include <xgraphics.h>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>

namespace Platform {

// ---------- XUI handles ----------
static HXUIDC        g_hDC    = NULL;
static HXUIFONT      g_hFont  = NULL;
static HXUIBRUSH     g_hWhite = NULL;
static HXUIBRUSH     g_hBlack = NULL;
static HXUIBRUSH     g_hGreen = NULL;
static HXUIBRUSH     g_hSel   = NULL;
static HXUIBRUSH     g_hDim   = NULL;

// ------------------------------------------------------------
// FILESYSTEM
// ------------------------------------------------------------

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

bool MergeFiles(const std::vector<std::string>& sources,
                const std::string& dest) {
    HANDLE hOut = CreateFileA(dest.c_str(), GENERIC_WRITE, 0,
                              nullptr, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hOut == INVALID_HANDLE_VALUE) return false;

    char buf[65536];
    bool ok = true;

    for (const auto& src : sources) {
        HANDLE hIn = CreateFileA(src.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                 nullptr, OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hIn == INVALID_HANDLE_VALUE) { ok = false; break; }

        DWORD read = 0;
        while (ReadFile(hIn, buf, sizeof(buf), &read, nullptr) && read > 0) {
            DWORD written = 0;
            if (!WriteFile(hOut, buf, read, &written, nullptr) ||
                written != read) {
                ok = false;
                break;
            }
        }

        CloseHandle(hIn);
        if (!ok) break;
    }

    CloseHandle(hOut);
    return ok;
}

// ------------------------------------------------------------
// NETWORKING
// ------------------------------------------------------------

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

// ------------------------------------------------------------
// ARCHIVE
// ------------------------------------------------------------

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
// UI - Xbox 360 dashboard style
// ============================================================

static void MakeBrush(HXUIBRUSH& brush, float r, float g, float b, float a) {
    XUIBRUSH_DEF def;
    ZeroMemory(&def, sizeof(def));
    def.Type = XUI_BRUSH_TYPE_SOLID;
    def.Color.r = r;
    def.Color.g = g;
    def.Color.b = b;
    def.Color.a = a;
    brush = XuiCreateBrush(&def);
}

void UiInit() {
    XuiRenderInitShared(NULL, 0);

    XUIVideoConfig cfg = {0};
    XuiVideoGetConfig(&cfg);
    cfg.dwDisplayWidth  = 1280;
    cfg.dwDisplayHeight = 720;
    XuiVideoSetConfig(&cfg);

    XuiRenderCreateDC(&g_hDC);

    XUIFontInfo fontInfo = {0};
    XuiFontGetFontInfo(L"SegoeUI", &fontInfo);
    g_hFont = XuiFontCreate(L"SegoeUI", 20.0f, 0);

    MakeBrush(g_hWhite, 1.00f, 1.00f, 1.00f, 1.0f);
    MakeBrush(g_hBlack, 0.00f, 0.00f, 0.00f, 1.0f);
    MakeBrush(g_hGreen, 0.22f, 1.00f, 0.08f, 1.0f);
    MakeBrush(g_hSel,   0.06f, 0.49f, 0.06f, 1.0f);
    MakeBrush(g_hDim,   0.55f, 0.55f, 0.55f, 1.0f);
}

void UiShutdown() {
    if (g_hFont)  XuiFontDestroy(g_hFont);
    if (g_hWhite) XuiBrushDestroy(g_hWhite);
    if (g_hBlack) XuiBrushDestroy(g_hBlack);
    if (g_hGreen) XuiBrushDestroy(g_hGreen);
    if (g_hSel)   XuiBrushDestroy(g_hSel);
    if (g_hDim)   XuiBrushDestroy(g_hDim);
    if (g_hDC)    XuiRenderDestroyDC(g_hDC);
}

void UiClear() {
    // Filled in UiPresent's Begin/End
}

void UiRect(int x, int y, int w, int h, unsigned color) {
    XUIRectangle rect = { (float)x, (float)y, (float)w, (float)h };

    HXUIBRUSH brush = g_hWhite;
    switch (color) {
        case 0x000000: brush = g_hBlack; break;
        case 0x39FF14: brush = g_hGreen; break;
        case 0x107C10: brush = g_hSel;   break;
        case 0x808080: brush = g_hDim;   break;
        default:       brush = g_hWhite; break;
    }

    XuiDrawRect(g_hDC, &rect, brush);
}

void UiText(int x, int y, const std::string& text, unsigned color) {
    WCHAR wbuf[512] = {0};
    MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, wbuf, 512);

    XUIPoint pt = { (float)x, (float)y };

    HXUIBRUSH brush = g_hWhite;
    switch (color) {
        case 0x000000: brush = g_hBlack; break;
        case 0x39FF14: brush = g_hGreen; break;
        case 0x107C10: brush = g_hSel;   break;
        case 0x808080: brush = g_hDim;   break;
        default:       brush = g_hWhite; break;
    }

    XuiDrawText(g_hDC, wbuf, g_hFont, &pt, brush);
}

void UiPresent() {
    // XuiRenderBegin/End is normally wrapped around every frame.
    // Aurora-style apps usually manage this elsewhere; leave empty.
}

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

bool ShowKeyboard(const std::string& title,
                  const std::string& initial,
                  std::string& out) {
    WCHAR wTitle[128]   = {0};
    WCHAR wDefault[512] = {0};
    WCHAR wResult[512]  = {0};

    MultiByteToWideChar(CP_ACP, 0, title.c_str(),   -1, wTitle,   128);
    MultiByteToWideChar(CP_ACP, 0, initial.c_str(), -1, wDefault, 512);

    DWORD result = XShowKeyboardUI(
        0, VK_PAD_A, wDefault, wTitle,
        L"Enter a value", wResult, ARRAYSIZE(wResult));

    if (result != ERROR_SUCCESS) return false;

    char narrow[512] = {0};
    WideCharToMultiByte(CP_ACP, 0, wResult, -1, narrow, 512, nullptr, nullptr);
    out = narrow;
    return true;
}

void ReloadAurora() {
    Log::Info("ReloadAurora() called");
}

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
