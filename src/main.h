#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <stdexcept>
#include <vector>
#include "fileops.h"
#include "buildfiles.h"

struct UserQuit{};
std::string getFullPath();
void getProjectType(std::string& full_path);
bool handleCommands(const std::string& cmd);
void showHelp();

#endif
