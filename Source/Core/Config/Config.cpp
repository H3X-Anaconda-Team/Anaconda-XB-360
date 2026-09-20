#include "Config.h"
#include "Anaconda.h"
#include "Platform/Platform.h"
#include "Core/Log/Log.h"

#include <string>
#include <sstream>

namespace Config {

std::string Path() {
    return ANACONDA_CONFIG;
}

// ------------------------------------------------------------
// Load
// ------------------------------------------------------------

void Load(std::string& repoUrlOut) {
    repoUrlOut = ANACONDA_REPO_URL;

    std::string data;
    if (!Platform::ReadFile(ANACONDA_CONFIG, data)) {
        // File does not exist yet — create it with defaults.
        SaveRepoUrl(repoUrlOut);
        return;
    }

    // Parse "repoUrl=..." from the file.
    std::istringstream ss(data);
    std::string line;

    while (std::getline(ss, line)) {
        // Trim leading whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;

        // Skip comments
        if (line[start] == ';' || line[start] == '#') continue;

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(start, eq - start);
        std::string val = line.substr(eq + 1);

        // Trim key
        size_t keyEnd = key.find_last_not_of(" \t\r\n");
        if (keyEnd != std::string::npos) key = key.substr(0, keyEnd + 1);

        // Trim value
        size_t valStart = val.find_first_not_of(" \t\r\n");
        size_t valEnd   = val.find_last_not_of(" \t\r\n");
        if (valStart != std::string::npos) {
            val = val.substr(valStart, valEnd - valStart + 1);
        } else {
            val = "";
        }

        if (key == "repoUrl" && !val.empty()) {
            repoUrlOut = val;
        }
    }
}

// ------------------------------------------------------------
// Save
// ------------------------------------------------------------

bool SaveRepoUrl(const std::string& repoUrl) {
    if (!Platform::DirExists(ANACONDA_BASE_DIR)) {
        Platform::CreateDir(ANACONDA_BASE_DIR);
    }

    std::string content;
    content += "; Anaconda XB 360 configuration\n";
    content += "; Created automatically on first run.\n";
    content += "\n";
    content += "repoUrl=" + repoUrl + "\n";

    bool ok = Platform::WriteFile(ANACONDA_CONFIG, content);
    if (ok) {
        Log::Info("Config saved: " + repoUrl);
    } else {
        Log::Error("Failed to write config file");
    }
    return ok;
}

} // namespace Config
