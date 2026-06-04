#include "buildfiles.h"

namespace fs = std::filesystem;

void makeExecutable(const std::string& path)
{
    fs::permissions(path,
        fs::perms::owner_all |
        fs::perms::group_read | fs::perms::group_exec |
        fs::perms::others_read | fs::perms::others_exec,
        fs::perm_options::replace);
}

void writeScript(const std::string& path, const std::string& content)
{
    std::ofstream out(path);
    if (!out.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + path);
    }
    out << content;
    out.close();
    makeExecutable(path);
}

void buildScripts(const std::string& full_path, const std::string& app_name, const std::vector<std::string> deps)
{
    writeScript(full_path + "/build.sh", createBuild(full_path, app_name));
    writeScript(full_path + "/run.sh",   createRun(app_name));
    writeScript(full_path + "/app.sh",   createApp(app_name));
}


std::string createBuild(const std::string& proj_path, const std::string& app_name, const std::vector<std::string> deps)
{
    if (proj_path.empty() || app_name.empty())
    {
        return "";
    }

    std::string output { R"SH(#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="__PROJECT_DIR__"

detect_os() {
    case "$(uname -s)" in
        Linux*)   echo "linux" ;;
        Darwin*)  echo "macos" ;;
        CYGWIN*|MINGW*|MSYS*) echo "windows" ;;
        *)        echo "unknown" ;;
    esac
}

detect_compiler() {
    if command -v g++ &>/dev/null; then
        echo "g++"
    elif command -v clang++ &>/dev/null; then
        echo "clang++"
    elif command -v c++ &>/dev/null; then
        echo "c++"
    else
        echo ""
    fi
}

OS=$(detect_os)
CXX=${CXX:-$(detect_compiler)}

if [ -z "$CXX" ]; then
    echo "Error: No C++ compiler found. Install g++ or clang++." >&2
    exit 1
fi

echo "OS:      $OS"
echo "Compiler: $CXX"
echo ""

mkdir -p "$PROJECT_DIR/build"
mkdir -p "$PROJECT_DIR/build/linux"
mkdir -p "$PROJECT_DIR/build/windows"

SOURCES=$(find "$PROJECT_DIR/src" -name '*.cpp' | sort)
if [ -z "$SOURCES" ]; then
    echo "Error: no .cpp files found in $PROJECT_DIR/src" >&2
    exit 1
fi

case "$OS" in
    windows)
        if echo "$CXX" | grep -qi "g++\|mingw"; then
            # shellcheck disable=SC2086
            "$CXX" -std=c++17 -Wall -Wextra -pedantic \
                -I"$PROJECT_DIR/src" \
                -o "$PROJECT_DIR/build/windows/__APP_NAME__.exe" \
                $SOURCES
        else
            echo "Error: Windows build currently requires g++ (MinGW)." >&2
            exit 1
        fi
        ;;
    *)
        # shellcheck disable=SC2086
        "$CXX" -std=c++17 -Wall -Wextra -pedantic \
            -I"$PROJECT_DIR/src" \
            -o "$PROJECT_DIR/build/linux/__APP_NAME__" \
            $SOURCES
        ;;
esac

echo "Build complete: $PROJECT_DIR/build/<ENV>/__APP_NAME__"
)SH" };

    size_t pos = 0;
    while ((pos = output.find("__PROJECT_DIR__", pos)) != std::string::npos) {
        output.replace(pos, 15, proj_path);
        pos += proj_path.length();
    }
    pos = 0;
    while ((pos = output.find("__APP_NAME__", pos)) != std::string::npos) {
        output.replace(pos, 12, app_name);
        pos += app_name.length();
    }

    return output;
}

std::string createRun(const std::string& app_name)
{
    if (app_name.empty())
    {
        return "";
    }

    std::string output { R"SH(#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
"$SCRIPT_DIR/build.sh"
exec "$SCRIPT_DIR/build/linux/__APP_NAME__" "$@"
)SH"};

    size_t pos = 0;
    while ((pos = output.find("__APP_NAME__", pos)) != std::string::npos) {
        output.replace(pos, 12, app_name);
        pos += app_name.length();
    }

    return output;
}

std::string createApp(const std::string& app_name)
{
    if (app_name.empty())
    {
        return "";
    }

    std::string output { R"SH(#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec "$SCRIPT_DIR/build/linux/__APP_NAME__" "$@"
)SH"};

    size_t pos = 0;
    while ((pos = output.find("__APP_NAME__", pos)) != std::string::npos) {
        output.replace(pos, 12, app_name);
        pos += app_name.length();
    }

    return output;
}
