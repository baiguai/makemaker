#if defined(__APPLE__) || defined(__linux__)
#include "fileops.h"
#include <cstdlib>

std::filesystem::path defaultTempFolder()
{
#if defined(__APPLE__)
    return std::filesystem::path("/tmp");
#else
    const char* t = std::getenv("TMPDIR");
    return t ? std::filesystem::path(t) : std::filesystem::path("/tmp");
#endif
}
#endif
