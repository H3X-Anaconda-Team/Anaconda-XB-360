#pragma once

// ============================================================
// Ini - minimal .ini parser
// ------------------------------------------------------------
// Handles:
//   - [Section] headers
//   - key=value pairs
//   - ; and # comments
//   - whitespace trimming around keys and values
//
// Does NOT handle:
//   - nested sections
//   - multi-line values
//   - quoted strings with escapes
// ============================================================

#include <string>
#include <vector>
#include <map>

struct IniSection {
    std::string name;
    std::map<std::string, std::string> values;

    // Returns "" if the key is missing.
    std::string Get(const std::string& key) const;

    // Returns defaultValue if the key is missing.
    std::string Get(const std::string& key,
                    const std::string& defaultValue) const;

    // Returns true if the key exists (even with an empty value).
    bool Has(const std::string& key) const;
};

struct IniFile {
    std::vector<IniSection> sections;

    // Returns nullptr if not found.
    const IniSection* Find(const std::string& name) const;
};

namespace Ini {

    // Parse .ini text into an IniFile.
    IniFile Parse(const std::string& text);

} // namespace Ini
