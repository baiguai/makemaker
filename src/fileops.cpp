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
