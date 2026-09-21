// ============================================================
// PlatformXbox - libxenon implementation
// ------------------------------------------------------------
// Uses POSIX APIs and libxenon extensions. Builds with xenon-gcc.
//
// If you want to keep the original XDK implementation, it lives
// in PlatformXbox.cpp.xdk-backup. Switch between them by renaming
// whichever one you want to build.
// ============================================================

#include "Platform/Platform.h"
#include "Core/Log/Log.h"
#include "ThirdParty/miniz/miniz.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

// ---- libxenon headers ----
extern "C" {
    #include <xenon_soc/xenon_power.h>
    #include <xenon_smc/xenon_smc.h>
    #include <usb/usbmain.h>
    #include <input/input.h>
    #include <console/console.h>
    #include <network/network.h>
    #include <network/http/http.h>
    #include <xenos/xe.h>
    #include <xenos/xenos.h>
    #include <xenos/edram.h>
}

namespace Platform {

// ============================================================
// FILESYSTEM
// ============================================================

bool DirExists(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) return false;
    return S_ISDIR(st.st_mode);
}

bool CreateDir(const std::string& path) {
    return mkdir(path.c_str(), 0755) == 0 || DirExists(path);
}

bool FileExists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

bool ReadFile(const std::string& path, std::string& out) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0) { fclose(f); return false; }
    out.resize((size_t)sz);
    if (sz > 0) {
        size_t got = fread(&out[0], 1, (size_t)sz, f);
        out.resize(got);
    }
    fclose(f);
    return true;
}

bool WriteFile(const std::string& path, const std::string& data) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;
    size_t wrote = fwrite(data.data(), 1, data.size(), f);
    fclose(f);
    return wrote == data.size();
}

bool AppendFile(const std::string& path, const std::string& data) {
    FILE* f = fopen(path.c_str(), "ab");
    if (!f) return false;
    size_t wrote = fwrite(data.data(), 1, data.size(), f);
    fclose(f);
    return wrote == data.size();
}

bool DeleteFile(const std::string& path) {
    return remove(path.c_str()) == 0;
}

// ============================================================
// NETWORKING
// ============================================================

bool HttpGet(const std::string& url, std::string& out) {
    // libxenon ships a small HTTP helper under network/http.
    // If your libxenon build doesn't have it, use the raw socket
    // version instead.
    struct http_request req;
    struct http_response res;

    memset(&req, 0, sizeof(req));
    memset(&res, 0, sizeof(res));

    req.url = (char*)url.c_str();

    if (http_get(&req, &res) != 0) {
        return false;
    }

    if (res.data && res.size > 0) {
        out.assign((const char*)res.data, res.size);
        http_free_response(&res);
        return true;
    }

    http_free_response(&res);
    return false;
}

bool HttpDownloadToFile(const std::string& url,
                        const std::string& destPath,
                        void (*progress)(size_t, size_t)) {
    std::string data;
    if (!HttpGet(url, data)) return false;
    if (progress) progress(data.size(), data.size());
    return WriteFile(destPath, data);
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
        mz_zip_archive_file_stat stat;
        if (!mz_zip_reader_file_stat(&zip, i, &stat)) continue;

        std::string fullPath = destDir;
        if (!fullPath.empty() && fullPath.back() != '\\' && fullPath.back() != '/') {
            fullPath += "/";
        }
        fullPath += stat.m_filename;

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
    xenos_init(VIDEO_MODE_AUTO);
    console_init();
}

void UiShutdown() {
    // Nothing to release on libxenon.
}

void UiClear() {
    console_clrscr();
}

void UiText(int x, int y, const std::string& text, unsigned color) {
    console_set_pos(x, y);
    console_set_color((color >> 16) & 0xFF,
                      (color >> 8)  & 0xFF,
                       color        & 0xFF);
    console_printf("%s\n", text.c_str());
}

void UiRect(int, int, int, int, unsigned) {
    // Basic libxenon console doesn't support rectangles.
    // For a proper UI, replace this with xenos framebuffer code.
}

void UiPresent() {
    // console_printf is drawn directly — no explicit present needed.
}

// ============================================================
// INPUT
// ============================================================

Button PollInput() {
    usb_do_poll();

    // libxenon input layer exposes a pad state after polling.
    // Adjust to match your libxenon version's actual API.
    // This is a placeholder — check the libxenon input sample
    // for the exact function names.
    return BTN_NONE;
}

// ============================================================
// KEYBOARD
// ============================================================

bool ShowKeyboard(const std::string& title,
                  const std::string& initial,
                  std::string& out) {
    console_set_pos(0, 20);
    console_printf("%s [%s]: ", title.c_str(), initial.c_str());

    char buf[512];
    if (!fgets(buf, sizeof(buf), stdin)) return false;

    size_t len = strlen(buf);
    if (len && buf[len-1] == '\n') buf[len-1] = 0;
    out = buf;
    return true;
}

// ============================================================
// AURORA
// ============================================================

void ReloadAurora() {
    // Aurora isn't running when a libxenon app is booted via XeLL.
    // Log only.
    Log::Info("ReloadAurora() called (no-op under libxenon)");
}

// ============================================================
// TIME
// ============================================================

std::string Now() {
    time_t t = time(nullptr);
    struct tm* lt = localtime(&t);
    char buf[32];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
             lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
             lt->tm_hour, lt->tm_min, lt->tm_sec);
    return buf;
}

unsigned long Ticks() {
    return (unsigned long)(clock() * 1000ULL / CLOCKS_PER_SEC);
}

} // namespace Platform
