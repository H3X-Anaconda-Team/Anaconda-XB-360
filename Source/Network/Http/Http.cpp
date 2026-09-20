#include "Http.h"
#include "Platform/Platform.h"
#include "Core/Log/Log.h"

#include <string>

namespace Http {

// ------------------------------------------------------------
// Get
// ------------------------------------------------------------

bool Get(const std::string& url, std::string& out) {
    Log::Info("GET " + url);

    if (!Platform::HttpGet(url, out)) {
        Log::Error("  -> failed");
        return false;
    }

    Log::Info("  -> ok, " + std::to_string(out.size()) + " bytes");
    return true;
}

// ------------------------------------------------------------
// DownloadToFile - no progress
// ------------------------------------------------------------

bool DownloadToFile(const std::string& url,
                    const std::string& dest) {
    return DownloadToFile(url, dest, nullptr);
}

// ------------------------------------------------------------
// DownloadToFile - with progress
// ------------------------------------------------------------

bool DownloadToFile(const std::string& url,
                    const std::string& dest,
                    void (*progress)(size_t, size_t)) {

    Log::Info("DOWNLOAD " + url);
    Log::Info("  -> " + dest);

    if (!Platform::HttpDownloadToFile(url, dest, progress)) {
        Log::Error("  -> failed");
        return false;
    }

    Log::Info("  -> done");
    return true;
}

} // namespace Http
