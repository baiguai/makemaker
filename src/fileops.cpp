#include "fileops.h"

bool folderExists(const std::filesystem::path& p)
{
    try
    {
        return std::filesystem::exists(p) && std::filesystem::is_directory(p);
    }
    catch (...)
    {
        return false;
    }
}

std::string expandTilde(const std::string& path)
{
    if (path.empty() || path[0] != '~') return path;

#if defined(_WIN32)
    const char* home = std::getenv("USERPROFILE");
    if (!home)
    {
        const char* drive = std::getenv("HOMEDRIVE");
        const char* pathenv = std::getenv("HOMEPATH");
        if (drive && pathenv)
        {
            std::string hp = std::string(drive) + std::string(pathenv);
            return hp + path.substr(1);
        }
    }
#else
    const char* home = std::getenv("HOME");
#endif

    if (!home) return path; // fallback: return unchanged if home not found
    return std::string(home) + path.substr(1);
}
