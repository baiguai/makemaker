#ifndef BUILDFILES_H
#define BUILDFILES_H

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

void buildScripts(const std::string& p);
std::string createBuild(const std::string& p);
std::string createRun();
std::string createApp();

#endif
