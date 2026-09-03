#include "buildfiles.h"
#include <filesystem>

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




void buildCMakeList(const std::string& full_path)
{
    fs::create_directories(full_path + "/src");
    fs::create_directories(full_path + "/patches");
    writeScript(full_path + "/patches/ftxui-c0-delivery.patch", createPatch());
    writeScript(full_path + "/config.sh", createCMakeConfig());
    writeScript(full_path + "/CMakeLists.txt.in", createCMakeTmplt());
    writeScript(full_path + "/build.sh", createCMakeBuild());
    writeScript(full_path + "/build-windows.sh", createCMakeBuildWindows());
    writeScript(full_path + "/leakcheck.sh", createCMakeLeak());
    writeScript(full_path + "/run.sh", createCMakeRun());
    writeScript(full_path + "/app.sh", createCMakeApp());
    writeScript(full_path + "/setup.sh", createSetup());
    writeScript(full_path + "/upgrade.sh", createUpgrade());
}

void copyTemplate(const std::string& full_path, const std::string& template_name)
{
    namespace fs = std::filesystem;

    fs::path src = template_name;
    fs::path dst = full_path;

    for (const auto& entry : fs::directory_iterator(src)) {
        fs::copy(entry.path(), dst / entry.path().filename(),
                 fs::copy_options::recursive |
                 fs::copy_options::overwrite_existing);
    }
}




















