#include "Installer.h"
#include "Network/Http/Http.h"
#include "Platform/Platform.h"
#include "Core/Log/Log.h"
#include "UI/Ui.h"

#include <string>
#include <cstddef>

namespace Installer {

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

// Converts "/Apps/Name/" to "Hdd:\Apps\Name\" for the console.
static std::string ToConsolePath(const std::string& p) {
    std::string out = "Hdd:";
    for (char c : p) {
        out += (c == '/') ? '\\' : c;
    }
    if (out.empty() || out.back() != '\\') {
        out += '\\';
    }
    return out;
}

// Ensures Hdd:\Anaconda\cache\ exists.
static bool EnsureCacheDir() {
    if (!Platform::DirExists(ANACONDA_BASE_DIR)) {
        if (!Platform::CreateDir(ANACONDA_BASE_DIR)) {
            Log::Error("Could not create " + std::string(ANACONDA_BASE_DIR));
            return false;
        }
    }
    if (!Platform::DirExists(ANACONDA_CACHE_DIR)) {
        if (!Platform::CreateDir(ANACONDA_CACHE_DIR)) {
            Log::Error("Could not create " + std::string(ANACONDA_CACHE_DIR));
            return false;
        }
    }
    return true;
}

// ------------------------------------------------------------
// Install - no progress callback
// ------------------------------------------------------------

bool Install(const Package& pkg) {
    return Install(pkg, nullptr);
}

// ------------------------------------------------------------
// Install - full pipeline
// ------------------------------------------------------------

bool Install(const Package& pkg,
             void (*progress)(size_t, size_t)) {

    Log::Info("Installing: " + pkg.title + " v" + pkg.version);

    // --- 1. Make sure the cache folder exists ---
    if (!EnsureCacheDir()) {
        Ui::Message("Install failed",
            "Could not create the cache folder.\n"
            "Check Hdd:\\Anaconda\\ permissions.");
        return false;
    }

    // --- 2. Download ---
    Ui::Message("Downloading", pkg.title + "\n\nPlease wait...");

    std::string tempArchive = ANACONDA_TEMP_7Z;

    if (!Http::DownloadToFile(pkg.dataurl, tempArchive, progress)) {
        Log::Error("Download failed: " + pkg.dataurl);
        Ui::Message("Download failed",
            "Could not download:\n" + pkg.dataurl +
            "\n\nCheck your internet connection and try again.");
        return false;
    }

    Log::Info("Downloaded to " + tempArchive);

    // --- 3. Prepare the destination folder ---
    std::string dest = ToConsolePath(pkg.path);
    Log::Info("Install path: " + dest);

    if (!Platform::DirExists(dest)) {
        if (!Platform::CreateDir(dest)) {
            Log::Error("Could not create " + dest);
            Platform::DeleteFile(tempArchive);
            Ui::Message("Install failed",
                "Could not create:\n" + dest);
            return false;
        }
    }

    // --- 4. Extract ---
    if (!Platform::Extract7z(tempArchive, dest)) {
        Log::Error("Extract failed: " + tempArchive);
        Platform::DeleteFile(tempArchive);
        Ui::Message("Extract failed",
            "The downloaded file could not be unpacked.\n\n"
            "The file may be corrupt. Try again, or delete\n"
            "Hdd:\\Anaconda\\cache\\ and retry.");
        return false;
    }

    Log::Info("Extracted to " + dest);

    // --- 5. Clean up the archive ---
    Platform::DeleteFile(tempArchive);

    // --- 6. Reload Aurora if the package asked for it ---
    if (pkg.reload) {
        Log::Info("Reloading Aurora");
        Platform::ReloadAurora();
    }

    Log::Info("Install complete: " + pkg.title);
    return true;
}

} // namespace Installer
