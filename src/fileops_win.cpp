#if defined(_WIN32) || defined(_WIN64)
#include "fileops.h"
#include <windows.h>

std::filesystem::path defaultTempFolder() {
    wchar_t buf[MAX_PATH];
    if (GetTempPathW(MAX_PATH, buf)) return std::filesystem::path(buf);
    return std::filesystem::path("C:\\Temp");
}

#endif
