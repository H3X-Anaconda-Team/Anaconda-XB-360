#include "Installer.h"
#include "Network/Http/Http.h"
#include "Platform/Platform.h"
#include "Core/Log/Log.h"
#include "Ui/Ui.h"

#include <string>
#include <vector>
#include <cstddef>

namespace Installer {

static std::string ToConsolePath(const std::string& p) {
    std::string out = "Hdd:";
    for (char c : p) out += (c == '/') ? '\\' : c;
    if (out.empty() || out.back() != '\\') out += '\\';
    return out;
}

static bool EnsureCacheDir() {
    if (!Platform::DirExists(ANACONDA_BASE_DIR))
        if (!Platform::CreateDir(ANACONDA_BASE_DIR)) return false;

    if (!Platform::DirExists(ANACONDA_CACHE_DIR))
        if (!Platform::CreateDir(ANACONDA_CACHE_DIR)) return false;

    return true;
}

static std::string PartPath(size_t index) {
    return std::string(ANACONDA_CACHE_DIR)
         + "part" + std::to_string(index);
}

static bool DownloadAndMerge(const Package& pkg,
                             void (*progress)(size_t, size_t),
                             std::string& mergedPathOut) {

    const std::vector<std::string>& urls = pkg.dataurls;
    if (urls.empty()) return false;

    std::vector<std::string> parts;

    for (size_t i = 0; i < urls.size(); ++i) {
        std::string dest = PartPath(i + 1);

        if (!Http::DownloadToFile(urls[i], dest, progress)) {
            for (const auto& p : parts) Platform::DeleteFile(p);
            Platform::DeleteFile(dest);
            return false;
        }

        parts.push_back(dest);
    }

    if (parts.size() == 1) {
        mergedPathOut = parts[0];
        return true;
    }

    std::string merged = std::string(ANACONDA_CACHE_DIR) + "package.zip";

    if (!Platform::MergeFiles(parts, merged)) {
        for (const auto& p : parts) Platform::DeleteFile(p);
        Platform::DeleteFile(merged);
        return false;
    }

    for (const auto& p : parts) Platform::DeleteFile(p);

    mergedPathOut = merged;
    return true;
}

bool Install(const Package& pkg) {
    return Install(pkg, nullptr);
}

bool Install(const Package& pkg,
             void (*progress)(size_t, size_t)) {

    Log::Info("Installing: " + pkg.title);

    if (!EnsureCacheDir()) {
        Ui::Message("Install failed",
            "Could not create the cache folder.");
        return false;
    }

    std::string msg = pkg.title + "\n\n";
    msg += (pkg.dataurls.size() > 1)
         ? "Downloading " + std::to_string(pkg.dataurls.size()) + " parts..."
         : "Downloading...";
    Ui::Message("Downloading", msg);

    std::string archivePath;
    if (!DownloadAndMerge(pkg, progress, archivePath)) {
        Ui::Message("Download failed",
            "Could not download:\n" + pkg.title);
        return false;
    }

    std::string dest = ToConsolePath(pkg.path);
    if (!Platform::DirExists(dest) && !Platform::CreateDir(dest)) {
        Platform::DeleteFile(archivePath);
        Ui::Message("Install failed", "Could not create:\n" + dest);
        return false;
    }

    if (!Platform::ExtractZip(archivePath, dest)) {
        Platform::DeleteFile(archivePath);
        Ui::Message("Extract failed",
            "The file could not be unpacked.");
        return false;
    }

    Platform::DeleteFile(archivePath);

    if (pkg.reload) Platform::ReloadAurora();

    Ui::Message("Installed",
        pkg.title + " installed to Hdd:" + pkg.path);
    return true;
}

} // namespace Installer
