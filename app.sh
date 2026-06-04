#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

EXECUTABLE=""
case "$(uname -s)" in
    CYGWIN*|MINGW*|MSYS*) EXECUTABLE="$SCRIPT_DIR/build/makemaker.exe" ;;
    *)                    EXECUTABLE="$SCRIPT_DIR/build/makemaker" ;;
esac

exec "$EXECUTABLE" "$@"
