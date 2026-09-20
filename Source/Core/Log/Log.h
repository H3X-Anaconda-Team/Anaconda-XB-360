#pragma once

// ============================================================
// Log - writes to Hdd:\Anaconda\anaconda.log
// ------------------------------------------------------------
// Also mirrors every line to stdout so Desktop builds show
// output on the terminal.
// ============================================================

#include <string>

namespace Log {

    // Call once at startup. Creates the log directory if needed.
    void Init();

    // Append a timestamped line to the log file.
    void Write(const std::string& msg);

    // Convenience wrappers with a level prefix.
    void Info(const std::string& msg);
    void Warn(const std::string& msg);
    void Error(const std::string& msg);

} // namespace Log
