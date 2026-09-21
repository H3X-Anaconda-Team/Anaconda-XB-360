#include "Repo.h"
#include "Network/Http/Http.h"
#include "Core/Ini/Ini.h"
#include "Core/Log/Log.h"

#include <string>
#include <vector>

namespace Repo {

static bool LooksExternal(const std::string& name) {
    return  name.find("Free60")   != std::string::npos
         || name.find("X-Store")  != std::string::npos
         || name.find("External") != std::string::npos;
}

bool LoadCategories(const std::string& repoUrl,
                    std::vector<Category>& out) {
    out.clear();

    std::string text;
    if (!Http::Get(repoUrl, text)) {
        Log::Error("Could not fetch repo.ini: " + repoUrl);
        return false;
    }

    IniFile ini = Ini::Parse(text);

    std::vector<Category> locals;
    std::vector<Category> externals;

    for (const auto& s : ini.sections) {
        if (!s.Has("iniurl")) continue;

        Category c;
        c.name       = s.name;
        c.iniurl     = s.Get("iniurl");
        c.isExternal = LooksExternal(s.name);

        if (c.isExternal) externals.push_back(c);
        else              locals.push_back(c);
    }

    if (locals.empty() && externals.empty()) {
        Log::Warn("repo.ini contained no categories");
        return false;
    }

    out.reserve(locals.size() + externals.size());
    out.insert(out.end(), locals.begin(), locals.end());
    out.insert(out.end(), externals.begin(), externals.end());

    Log::Info("Loaded " + std::to_string(out.size()) + " categories");
    return true;
}

bool LoadPackages(const std::string& categoryUrl,
                  std::vector<Package>& out) {
    out.clear();

    std::string text;
    if (!Http::Get(categoryUrl, text)) {
        Log::Error("Could not fetch category: " + categoryUrl);
        return false;
    }

    IniFile ini = Ini::Parse(text);

    for (const auto& s : ini.sections) {
        if (!s.Has("dataurl")) continue;

        Package p;
        p.id          = s.name;
        p.title       = s.Get("itemTitle",       s.name);
        p.version     = s.Get("itemVersion",     "?");
        p.author      = s.Get("itemAuthor",      "Unknown");
        p.description = s.Get("itemDescription", "");
        p.path        = s.Get("path", "/Apps/" + s.name + "/");
        p.reload      = (s.Get("reload", "False") == "True");

        // Collect every part in order: dataurl, dataurlpart2, dataurlpart3, ...
        p.dataurls.push_back(s.Get("dataurl"));

        int partN = 2;
        while (true) {
            std::string key = "dataurlpart" + std::to_string(partN);
            if (!s.Has(key)) break;

            std::string url = s.Get(key);
            if (!url.empty()) p.dataurls.push_back(url);
            ++partN;
        }

        out.push_back(p);
    }

    Log::Info("Loaded " + std::to_string(out.size()) + " packages from "
              + categoryUrl);

    return true;
}

} // namespace Repo
