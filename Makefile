.PHONY: all clean build rebuild test

# Project configuration
BUILD_DIR = build
BIN_DIR := $(BUILD_DIR)/bin

# Default target
all: build

# Create build directory and generate CMake files
cmake_init:
	@cmake -B $(BUILD_DIR) -S .

# Build the project
build: cmake_init
	@cd $(BUILD_DIR) && make -j$(shell nproc)

run_%:
	@echo "Running program: $*"
	@$(BIN_DIR)/$*

# Clean build directory
clean:
	@rm -rf $(BUILD_DIR)

# Rebuild the project
rebuild: clean build

# Show help
help:
	@echo "Available targets:"
	@echo "  make          - Build the project"
	@echo "  make clean    - Clean build files"
	@echo "  make rebuild  - Clean and rebuild"
	@echo "  make help     - Show this help message"


