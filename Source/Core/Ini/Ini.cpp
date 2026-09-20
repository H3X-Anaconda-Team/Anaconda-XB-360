#include "Ini.h"

#include <sstream>

// ------------------------------------------------------------
// IniSection
// ------------------------------------------------------------

std::string IniSection::Get(const std::string& key) const {
    auto it = values.find(key);
    if (it == values.end()) return "";
    return it->second;
}

std::string IniSection::Get(const std::string& key,
                            const std::string& defaultValue) const {
    auto it = values.find(key);
    if (it == values.end()) return defaultValue;
    return it->second;
}

bool IniSection::Has(const std::string& key) const {
    return values.find(key) != values.end();
}

// ------------------------------------------------------------
// IniFile
// ------------------------------------------------------------

const IniSection* IniFile::Find(const std::string& name) const {
    for (const auto& s : sections) {
        if (s.name == name) return &s;
    }
    return nullptr;
}

// ------------------------------------------------------------
// Parser
// ------------------------------------------------------------

static std::string Trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

namespace Ini {

IniFile Parse(const std::string& text) {
    IniFile out;
    std::istringstream ss(text);
    std::string line;
    IniSection* current = nullptr;

    while (std::getline(ss, line)) {
        std::string t = Trim(line);

        // Skip blank lines
        if (t.empty()) continue;

        // Skip comments
        if (t[0] == ';' || t[0] == '#') continue;

        // Section header: [Name]
        if (t.front() == '[' && t.back() == ']') {
            IniSection s;
            s.name = t.substr(1, t.size() - 2);
            out.sections.push_back(s);
            current = &out.sections.back();
            continue;
        }

        // key=value
        if (current) {
            size_t eq = t.find('=');
            if (eq != std::string::npos) {
                std::string key   = Trim(t.substr(0, eq));
                std::string value = Trim(t.substr(eq + 1));
                current->values[key] = value;
            }
        }
    }

    return out;
}

} // namespace Ini
