#include "main.h"

const std::vector<std::string> proj_templates = {
    "CMakeList Project",
    "Dear ImGui Project"};

int main()
{
    std::string full_path { "" };
    int project_type { 1 };

    std::cout << "\n\nWELCOME TO MAKEMAKER.\n";
    std::cout << "? - help   q - exit\n\n";
    std::cout << "Project Templates:\n";

    int idx = 1;
    for (const auto& proj : proj_templates)
    {
        std::cout << "  " << idx << ": " << proj << "\n";
        ++idx;
    }

    std::cout << "\n\n";

    try
    {
        full_path = getFullPath();
        project_type = getProjectType();
    }
    catch (UserQuit&)
    {
        return 0;
    }

    generateTemplate(project_type, full_path);

    std::cout << "\n\nTemplate created at: " << full_path << "\n\n\n";

    return 0;
}

std::string getFullPath()
{
    std::string p { "" };
    std::cout << "Enter the full path to your project:\n";
    std::getline(std::cin, p);

    if (handleCommands(p))
    {
        return getFullPath();
    }

    p = expandTilde(p);

    if (!folderExists(std::filesystem::path(p)))
    {
        std::cout << "The specified directory doesn't exist.\n\n";
        return getFullPath();
    }

    return p;
}

int getProjectType()
{
    std::string tmp { "" };
    int t = 1;
    std::cout << "Enter the template number:\n";
    std::getline(std::cin, tmp);

    if (handleCommands(tmp))
    {
        return getProjectType();
    }

    try
    {
        t = std::stoi(tmp) - 1;
        if (t < 0 || static_cast<std::size_t>(t) >= proj_templates.size()) throw;

        return t;
    }
    catch (...)
    {
        std::cout << "Be sure to enter a valid project template number.\n\n";
        return 1;
    }

    return t;
}

bool handleCommands(const std::string& cmd)
{
    if (cmd == "q")
    {
        throw UserQuit{};
    }

    if (cmd == "?")
    {
        showHelp();
        return true;
    }

    return false;
}

void showHelp()
{
    std::cout << "\n\n";
    std::cout << "Available Project Templates:\n\n";
}

void generateTemplate(const int& project_type, const std::string& full_path)
{
    switch (project_type)
    {
        case 1: // Dear ImGui
            break;

        default: // CMakeList
            buildCMakeList(full_path);
            break;
    }
}
