#pragma once

// ============================================================
// Repo - reads the .ini catalog hierarchy
// ------------------------------------------------------------
// LoadCategories() reads repo.ini and returns a list of
// categories.
//
// LoadPackages() reads a single category file and returns the
// packages listed inside it.
//
// Local categories come first. Categories whose name contains
// "Free60", "X-Store", or "External" are moved to the end and
// flagged with isExternal = true, so the UI can prefix them
// with "[Free60]".
// ============================================================

#include "Anaconda.h"
#include <string>
#include <vector>

namespace Repo {

    // Fetches and parses repo.ini.
    bool LoadCategories(const std::string& repoUrl,
                        std::vector<Category>& out);

    // Fetches and parses a single category .ini.
    bool LoadPackages(const std::string& categoryUrl,
                      std::vector<Package>& out);

} // namespace Repo
