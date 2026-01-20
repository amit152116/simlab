# Project-specific defaults
target ?=
compiler ?=

# Build script (installed system-wide)
BUILD_SCRIPT := cbuild

# Include master makefile
include $(HOME)/.dotfiles/scripts/cmake.mk


# Run the program
.PHONY: run
run:
	@echo "▶ Running $(target)..."
	./build/bin/$(target); \
