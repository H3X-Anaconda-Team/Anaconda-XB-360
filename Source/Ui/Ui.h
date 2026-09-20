#pragma once

// ============================================================
// Ui - menus, dialogs, prompts
// ------------------------------------------------------------
// All drawing goes through Platform::Ui* primitives, so this
// code is 100% portable between Desktop and Xbox 360 builds.
//
// External categories (Free60, X-Store, External) are expected
// to already be prefixed by the caller. This module just draws
// whatever strings it is given.
// ============================================================

#include <string>
#include <vector>

namespace Ui {

    // Shows a scrollable list. Returns the selected index,
    // or -1 if the user pressed B / Back.
    int Menu(const std::string& title,
             const std::vector<std::string>& items);

    // Shows a modal message box. Waits for A.
    void Message(const std::string& title,
                 const std::string& body);

    // Yes / No confirm. Returns true if the user picked Yes.
    bool Confirm(const std::string& title,
                 const std::string& body);

    // On-screen keyboard prompt.
    // Returns true if the user confirmed a value.
    bool Prompt(const std::string& title,
                const std::string& initial,
                std::string& out);

} // namespace Ui
