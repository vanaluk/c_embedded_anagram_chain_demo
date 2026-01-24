# Anagram Chain Finder - Makefile
#
# Targets:
#   all       - Build native binary (default)
#   arm       - Cross-compile for ARM
#   debug     - Build with debug symbols
#   test      - Build and run unit tests
#   clean     - Remove build artifacts
#   docker-build - Build Docker image
#   docker-run   - Run in Docker container

# Compilers
CC = gcc
CC_ARM = arm-linux-gnueabihf-gcc

# Flags
CFLAGS = -Wall -Wextra -std=c11 -O2
CFLAGS_DEBUG = -Wall -Wextra -std=c11 -g -O0 -DDEBUG
CFLAGS_ARM = -Wall -Wextra -std=c11 -O2 -static

# Directories
SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build
BIN_DIR = bin

# Source files
MAIN_SRC = $(SRC_DIR)/anagram_chain.c
TEST_SRC = $(TEST_DIR)/test_anagram.c

# Output binaries
TARGET = $(BIN_DIR)/anagram_chain
TARGET_ARM = $(BIN_DIR)/anagram_chain_arm
TARGET_DEBUG = $(BIN_DIR)/anagram_chain_debug
TARGET_TEST = $(BUILD_DIR)/test_anagram

# Default target
.PHONY: all
all: $(TARGET)

# Native build
$(TARGET): $(MAIN_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<
	@echo "Built: $@"

# ARM cross-compilation
.PHONY: arm
arm: $(TARGET_ARM)

$(TARGET_ARM): $(MAIN_SRC) | $(BIN_DIR)
	$(CC_ARM) $(CFLAGS_ARM) -o $@ $<
	@echo "Built: $@ (ARM)"

# Debug build
.PHONY: debug
debug: $(TARGET_DEBUG)

$(TARGET_DEBUG): $(MAIN_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $<
	@echo "Built: $@ (Debug)"

# Test build and run
.PHONY: test
test: $(TARGET_TEST)
	@echo "Running tests..."
	@./$(TARGET_TEST)

$(TARGET_TEST): $(TEST_SRC) $(MAIN_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DTEST_BUILD -o $@ $(TEST_SRC) -I$(SRC_DIR)
	@echo "Built: $@"

# Create directories
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Docker targets
.PHONY: docker-build
docker-build:
	docker build -t anagram-chain -f docker/Dockerfile .

.PHONY: docker-run
docker-run:
	docker run --rm anagram-chain $(ARGS)

.PHONY: docker-run-arm
docker-run-arm:
	docker run --rm --entrypoint qemu-arm anagram-chain -L /usr/arm-linux-gnueabihf ./bin/anagram_chain_arm $(ARGS)

# Format code
.PHONY: format
format:
	clang-format -i $(SRC_DIR)/*.c $(TEST_DIR)/*.c 2>/dev/null || true

# Clean
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Cleaned build artifacts"

# Help
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all          - Build native binary (default)"
	@echo "  arm          - Cross-compile for ARM"
	@echo "  debug        - Build with debug symbols"
	@echo "  test         - Build and run unit tests"
	@echo "  docker-build - Build Docker image"
	@echo "  docker-run   - Run in Docker (use ARGS=\"...\")"
	@echo "  format       - Format code with clang-format"
	@echo "  clean        - Remove build artifacts"
	@echo "  help         - Show this help"
