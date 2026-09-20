#pragma once

// ============================================================
// Http - thin wrapper around Platform::HttpGet
// ------------------------------------------------------------
// Adds logging and error reporting. Redirects and TLS live in
// the Platform layer — this file just reports what happened.
// ============================================================

#include <string>
#include <cstddef>

namespace Http {

    // Simple blocking GET. Logs the URL and the result.
    bool Get(const std::string& url, std::string& out);

    // Streaming download to disk. Logs start and finish.
    bool DownloadToFile(const std::string& url,
                        const std::string& dest);

    // Same as above but reports progress to a caller-supplied
    // callback. The callback may be null.
    bool DownloadToFile(const std::string& url,
                        const std::string& dest,
                        void (*progress)(size_t, size_t));

} // namespace Http