// Ftxui Patch
////////////////////////////////////////////////////////////////////////////////
std::string createPatch()
{
    std::string output { R"SH(
diff --git a/src/ftxui/component/terminal_input_parser.cpp b/src/ftxui/component/terminal_input_parser.cpp
index 3ba0e69d..01b13c69 100644
--- a/src/ftxui/component/terminal_input_parser.cpp
+++ b/src/ftxui/component/terminal_input_parser.cpp
@@ -166,10 +166,6 @@ TerminalInputParser::Output TerminalInputParser::Parse() {
   }
 
   switch (Current()) {
-    case 24:  // CAN NOLINT
-    case 26:  // SUB NOLINT
-      return DROP;
-
     case '\x1B':
       return ParseESC();
     default:
    )SH"};

    return output;
}




// CMake Config
////////////////////////////////////////////////////////////////////////////////
std::string createCMakeConfig()
{
    std::string output { R"SH(#!/bin/bash

# Central configuration - edit values here, all scripts pick them up

APP_NAME="App"

SOURCES=(
    "src/main.cpp"
    "src/utils.cpp"
)

LIBS=(
    "ftxui::screen"
    "ftxui::dom"
    "ftxui::component"
)

HEADERS=(
    "src/main.hpp"
    "src/utils.hpp"
)
)SH"};

    return output;
}




// CMake Lists Template
////////////////////////////////////////////////////////////////////////////////
std::string createCMakeTmplt()
{
    std::string output { R"SH(cmake_minimum_required(VERSION 3.16)
project(<<TARGET_NAME>> VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create executable
add_executable(<<TARGET_NAME>>
<<SOURCES>>
)

include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
  GIT_TAG v5.0.0
  PATCH_COMMAND git checkout -- . && git apply ${CMAKE_CURRENT_SOURCE_DIR}/patches/ftxui-c0-delivery.patch
)
FetchContent_MakeAvailable(ftxui)

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    # Linux/X11
    find_package(X11 REQUIRED)
    target_link_libraries(<<TARGET_NAME>> PRIVATE X11::X11 X11::Xfixes)
elseif(APPLE)
    # macOS
    find_library(COCOA Cocoa)
    find_library(COREFOUNDATION CoreFoundation)
    target_link_libraries(<<TARGET_NAME>> PRIVATE ${COCOA} ${COREFOUNDATION})
elseif(WIN32)
    # Windows
    target_link_libraries(<<TARGET_NAME>> PRIVATE user32 kernel32 gdi32)
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(<<TARGET_NAME>> PRIVATE -Wall -Wextra -O2)
endif()

# Set output directory
set_target_properties(<<TARGET_NAME>> PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
)SH"};

    return output;
}





// CMake Build
////////////////////////////////////////////////////////////////////////////////
std::string createCMakeBuild()
{
    std::string output { R"SH(#!/bin/bash

# Build Script

set -e  # Exit on any error
source ./config.sh

# Determine build type
BUILD_TYPE="Debug"
if [ "$1" == "r" ]; then
    BUILD_TYPE="Release"
    echo "Performing RELEASE build."
else
    echo "Performing DEBUG build (default)."
fi

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

echo "Building Windows EXE..."
./build-windows.sh || echo "Warning: Windows build failed, continuing with Linux build..."

# Navigate to build directory
cd build

# Generate CMakeLists.txt from template using config.sh values
echo "Generating CMakeLists.txt..."
cp ../CMakeLists.txt.in ../CMakeLists.txt

# Inject app name
sed -i "s/<<TARGET_NAME>>/$APP_NAME/g" ../CMakeLists.txt

# Inject source files
SOURCES_TMP=$(mktemp)
for s in "${SOURCES[@]}"; do
    echo "    $s" >> "$SOURCES_TMP"
done
sed -i "/^<<SOURCES>>$/{
    r $SOURCES_TMP
    d
}" ../CMakeLists.txt
rm -f "$SOURCES_TMP"

# Inject library files
for lib in "${LIBS[@]}"; do
    if [[ "$lib" == *::* ]]; then
        echo "target_link_libraries($APP_NAME PRIVATE $lib)" >> ../CMakeLists.txt
    else
        echo "target_link_libraries($APP_NAME PRIVATE \${CMAKE_SOURCE_DIR}/$lib)" >> ../CMakeLists.txt
    fi
done

# Configure with CMake
echo "Configuring with CMake..."
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..

# Build the project
echo "Compiling..."
make

# Check if build was successful
if [ -f "bin/$APP_NAME" ]; then




    # Add any custom cp's or other actions here
    # mkdir -p "./bin/data/themes"
    # cp -r ../themes/* "./bin/data/themes/" 2>/dev/null || true




    echo "-- Build successful --"
    echo "Executable: $(pwd)/bin/$APP_NAME"
    echo ""
    echo "To run $APP_NAME:"
    echo "  ./bin/$APP_NAME"
    echo ""
    echo "Or from the parent directory:"
    echo "  ./build/bin/$APP_NAME"
else
    echo "! failed !"
    exit 1
fi
)SH"};

    return output;
}





// CMake Build Windows
////////////////////////////////////////////////////////////////////////////////
std::string createCMakeBuildWindows()
{
    std::string output { R"SH(#!/bin/bash

# Requirements:
# sudo apt install -y mingw-w64

source ./config.sh

set -e

echo "Building $APP_NAME for Windows..."

BUILD_TYPE="Debug"
if [ "$1" == "r" ]; then
    BUILD_TYPE="Release"
fi
echo "Performing $BUILD_TYPE build."

TOOLCHAIN="$(dirname "$0")/cmake/mingw-x86_64.cmake"

mkdir -p build-windows

# Generate CMakeLists.txt from template
cp CMakeLists.txt.in CMakeLists.txt
sed -i "s/<<TARGET_NAME>>/$APP_NAME/g" CMakeLists.txt
SOURCES_TMP=$(mktemp)
for s in "${SOURCES[@]}"; do
    echo "    $s" >> "$SOURCES_TMP"
done
sed -i "/^<<SOURCES>>$/{
    r $SOURCES_TMP
    d
}" CMakeLists.txt

rm -f "$SOURCES_TMP"

for lib in "${LIBS[@]}"; do
    if [[ "$lib" == *::* ]]; then
        echo "target_link_libraries($APP_NAME PRIVATE $lib)" >> CMakeLists.txt
    else
        echo "target_link_libraries($APP_NAME PRIVATE \${CMAKE_SOURCE_DIR}/$lib)" >> CMakeLists.txt
    fi
done

cmake -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -B build-windows \
      -S .

cmake --build build-windows

echo "Build complete: build-windows/bin/$APP_NAME.exe"

# The cross-compiled exe depends on the MinGW-w64 runtime DLLs, which are not
# present on a stock Windows machine. Resolve each DLL through the compiler
# driver and copy it next to the exe.
MINGW_CC="${MINGW_CC:-x86_64-w64-mingw32-gcc}"
DEST="build-windows/bin"
for dll in libstdc++-6.dll libgcc_s_seh-1.dll libwinpthread-1.dll; do
    src="$("$MINGW_CC" -print-file-name="$dll")"
    if [ -f "$src" ] && [ "$src" != "$dll" ]; then
        cp -f "$src" "$DEST/"
        echo "Copied $dll next to $APP_NAME.exe"
    else
        echo "Warning: could not locate $dll (got: $src) - $APP_NAME.exe may not run on a clean Windows machine" >&2
    fi
done
)SH"};

    return output;
}




