#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <stdexcept>
#include <vector>
#include "fileops.h"
#include "buildfiles.h"

struct UserQuit{};
std::string getFullPath();
int getProjectType();
bool handleCommands(const std::string& cmd);
void showHelp();
void generateTemplate(const int& project_type, const std::string& full_path);

#endif
