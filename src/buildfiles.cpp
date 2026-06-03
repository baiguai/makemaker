#include "buildfiles.h"

void buildScripts(const std::string& p)
{
    std::string build_script;
    std::string build_path = p + "/build.sh";
    std::string run_script;
    std::string run_path = p + "/run.sh";
    std::string app_script;
    std::string app_path = p + "/app.sh";


    build_script = createBuild(p);
    run_script = createRun();
    app_script = createApp();


    std::ofstream bld(build_path);
    if (!bld.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + build_path);
    }

    bld << build_script;
    bld.close();

    std::ofstream run(run_path);
    if (!run.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + run_path);
    }

    run << run_script;
    run.close();

    std::ofstream app(app_path);
    if (!app.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + app_path);
    }

    app << app_script;
    app.close();
}


std::string createBuild(const std::string& p)
{
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
                -o "$PROJECT_DIR/build/makemaker.exe" \
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
            -o "$PROJECT_DIR/build/makemaker" \
            $SOURCES
        ;;
esac

echo "Build complete: $PROJECT_DIR/build/makemaker"
)SH" };

    size_t pos = 0;
    while ((pos = output.find("__PROJECT_DIR__", pos)) != std::string::npos) {
        output.replace(pos, 15, p);
        pos += p.length();
    }

    return output;
}

std::string createRun()
{
    return R"SH(#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
"$SCRIPT_DIR/build.sh"
exec "$SCRIPT_DIR/build/makemaker" "$@"
)SH";
}

std::string createApp()
{
    return R"SH(#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec "$SCRIPT_DIR/build/makemaker" "$@"
)SH";
}