// CMake Leak Check
////////////////////////////////////////////////////////////////////////////////
std::string createCMakeLeak()
{
    std::string output { R"SH(#!/bin/bash

source ./config.sh

valgrind --leak-check=full "./build/bin/$APP_NAME"
)SH"};

    return output;
}




// CMake Run
////////////////////////////////////////////////////////////////////////////////
std::string createCMakeRun()
{
    std::string output { R"SH(#!/bin/bash

# Run Script
# This script builds and runs your application

set -e  # Exit on any error
source ./config.sh

echo "Building $APP_NAME..."
./build.sh

# Check if build was successful
if [ ! -f "build/bin/$APP_NAME" ]; then
    echo "! Build failed ! -- Cannot start $APP_NAME."
    exit 1
fi

echo "-- Build successful --"
echo ""
echo ""
echo "Starting $APP_NAME..."
echo ""
echo "----------------------------------------"
echo ""
echo ""

# Run the application
"./build/bin/$APP_NAME"
)SH"};

    return output;
}




// CMake App
////////////////////////////////////////////////////////////////////////////////
std::string createCMakeApp()
{
    std::string output { R"SH(#!/bin/bash

# Run Script
# This script JUST runs your application

set -e  # Exit on any error
source ./config.sh

echo "...Starting $APP_NAME..."

# Check if build was successful
if [ ! -f "build/bin/$APP_NAME" ]; then
    echo "! Run failed ! -- Cannot start $APP_NAME."
    exit 1
fi

# Run the application
"./build/bin/$APP_NAME"
)SH"};

    return output;
}




// Setup Script
////////////////////////////////////////////////////////////////////////////////
std::string createSetup()
{
    std::string output { R"SH(#!/bin/bash
#
# app setup - checks prerequisites and prepares the FTXUI dependency.
#
# Safe to run at any time; it does NOT assume a blank/fresh project and never
# creates scaffolding. Optional flag:
#   ./setup.sh --prefetch   download FTXUI now so the first ./build.sh works
#                           even without network access.
#
# FTXUI requirements covered here:
#   - cmake 3.16+            (FetchContent needs it; build.sh uses cmake+make)
#   - a C++17 compiler        (g++ or clang++)
#   - git                     (FetchContent clones FTXUI from GitHub; the
#                              PATCH_COMMAND also runs 'git apply')
#   - make                    (build.sh compiles with 'make')
#   - network to github.com   (to download FTXUI on first build)
#   - mingw-w64               (optional, only for ./build-windows.sh)

set -e

PREFETCH=0
case "${1:-}" in
    "" ) ;;
    --prefetch|-p) PREFETCH=1 ;;
    * ) echo "usage: ./setup.sh [--prefetch]"; exit 2 ;;
esac

echo "=== app setup ==="

# --- cmake -----------------------------------------------------------------
command -v cmake >/dev/null 2>&1 || { echo "ERROR: cmake is required (3.16+). Install it and try again."; exit 1; }
CMAKE_VER=$(cmake --version | head -1 | grep -oP '\d+\.\d+')
if [ "$(printf '%s\n3.16\n' "$CMAKE_VER" | sort -V | head -1)" != "3.16" ]; then
    echo "ERROR: cmake 3.16+ required (found $CMAKE_VER)"
    exit 1
fi
echo "[OK] cmake $CMAKE_VER"

# --- compiler --------------------------------------------------------------
if command -v g++ >/dev/null 2>&1; then
    echo "[OK] g++ $(g++ -dumpversion)"
elif command -v clang++ >/dev/null 2>&1; then
    echo "[OK] clang++ ($(clang++ --version | head -1))"
else
    echo "ERROR: no C++17 compiler found (g++ or clang++)"
    exit 1
fi

# --- git (FTXUI is fetched from GitHub and patched with 'git apply') -------
command -v git >/dev/null 2>&1 || { echo "ERROR: git is required (FTXUI is fetched from GitHub and patched)."; exit 1; }
echo "[OK] git $(git --version | cut -d' ' -f3)"

# --- make (build.sh compiles with 'make') ----------------------------------
command -v make >/dev/null 2>&1 || { echo "ERROR: make is required (build.sh runs 'make')."; exit 1; }
echo "[OK] make $(make --version | head -1 | sed 's/^GNU Make //')"

