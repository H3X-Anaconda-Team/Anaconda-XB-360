// ============================================================
// Installer - downloads packages and extracts them
// ------------------------------------------------------------
// Supports:
//   - Single-file packages  (dataurl)
//   - Multi-part packages   (dataurl, dataurlpart2, dataurlpart3, ...)
//
// Multi-part flow:
//   1. Download every part to Hdd:\Anaconda\cache\partN
//   2. Merge them into Hdd:\Anaconda\cache\package.zip
//   3. Extract package.zip to the install path
//   4. Delete all temp files
// ============================================================

#include "Installer.h"
#include "Network/Http/Http.h"
#include "Platform/Platform.h"
#include "Core/Log/Log.h"
#include "Ui/Ui.h"

#include <string>
#include <vector>
#include <cstddef>

namespace Installer {

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

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

// Returns the temp path for part N (1-based).
static std::string PartPath(size_t index) {
    return std::string(ANACONDA_CACHE_DIR)
         + "part"
         + std::to_string(index);
}

// ------------------------------------------------------------
// Download all parts, merge, return merged path
// ------------------------------------------------------------

static bool DownloadAndMerge(const Package& pkg,
                             void (*progress)(size_t, size_t),
                             std::string& mergedPathOut) {

    const std::vector<std::string>& urls = pkg.dataurls;
    if (urls.empty()) {
        Log::Error("No download URLs for " + pkg.title);
        return false;
    }

    std::vector<std::string> parts;

    // --- 1. Download each part one at a time ---
    for (size_t i = 0; i < urls.size(); ++i) {
        std::string dest = PartPath(i + 1);

        Log::Info("Downloading part " + std::to_string(i + 1)
                  + "/" + std::to_string(urls.size())
                  + ": " + urls[i]);

        if (!Http::DownloadToFile(urls[i], dest, progress)) {
            Log::Error("Part " + std::to_string(i + 1) + " download failed");

            // Clean up whatever we already downloaded
            for (const auto& p : parts) Platform::DeleteFile(p);
            Platform::DeleteFile(dest);

            return false;
        }

        parts.push_back(dest);
    }

    // --- 2. If there's only one part, that IS the final archive ---
    if (parts.size() == 1) {
        mergedPathOut = parts[0];
        Log::Info("Single-part download: " + mergedPathOut);
        return true;
    }

    // --- 3. Merge all parts into one file ---
    std::string merged = std::string(ANACONDA_CACHE_DIR) + "package.zip";

    Log::Info("Merging " + std::to_string(parts.size())
              + " parts into " + merged);

    if (!Platform::MergeFiles(parts, merged)) {
        Log::Error("Merge failed");

        for (const auto& p : parts) Platform::DeleteFile(p);
        Platform::DeleteFile(merged);

        return false;
    }

    // --- 4. Delete the individual parts ---
    for (const auto& p : parts) Platform::DeleteFile(p);

    mergedPathOut = merged;
    Log::Info("Merged archive ready: " + mergedPathOut);
    return true;
}

// ------------------------------------------------------------
// Install
// ------------------------------------------------------------

bool Install(const Package& pkg) {
    return Install(pkg, nullptr);
}

bool Install(const Package& pkg,
             void (*progress)(size_t, size_t)) {

    Log::Info("Installing: " + pkg.title + " v" + pkg.version);

    if (!EnsureCacheDir()) {
        Ui::Message("Install failed",
            "Could not create the cache folder.\n"
            "Check Hdd:\\Anaconda\\ permissions.");
        return false;
    }

    // --- 1. Show download message ---
    std::string msg = pkg.title + "\n\n";
    if (pkg.dataurls.size() > 1) {
        msg += "Downloading " + std::to_string(pkg.dataurls.size())
             + " parts. Please wait...";
    } else {
        msg += "Please wait...";
    }
    Ui::Message("Downloading", msg);

    // --- 2. Download + merge ---
    std::string archivePath;
    if (!DownloadAndMerge(pkg, progress, archivePath)) {
        Ui::Message("Download failed",
            "Could not download all parts of:\n" + pkg.title +
            "\n\nCheck your internet connection and try again.");
        return false;
    }

    // --- 3. Prepare destination ---
    std::string dest = ToConsolePath(pkg.path);
    Log::Info("Install path: " + dest);

    if (!Platform::DirExists(dest)) {
        if (!Platform::CreateDir(dest)) {
            Log::Error("Could not create " + dest);
            Platform::DeleteFile(archivePath);
            Ui::Message("Install failed", "Could not create:\n" + dest);
            return false;
        }
    }

    // --- 4. Extract ---
    if (!Platform::ExtractZip(archivePath, dest)) {
        Log::Error("Extract failed: " + archivePath);
        Platform::DeleteFile(archivePath);
        Ui::Message("Extract failed",
            "The downloaded file could not be unpacked.\n\n"
            "The file may be corrupt. Try again, or delete\n"
            "Hdd:\\Anaconda\\cache\\ and retry.");
        return false;
    }

    Log::Info("Extracted to " + dest);

    // --- 5. Clean up ---
    Platform::DeleteFile(archivePath);

    // --- 6. Reload Aurora if requested ---
    if (pkg.reload) {
        Log::Info("Reloading Aurora");
        Platform::ReloadAurora();
    }

    Log::Info("Install complete: " + pkg.title);
    return true;
}

} // namespace Installer
