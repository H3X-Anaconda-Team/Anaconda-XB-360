#pragma once

// ============================================================
// Platform - the ONLY interface that touches the console
// ------------------------------------------------------------
// Two implementations exist:
//   Source/Platform/Desktop/PlatformDesktop.cpp   (PC test build)
//   Source/Platform/Xbox360/PlatformXbox.cpp      (real console)
//
// Your build system compiles ONE of them. Everything else in
// the project calls the functions below and never touches the
// OS directly.
// ============================================================

#include <string>
#include <cstddef>

namespace Platform {

    // ---------------- Filesystem ----------------
    bool DirExists(const std::string& path);
    bool CreateDir(const std::string& path);
    bool FileExists(const std::string& path);
    bool ReadFile(const std::string& path, std::string& out);
    bool WriteFile(const std::string& path, const std::string& data);
    bool AppendFile(const std::string& path, const std::string& data);
    bool DeleteFile(const std::string& path);

    // ---------------- Networking ----------------
    bool HttpGet(const std::string& url, std::string& out);

    bool HttpDownloadToFile(const std::string& url,
                            const std::string& destPath,
                            void (*progress)(size_t, size_t) = nullptr);

    // ---------------- Archive ----------------
    // Extracts a .zip archive into destDir.
    // Creates destDir if it does not exist.
    bool ExtractZip(const std::string& archivePath,
                    const std::string& destDir);

    // ---------------- UI primitives ----------------
    void UiInit();
    void UiShutdown();
    void UiClear();
    void UiText(int x, int y, const std::string& text, unsigned color);
    void UiRect(int x, int y, int w, int h, unsigned color);
    void UiPresent();

    // ---------------- Input ----------------
    enum Button {
        BTN_NONE,
        BTN_UP,
        BTN_DOWN,
        BTN_LEFT,
        BTN_RIGHT,
        BTN_A,
        BTN_B,
        BTN_X,
        BTN_Y,
        BTN_START,
        BTN_BACK
    };

    Button PollInput();

    // ---------------- Keyboard prompt ----------------
    bool ShowKeyboard(const std::string& title,
                      const std::string& initial,
                      std::string& out);

    // ---------------- Aurora integration ----------------
    void ReloadAurora();

    // ---------------- Time ----------------
    std::string Now();
    unsigned long Ticks();

} // namespace Platform