# --- FTXUI dependency ------------------------------------------------------
FTXUI_SRC="build/_deps/ftxui-src"

# A checkout counts as present only if it is actually populated (a stale empty
# directory must not look like a cached dependency).
ftxui_present() {
    [ -f "$FTXUI_SRC/CMakeLists.txt" ] || [ -d "$FTXUI_SRC/src" ]
}

# Report the checked-out version, but only from the checkout's own git repo -
# `git -C` would otherwise walk up into the project's repo if .git is missing.
ftxui_version() {
    if [ -e "$FTXUI_SRC/.git" ] || [ -f "$FTXUI_SRC/.git" ]; then
        git -C "$FTXUI_SRC" describe --tags --always 2>/dev/null || echo "cached"
    else
        echo "cached"
    fi
}

if ftxui_present; then
    echo "[OK] FTXUI $(ftxui_version) (cached in $FTXUI_SRC)"
elif [ "$PREFETCH" -eq 1 ]; then
    echo "Prefetching FTXUI dependency (fetch only, no compile)..."
    # Regenerate CMakeLists.txt from the template, exactly like build.sh does
    # (keep this block in sync with build.sh / build-windows.sh).
    source ./config.sh
    cp CMakeLists.txt.in CMakeLists.txt
    sed -i "s/<<TARGET_NAME>>/$APP_NAME/g" CMakeLists.txt
    SOURCES_TMP=$(mktemp)
    for s in "${SOURCES[@]}"; do
        echo "    $s" >> "$SOURCES_TMP"
    done
    sed -i "/^<<SOURCES>>$/{
        r $SOURCES_TMP
        d
    }" CMakeLists.txt
    rm -f "$SOURCES_TMP"
    for lib in "${LIBS[@]}"; do
        if [[ "$lib" == *::* ]]; then
            echo "target_link_libraries($APP_NAME PRIVATE $lib)" >> CMakeLists.txt
        else
            echo "target_link_libraries($APP_NAME PRIVATE \${CMAKE_SOURCE_DIR}/$lib)" >> CMakeLists.txt
        fi
    done
    mkdir -p build
    # Clear any stale/empty checkout so FetchContent's download step can clone
    # fresh (git clone refuses an existing non-empty directory).
    rm -rf "$FTXUI_SRC"
    cmake -S . -B build
    if ftxui_present; then
        echo "[OK] FTXUI $(ftxui_version) fetched into $FTXUI_SRC"
    else
        echo "WARNING: FTXUI fetch did not produce $FTXUI_SRC - check network and rerun."
    fi
else
    echo "FTXUI not fetched yet - it will be downloaded on first ./build.sh (needs network)."
    echo "  To fetch it now:  ./setup.sh --prefetch"
fi

# --- Windows cross-build (optional) ----------------------------------------
if command -v x86_64-w64-mingw32-gcc >/dev/null 2>&1; then
    echo "[OK] MinGW-w64 cross-compiler (Windows builds supported)"
else
    echo "MinGW-w64 not found - Windows builds (./build-windows.sh) will fail."
    echo "  Install it with:  sudo apt install -y mingw-w64"
fi

echo ""
echo "=== Setup complete ==="
echo "Run ./build.sh to build."
    )SH"};

    return output;
}




