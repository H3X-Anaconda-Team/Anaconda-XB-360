#pragma once

// ============================================================
// Config - reads and writes Hdd:\Anaconda\anaconda.cfg
// ------------------------------------------------------------
// Format:
//   repoUrl=https://...
//
// On first launch the file is created with the default URL.
// ============================================================

#include <string>

namespace Config {

    // Loads the config from disk. If the file does not exist,
    // creates it with the default repo URL from Anaconda.h.
    void Load(std::string& repoUrlOut);

    // Saves just the repo URL. Everything else is preserved.
    bool SaveRepoUrl(const std::string& repoUrl);

    // Returns the config file path (useful for logging).
    std::string Path();

} // namespace Config
