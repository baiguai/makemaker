#include "main.h"

int main()
{
    std::string full_path { "" };
    std::string app_name { "" };

    std::cout << "\n\nWELCOME TO MAKEMAKER.\n\n\n";

    try
    {
        full_path = getFullPath();
        app_name = getAppName();
        printPlan(full_path, app_name);
        confirmCreate(full_path, app_name);
    }
    catch (UserQuit&)
    {
        return 0;
    }

    

    return 0;
}

std::string getFullPath()
{
    std::string p { "" };
    std::cout << "Enter the full path to your application:\n";
    std::getline(std::cin, p);

    if (handleCommands(p))
    {
        return getFullPath();
    }

    if (!folderExists(std::filesystem::path(p)))
    {
        std::cout << "The specified directory doesn't exist.\n\n";
        return getFullPath();
    }

    return p;
}

std::string getAppName()
{
    std::string p { "" };
    std::cout << "Enter the name of your application:\n";
    std::getline(std::cin, p);

    if (p.empty())
    {
        p = "q"; // if blank, assume the user wants to exit
    }

    if (handleCommands(p))
    {
        return getAppName();
    }

    return p;
}

bool handleCommands(const std::string& cmd)
{
    if (cmd == "q")
    {
        throw UserQuit{};
    }

    return false;
}


void printPlan(const std::string& full_path,
               const std::string& app_name)
{
    std::cout << "\n\nApplication Path:\n   " << full_path;
    std::cout << "\nApplication Name:\n   " << app_name;

    std::cout << "\n\n";
}

void confirmCreate(const std::string& full_path,
               const std::string& app_name)
{
    std::string p { "" };
    std::cout << "Create makefile and build/run scripts (y/n)?\n";
    std::getline(std::cin, p);

    if (p.empty() || p == "n")
    {
        p = "q";
    }

    if (handleCommands(p))
    {
        return;
    }

    if (p == "y")
    {
        buildScripts(full_path, app_name);
        std::cout << "\nScripts built.\n\n";
    }
}
