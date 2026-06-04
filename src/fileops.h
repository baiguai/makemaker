#ifndef FILEOPS_H
#define FILEOPS_H

#include <filesystem>
#include <string>

bool folderExists(const std::filesystem::path& p);
std::filesystem::path defaultTempFolder();

#endif
