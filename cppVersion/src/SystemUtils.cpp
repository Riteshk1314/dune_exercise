#include "SystemUtils.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

void SystemUtils::clearCaches() {
#ifdef _WIN32
    // needs admin privileges on Windows
    std::cout << "    [Note: cache clearing on Windows needs admin tools like RAMMap]" << std::endl;
#else
    // needs sudo on Linux
    int result = system("sync; echo 3 > /proc/sys/vm/drop_caches 2>/dev/null");
    if (result != 0) {
        std::cout << "    [Note: cache clearing needs sudo - run manually if needed]" << std::endl;
    }
#endif
}

std::string SystemUtils::getSystemInfo() {
    std::string info = "System: ";
#ifdef _WIN32
    info += "Windows";
#elif __linux__
    info += "Linux";
#elif __APPLE__
    info += "macOS";
#else
    info += "Unknown";
#endif
    return info;
}
