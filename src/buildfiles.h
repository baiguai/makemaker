#ifndef BUILDFILES_H
#define BUILDFILES_H

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <stdexcept>

void buildScripts(const std::string& full_path, const std::string& app_name);
std::string createBuild(const std::string& proj_path, const std::string& app_name);
std::string createRun(const std::string& app_name);
std::string createApp(const std::string& app_name);

#endif
