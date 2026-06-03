#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR"

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
