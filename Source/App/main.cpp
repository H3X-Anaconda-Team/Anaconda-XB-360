#include "Anaconda.h"
#include "Core/Log/Log.h"
#include "Core/Config/Config.h"
#include "Network/Repo/Repo.h"
#include "Install/Installer.h"
#include "Ui/Ui.h"
#include "Platform/Platform.h"

#include <string>
#include <vector>

static std::string BuildCategoryLabel(const Category& c) {
    if (c.isExternal) {
        return "[Free60] " + c.name;
    }
    return c.name;
}

static std::string BuildPackageLabel(const Package& p) {
    return p.title + "  v" + p.version + "  by " + p.author;
}

static int ShowCategoryMenu(const std::vector<Category>& cats) {
    std::vector<std::string> labels;
    labels.reserve(cats.size() + 1);

    labels.push_back("Settings");
    for (const auto& c : cats) {
        labels.push_back(BuildCategoryLabel(c));
    }

    std::string title = std::string(ANACONDA_NAME) + " v" + ANACONDA_VERSION;
    return Ui::Menu(title, labels);
}

static void OpenCategory(const Category& cat) {
    std::vector<Package> pkgs;

    if (!Repo::LoadPackages(cat.iniurl, pkgs)) {
        Ui::Message(cat.name, "Failed to load this category.");
        return;
    }

    if (pkgs.empty()) {
        Ui::Message(cat.name, "No items in this category yet.");
        return;
    }

    std::vector<std::string> labels;
    labels.reserve(pkgs.size());
    for (const auto& p : pkgs) {
        labels.push_back(BuildPackageLabel(p));
    }

    int pick = Ui::Menu(cat.name, labels);
    if (pick < 0) return;

    const Package& pkg = pkgs[pick];

    std::string body = pkg.description;
    if (!body.empty()) body += "\n\n";
    body += "Version: " + pkg.version + "\n";
    body += "Author:  " + pkg.author  + "\n";
    body += "Install: Hdd:" + pkg.path;

    if (!Ui::Confirm("Install " + pkg.title + "?", body)) {
        return;
    }

    if (Installer::Install(pkg)) {
        Ui::Message("Installed",
            pkg.title + " was installed to Hdd:" + pkg.path);
    } else {
        Ui::Message("Failed",
            "Could not install " + pkg.title + ".\nCheck the log file.");
    }
}

static void OpenSettings(std::string& repoUrl,
                         std::vector<Category>& cats) {
    std::string newUrl;
    if (!Ui::Prompt("Repository URL", repoUrl, newUrl)) return;
    if (newUrl.empty() || newUrl == repoUrl) return;

    repoUrl = newUrl;
    Config::SaveRepoUrl(repoUrl);

    if (!Repo::LoadCategories(repoUrl, cats)) {
        Ui::Message("Settings", "Saved, but could not load the new repository.");
    }
}

int main() {
    Log::Init();
    Log::Info(std::string(ANACONDA_NAME) + " v" + ANACONDA_VERSION + " starting");

    Platform::UiInit();

    std::string repoUrl;
    Config::Load(repoUrl);
    Log::Info("Repo URL: " + repoUrl);

    std::vector<Category> cats;
    if (!Repo::LoadCategories(repoUrl, cats)) {
        Ui::Message(ANACONDA_NAME,
            "Could not reach the repository:\n" + repoUrl +
            "\n\nCheck your internet connection or change the URL in Settings.");
        Platform::UiShutdown();
        return 1;
    }

    Log::Info("Loaded " + std::to_string(cats.size()) + " categories");

    while (true) {
        int pick = ShowCategoryMenu(cats);

        if (pick < 0) break;

        if (pick == 0) {
            OpenSettings(repoUrl, cats);
            continue;
        }

        const Category& cat = cats[pick - 1];
        OpenCategory(cat);
    }

    Platform::UiShutdown();
    Log::Info("Anaconda XB 360 exiting");
    return 0;
}
