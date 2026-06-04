# ============================================================================
# Makemaker Makefile
# ============================================================================
# Usage:
#   make          - build the project
#   make clean    - remove build artifacts
#
# Customising for your project:
#   - Add source subdirectories to SRCS below (see "Source files" section)
#   - Add -I flags to CXXFLAGS for extra include paths
#   - Add -L and -l flags to LDFLAGS for external libraries
#   - Change TARGET to rename the output binary
# ============================================================================

# ---- Toolchain -------------------------------------------------------------
# Override on the command line, e.g.:  make CXX=clang++ CXXFLAGS="-O2"
CXX      ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic
LDFLAGS  ?=

# ---- Directories -----------------------------------------------------------
SRCDIR   := src
BUILDDIR := build
TARGET   := $(BUILDDIR)/makemaker

# ---- Source files ----------------------------------------------------------
# Flat (current dir only):
SRCS     := $(wildcard $(SRCDIR)/*.cpp)

# If you add subdirectories, extend with additional wildcard lines, e.g.:
# SRCS     += $(wildcard $(SRCDIR)/utils/*.cpp)
# SRCS     += $(wildcard $(SRCDIR)/core/*.cpp)
# SRCS     += $(wildcard $(SRCDIR)/**/*.cpp)          # recursive (GNU make 4+)

# To pull in a static library (.a) or object files from a dependency dir:
# SRCS     += deps/sqlite/sqlite3.c                   # C source needs %.c rule
# LDFLAGS  += deps/sqlite/libsqlite3.a                # static link

# ---- Derived file lists ----------------------------------------------------
OBJS     := $(SRCS:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
DEPS     := $(OBJS:.o=.d)

# ---- Phony targets ---------------------------------------------------------
.PHONY: all clean

all: $(TARGET)

# ---- Build directories -----------------------------------------------------
$(BUILDDIR):
	mkdir -p $@

# If you add subdirectory sources, also create their build dirs, e.g.:
# $(BUILDDIR)/utils $(BUILDDIR)/core:
# 	mkdir -p $@

# ---- Compilation rule ------------------------------------------------------
# Generic: compiles any $(SRCDIR)/%.cpp into $(BUILDDIR)/%.o
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# ---- Link rule -------------------------------------------------------------
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# ---- Clean -----------------------------------------------------------------
clean:
	rm -rf $(BUILDDIR)

# ---- Auto-dependency files -------------------------------------------------
-include $(DEPS)
