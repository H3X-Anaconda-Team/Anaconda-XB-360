#pragma once

// ============================================================
// Anaconda XB 360 - Global Definitions
// ============================================================

#define ANACONDA_NAME     "Anaconda XB 360"
#define ANACONDA_VERSION  "1.0.0"
#define ANACONDA_AUTHOR   "H3X Anaconda Team"

#define ANACONDA_REPO_URL \
    "https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/Catalog/repo.ini"

// ---- Console paths ----
#define ANACONDA_BASE_DIR    "Hdd:\\Anaconda\\"
#define ANACONDA_CACHE_DIR   "Hdd:\\Anaconda\\cache\\"
#define ANACONDA_CONFIG      "Hdd:\\Anaconda\\anaconda.cfg"
#define ANACONDA_LOG         "Hdd:\\Anaconda\\anaconda.log"
#define ANACONDA_TEMP_ZIP    "Hdd:\\Anaconda\\cache\\package.zip"

#include <string>
#include <vector>

struct Package {
    std::string id;
    std::string title;
    std::string version;
    std::string author;
    std::string description;
    std::vector<std::string> dataurls;
    std::string path;
    bool        reload;
};

struct Category {
    std::string name;
    std::string iniurl;
    bool        isExternal;
};
