#include "Log.h"
#include "Anaconda.h"
#include "Platform/Platform.h"

#include <cstdio>
#include <string>

namespace Log {

static bool s_ready = false;

void Init() {
    if (!Platform::DirExists(ANACONDA_BASE_DIR)) {
        Platform::CreateDir(ANACONDA_BASE_DIR);
    }
    s_ready = true;
}

void Write(const std::string& msg) {
    if (!s_ready) Init();

    std::string line = "[" + Platform::Now() + "] " + msg + "\n";

    // Mirror to stdout for Desktop builds
    std::printf("%s", line.c_str());

    // Append to log file
    Platform::AppendFile(ANACONDA_LOG, line);
}

void Info(const std::string& msg) {
    Write("INFO  " + msg);
}

void Warn(const std::string& msg) {
    Write("WARN  " + msg);
}

void Error(const std::string& msg) {
    Write("ERROR " + msg);
}

} // namespace Log