// Upgrade Script
////////////////////////////////////////////////////////////////////////////////
std::string createUpgrade()
{
    std::string output { R"SH(#!/bin/bash
#
# upgrade.sh - bump the FTXUI dependency to a newer release.
#
#   ./upgrade.sh              upgrade to the latest FTXUI release on GitHub
#   ./upgrade.sh <tag>        pin a specific tag, e.g. ./upgrade.sh v5.0.0
#
# What it does:
#   - reads the current pin from CMakeLists.txt.in (the template build.sh uses)
#   - updates it to the chosen/latest tag
#   - clears the cached FetchContent checkouts so the new tag is actually used
#   - runs a configure pass to fetch the new tag and verify the local patch
#     (patches/ftxui-c0-delivery.patch) still applies; on failure it rolls
#     the template back to the previous pin
#
# FTXUI upgrades can break the app compile if its API changed - configure
# passing here only means the patch applied; then run ./build.sh.

set -euo pipefail

REPO="https://github.com/ArthurSonzogni/FTXUI.git"
TEMPLATE="CMakeLists.txt.in"
PATCH="patches/ftxui-c0-delivery.patch"

command -v git >/dev/null 2>&1 || { echo "ERROR: git is required."; exit 1; }
[ -f "$TEMPLATE" ] || { echo "ERROR: $TEMPLATE not found. Run from the repo root."; exit 1; }
[ -f "$PATCH" ] || { echo "ERROR: $PATCH not found."; exit 1; }

CURRENT=$(grep -oP '^  GIT_TAG\s+\K\S+' "$TEMPLATE")
echo "FTXUI currently pinned at: $CURRENT"

if [ $# -ge 1 ]; then
    TARGET="$1"
    echo "Using requested tag: $TARGET"
    git ls-remote --tags --refs "$REPO" | grep -q "refs/tags/$TARGET\$" \
        || { echo "ERROR: tag '$TARGET' not found in $REPO."; exit 1; }
else
    echo "Querying latest FTXUI release from GitHub..."
    VER=$(git ls-remote --tags --refs "$REPO" \
        | grep -oP 'refs/tags/\K[vV][0-9]+(\.[0-9]+)+$' \
        | sort -V | tail -1)
    [ -n "$VER" ] || { echo "ERROR: could not determine the latest FTXUI tag (network?)."; exit 1; }
    TARGET="$VER"
fi

if [ "$TARGET" = "$CURRENT" ]; then
    echo "FTXUI is already at $TARGET - nothing to do."
    exit 0
fi

echo ""
echo "Upgrading FTXUI: $CURRENT -> $TARGET"

sed -i "s|^  GIT_TAG .*|  GIT_TAG $TARGET|" "$TEMPLATE"

# Clear cached FetchContent checkouts so the new tag is fetched on next build
for dep in build/_deps/ftxui-src build/_deps/ftxui-build \
           build-windows/_deps/ftxui-src build-windows/_deps/ftxui-build; do
    if [ -e "$dep" ]; then
        rm -rf "$dep"
        echo "Removed cached: $dep"
    fi
done

# Regenerate CMakeLists.txt from the template (same as build.sh) and run a
# configure pass to fetch the new tag and verify the patch still applies.
echo ""
echo "Verifying FTXUI $TARGET fetches and the patch applies..."
source ./config.sh
cp "$TEMPLATE" CMakeLists.txt
sed -i "s/<<TARGET_NAME>>/$APP_NAME/g" CMakeLists.txt
SOURCES_TMP=$(mktemp)
for s in "${SOURCES[@]}"; do
    echo "    $s" >> "$SOURCES_TMP"
done
sed -i "/^<<SOURCES>>$/{
    r $SOURCES_TMP
    d
}" CMakeLists.txt
rm -f "$SOURCES_TMP"
for lib in "${LIBS[@]}"; do
    if [[ "$lib" == *::* ]]; then
        echo "target_link_libraries($APP_NAME PRIVATE $lib)" >> CMakeLists.txt
    else
        echo "target_link_libraries($APP_NAME PRIVATE \${CMAKE_SOURCE_DIR}/$lib)" >> CMakeLists.txt
    fi
done

mkdir -p build
if ! cmake -S . -B build; then
    echo ""
    echo "ERROR: configure failed for FTXUI $TARGET."
    echo "  The local patch ($PATCH) no longer applies cleanly."
    echo ""
    echo "  For FTXUI >= v6.0.0 the CAN/SUB fix is already upstream:"
    echo "  Parse() returns SPECIAL for every C0 byte (incl. Ctrl+X/Ctrl+Z),"
    echo "  so the patch is obsolete - remove the 'PATCH_COMMAND ...' line"
    echo "  from $TEMPLATE and run ./upgrade.sh again."
    echo ""
    echo "  For older versions, re-baseline the patch against the new source"
    echo "  and run ./upgrade.sh again."
    echo ""
    echo "  Rolling back the pin to $CURRENT..."
    sed -i "s|^  GIT_TAG .*|  GIT_TAG $CURRENT|" "$TEMPLATE"
    rm -rf build/_deps/ftxui-src build/_deps/ftxui-build
    echo "Rolled back. Nothing was changed."
    exit 1
fi

echo ""
echo "Done: FTXUI pinned to $TARGET ($TEMPLATE)."
echo "Run ./build.sh to rebuild with the new FTXUI."
    )SH"};

    return output;
}
