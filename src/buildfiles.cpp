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




void buildCMakeList(const std::string& full_path)
{
    fs::create_directories(full_path + "/src");
    writeScript(full_path + "/config.sh", createCMakeConfig());
    writeScript(full_path + "/CMakeLists.txt.in", createCMakeTmplt());
    writeScript(full_path + "/build.sh", createCMakeBuild());
    writeScript(full_path + "/build-windows.sh", createCMakeBuildWindows());
    writeScript(full_path + "/leakcheck.sh", createCMakeLeak());
    writeScript(full_path + "/run.sh", createCMakeRun());
    writeScript(full_path + "/app.sh", createCMakeApp());
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

HEADERS=(
    "src/main.h"
    "src/utils.h"
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

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create executable
add_executable(<<TARGET_NAME>>
<<SOURCES>>
)

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
./build-windows.sh

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

# Windows build script for your app
# Compiles your code into a Windows executable using MinGW-w64

set -e

echo "Building $APP_NAME for Windows..."

# Determine build type
BUILD_TYPE_FLAGS=""
BUILD_MESSAGE="DEBUG"
if [ "$1" == "r" ]; then
    BUILD_TYPE_FLAGS="-s" # Strip all symbol tables
    BUILD_MESSAGE="RELEASE"
fi
echo "Performing $BUILD_MESSAGE build."


# Check if source files exist - you can add or remove whichever files you like
for f in "${SOURCES[@]}" "${HEADERS[@]}"; do
    if [ ! -f "$f" ]; then
        echo "Error: $f not found"
        exit 1
    fi
done

# Check for MinGW-w64 compiler
if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo "Error: MinGW-w64 compiler not found"
    echo "Install with: sudo apt install mingw-w64"
    exit 1
fi

# Create build directory
mkdir -p build-windows

# Compile with MinGW-w64 for Windows
echo "Compiling with MinGW-w64..."
x86_64-w64-mingw32-g++ -std=c++17 \
    -O2 \
    -Wall \
    -Wextra \
    -D_WIN32 \
    -static-libgcc \
    -static-libstdc++ \
    -static \
    $BUILD_TYPE_FLAGS \
    -mwindows \
    -o "build-windows/$APP_NAME.exe" \
    "${SOURCES[@]}" \
    -luser32 \
    -lgdi32 \
    -lkernel32 \
    -lwinmm




# Add any custom cp's or other actions here
# mkdir -p "./build-windows/data/themes"
# cp -r ./themes/* "./build-windows/data/themes/" 2>/dev/null || true




echo "Build complete: build-windows/$APP_NAME.exe"
echo "Executable size: $(du -h "build-windows/$APP_NAME.exe" | cut -f1)"
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
