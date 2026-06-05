#!/bin/bash

# Central configuration - edit values here, all scripts pick them up

APP_NAME="makemaker"

SOURCES=(
    "src/buildfiles.cpp"
    "src/fileops.cpp"
    "src/fileops_linux.cpp"
    "src/fileops_win.cpp"
    "src/main.cpp"
)

HEADERS=(
    "src/buildfiles.h"
    "src/fileops.h"
    "src/main.h"
)
