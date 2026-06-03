#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <stdexcept>
#include <vector>
#include "fileops.h"
#include "buildfiles.h"

struct UserQuit{};
std::string getFullPath();
std::string getAppName();
void getDependencies(std::vector<std::string>& deps);
bool handleCommands(const std::string& cmd);

void printPlan(const std::string& full_path,
               const std::string& app_name,
               const std::vector<std::string> deps);
void confirmCreate(const std::string& full_path,
               const std::string& app_name,
               const std::vector<std::string> deps);

#endif
