#ifndef BUILDFILES_H
#define BUILDFILES_H

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <stdexcept>

void makeExecutable(const std::string& path);
void writeScript(const std::string& path, const std::string& content);
void buildCMakeList(const std::string& full_path);
void copyTemplate(const std::string& full_path, const std::string& template_name);

std::string createCMakeConfig();
std::string createCMakeTmplt();
std::string createCMakeBuild();
std::string createCMakeBuildWindows();
std::string createCMakeLeak();
std::string createCMakeRun();
std::string createCMakeApp();

#endif
