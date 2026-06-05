#ifndef FILEOPS_H
#define FILEOPS_H

#include <filesystem>
#include <string>
#include <cstdlib>
#include <iostream>

bool folderExists(const std::filesystem::path& p);
std::filesystem::path defaultTempFolder();
std::string expandTilde(const std::string& path);

#endif
