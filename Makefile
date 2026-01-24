# Anagram Chain Finder - Makefile
#
# Targets:
#   all            - Build native PC binary (default)
#   arm-baremetal  - Build for ARM Cortex-M3 bare-metal (QEMU lm3s6965evb)
#   arm-freertos   - Build for ARM Cortex-M3 with FreeRTOS (QEMU lm3s6965evb)
#   debug          - Build PC binary with debug symbols
#   test           - Build and run unit tests
#   clean          - Remove build artifacts

# ==============================================================================
# Compilers
# ==============================================================================

CC = gcc
CC_ARM_NONE = arm-none-eabi-gcc

# ==============================================================================
# Directories
# ==============================================================================

SRC_DIR = src
IMPL_DIR = $(SRC_DIR)/implementation
INCLUDE_DIR = $(SRC_DIR)/include
MAIN_DIR = $(SRC_DIR)/main
ARM_DIR = arm
TEST_DIR = tests
BUILD_DIR = build
BIN_DIR = bin

# ==============================================================================
# Flags
# ==============================================================================

# Common include paths
INCLUDES = -I$(INCLUDE_DIR)
INCLUDES_ARM = $(INCLUDES) -I$(ARM_DIR)

# PC flags
CFLAGS = -Wall -Wextra -std=c11 -O2 $(INCLUDES)
CFLAGS_DEBUG = -Wall -Wextra -std=c11 -g -O0 -DDEBUG $(INCLUDES)

# ARM bare-metal flags
CFLAGS_ARM_BAREMETAL = -Wall -Wextra -std=c11 -O2 \
    -mcpu=cortex-m3 -mthumb -mfloat-abi=soft \
    -ffunction-sections -fdata-sections \
    -specs=nano.specs -specs=nosys.specs \
    $(INCLUDES_ARM)

LDFLAGS_ARM_BAREMETAL = -T $(ARM_DIR)/linker.ld \
    -Wl,--gc-sections \
    -nostartfiles

# ==============================================================================
# Source Files
# ==============================================================================

# Implementation (library)
IMPL_SRC = $(IMPL_DIR)/anagram_chain.c

# Main files for each target
MAIN_PC_SRC = $(MAIN_DIR)/main_pc.c
MAIN_ARM_SRC = $(MAIN_DIR)/main_arm.c
MAIN_FREERTOS_SRC = $(MAIN_DIR)/main_freertos.c

# ARM startup and drivers
ARM_SRCS = $(ARM_DIR)/startup.s $(ARM_DIR)/uart.c $(ARM_DIR)/syscalls.c

# Tests
TEST_CORE_SRC = $(TEST_DIR)/test_core.c
TEST_PC_SRC = $(TEST_DIR)/test_main_pc.c
TEST_ARM_SRC = $(TEST_DIR)/test_main_arm.c
TEST_FREERTOS_SRC = $(TEST_DIR)/test_main_freertos.c
TEST_INCLUDES = -I$(TEST_DIR) -I$(INCLUDE_DIR)

# FreeRTOS sources (downloaded by Docker)
FREERTOS_DIR ?= $(ARM_DIR)/freertos/FreeRTOS-Kernel
FREERTOS_PORT_DIR = $(FREERTOS_DIR)/portable/GCC/ARM_CM3
FREERTOS_HEAP_DIR = $(FREERTOS_DIR)/portable/MemMang

FREERTOS_SRCS = $(FREERTOS_DIR)/tasks.c \
                $(FREERTOS_DIR)/list.c \
                $(FREERTOS_DIR)/queue.c \
                $(FREERTOS_PORT_DIR)/port.c \
                $(FREERTOS_HEAP_DIR)/heap_4.c

FREERTOS_INCLUDES = -I$(ARM_DIR)/freertos \
                    -I$(FREERTOS_DIR)/include \
                    -I$(FREERTOS_PORT_DIR)

# ==============================================================================
# Output Binaries
# ==============================================================================

TARGET_PC = $(BIN_DIR)/anagram_chain
TARGET_PC_DEBUG = $(BIN_DIR)/anagram_chain_debug
TARGET_ARM_ELF = $(BIN_DIR)/anagram_chain_baremetal.elf
TARGET_FREERTOS_ELF = $(BIN_DIR)/anagram_chain_freertos.elf
TARGET_TEST_PC = $(BUILD_DIR)/test_pc
TARGET_TEST_ARM = $(BIN_DIR)/test_baremetal.elf
TARGET_TEST_FREERTOS = $(BIN_DIR)/test_freertos.elf

# ==============================================================================
# Default Target
# ==============================================================================

.PHONY: all
all: $(TARGET_PC)

# ==============================================================================
# PC Native Build
# ==============================================================================

$(TARGET_PC): $(MAIN_PC_SRC) $(IMPL_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(MAIN_PC_SRC) $(IMPL_SRC)
	@echo "Built: $@ (PC native)"

# ==============================================================================
# PC Debug Build
# ==============================================================================

.PHONY: debug
debug: $(TARGET_PC_DEBUG)

$(TARGET_PC_DEBUG): $(MAIN_PC_SRC) $(IMPL_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $(MAIN_PC_SRC) $(IMPL_SRC)
	@echo "Built: $@ (PC debug)"

# ==============================================================================
# ARM Bare-metal Build
# ==============================================================================

.PHONY: arm-baremetal
arm-baremetal: $(TARGET_ARM_ELF)

$(TARGET_ARM_ELF): $(MAIN_ARM_SRC) $(IMPL_SRC) $(ARM_SRCS) | $(BIN_DIR)
	$(CC_ARM_NONE) $(CFLAGS_ARM_BAREMETAL) $(LDFLAGS_ARM_BAREMETAL) \
		-o $@ $(ARM_SRCS) $(MAIN_ARM_SRC) $(IMPL_SRC)
	@echo "Built: $@ (ARM Cortex-M3 bare-metal)"
	@arm-none-eabi-size $@ 2>/dev/null || true

# ==============================================================================
# ARM FreeRTOS Build
# ==============================================================================

.PHONY: arm-freertos
arm-freertos: $(TARGET_FREERTOS_ELF)

$(TARGET_FREERTOS_ELF): $(MAIN_FREERTOS_SRC) $(IMPL_SRC) $(ARM_SRCS) $(FREERTOS_SRCS) | $(BIN_DIR)
	$(CC_ARM_NONE) $(CFLAGS_ARM_BAREMETAL) $(LDFLAGS_ARM_BAREMETAL) \
		$(FREERTOS_INCLUDES) \
		-o $@ $(ARM_DIR)/startup.s $(ARM_DIR)/uart.c \
		$(FREERTOS_SRCS) $(MAIN_FREERTOS_SRC) $(IMPL_SRC)
	@echo "Built: $@ (ARM Cortex-M3 + FreeRTOS)"
	@arm-none-eabi-size $@ 2>/dev/null || true

# ==============================================================================
# Test Build and Run - PC
# ==============================================================================

.PHONY: test
test: $(TARGET_TEST_PC)
	@echo "Running PC tests..."
	@./$(TARGET_TEST_PC)

$(TARGET_TEST_PC): $(TEST_PC_SRC) $(TEST_CORE_SRC) $(IMPL_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_INCLUDES) -o $@ $(TEST_PC_SRC) $(TEST_CORE_SRC) $(IMPL_SRC)
	@echo "Built: $@ (PC tests)"

# ==============================================================================
# Test Build and Run - ARM Bare-metal
# ==============================================================================

.PHONY: test-arm
test-arm: $(TARGET_TEST_ARM)
	@echo "Running ARM bare-metal tests in QEMU..."
	@qemu-system-arm -M lm3s6965evb -nographic -kernel $(TARGET_TEST_ARM)

$(TARGET_TEST_ARM): $(TEST_ARM_SRC) $(TEST_CORE_SRC) $(IMPL_SRC) $(ARM_SRCS) | $(BIN_DIR)
	$(CC_ARM_NONE) $(CFLAGS_ARM_BAREMETAL) $(LDFLAGS_ARM_BAREMETAL) \
		-DPLATFORM_ARM $(TEST_INCLUDES) \
		-o $@ $(ARM_SRCS) $(TEST_ARM_SRC) $(TEST_CORE_SRC) $(IMPL_SRC)
	@echo "Built: $@ (ARM bare-metal tests)"
	@arm-none-eabi-size $@ 2>/dev/null || true

# ==============================================================================
# Test Build and Run - ARM FreeRTOS
# ==============================================================================

.PHONY: test-freertos
test-freertos: $(TARGET_TEST_FREERTOS)
	@echo "Running FreeRTOS tests in QEMU..."
	@qemu-system-arm -M lm3s6965evb -nographic -kernel $(TARGET_TEST_FREERTOS)

$(TARGET_TEST_FREERTOS): $(TEST_FREERTOS_SRC) $(TEST_CORE_SRC) $(IMPL_SRC) $(ARM_SRCS) $(FREERTOS_SRCS) | $(BIN_DIR)
	$(CC_ARM_NONE) $(CFLAGS_ARM_BAREMETAL) $(LDFLAGS_ARM_BAREMETAL) \
		-DPLATFORM_ARM $(TEST_INCLUDES) $(FREERTOS_INCLUDES) \
		-o $@ $(ARM_DIR)/startup.s $(ARM_DIR)/uart.c $(ARM_DIR)/syscalls.c \
		$(FREERTOS_SRCS) $(TEST_FREERTOS_SRC) $(TEST_CORE_SRC) $(IMPL_SRC)
	@echo "Built: $@ (FreeRTOS tests)"
	@arm-none-eabi-size $@ 2>/dev/null || true

# ==============================================================================
# Test All Platforms
# ==============================================================================

.PHONY: test-all
test-all: test test-arm test-freertos
	@echo "All platform tests completed"

# ==============================================================================
# Create Directories
# ==============================================================================

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# ==============================================================================
# Docker Targets
# ==============================================================================

.PHONY: docker-build
docker-build:
	docker build -t anagram-chain -f docker/Dockerfile .

.PHONY: docker-run
docker-run:
	docker run --rm anagram-chain $(ARGS)

.PHONY: docker-build-baremetal
docker-build-baremetal:
	docker build -t anagram-chain-baremetal --target baremetal -f docker/Dockerfile .

.PHONY: docker-run-baremetal
docker-run-baremetal:
	docker run --rm -it anagram-chain-baremetal

.PHONY: docker-build-freertos
docker-build-freertos:
	docker build -t anagram-chain-freertos --target freertos -f docker/Dockerfile .

.PHONY: docker-run-freertos
docker-run-freertos:
	docker run --rm -it anagram-chain-freertos

# Docker test targets
.PHONY: docker-build-test-baremetal
docker-build-test-baremetal:
	docker build -t anagram-chain-test-baremetal --target test-baremetal -f docker/Dockerfile .

.PHONY: docker-run-test-baremetal
docker-run-test-baremetal:
	docker run --rm -it anagram-chain-test-baremetal

.PHONY: docker-build-test-freertos
docker-build-test-freertos:
	docker build -t anagram-chain-test-freertos --target test-freertos -f docker/Dockerfile .

.PHONY: docker-run-test-freertos
docker-run-test-freertos:
	docker run --rm -it anagram-chain-test-freertos

# ==============================================================================
# Run Targets (local QEMU)
# ==============================================================================

.PHONY: run-baremetal
run-baremetal: $(TARGET_ARM_ELF)
	qemu-system-arm -M lm3s6965evb -nographic -kernel $(TARGET_ARM_ELF)

.PHONY: run-freertos
run-freertos: $(TARGET_FREERTOS_ELF)
	qemu-system-arm -M lm3s6965evb -nographic -kernel $(TARGET_FREERTOS_ELF)

# ==============================================================================
# Format Code
# ==============================================================================

.PHONY: format
format:
	clang-format -i $(IMPL_DIR)/*.c $(MAIN_DIR)/*.c $(TEST_DIR)/*.c $(INCLUDE_DIR)/*.h 2>/dev/null || true

# ==============================================================================
# Clean
# ==============================================================================

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Cleaned build artifacts"

# ==============================================================================
# Help
# ==============================================================================

.PHONY: help
help:
	@echo "Available targets:"
	@echo ""
	@echo "  Build:"
	@echo "    all                    - Build PC native binary (default)"
	@echo "    debug                  - Build PC binary with debug symbols"
	@echo "    arm-baremetal          - Build ARM Cortex-M3 bare-metal binary"
	@echo "    arm-freertos           - Build ARM Cortex-M3 FreeRTOS binary"
	@echo ""
	@echo "  Tests:"
	@echo "    test                   - Build and run PC unit tests"
	@echo "    test-arm               - Build and run ARM bare-metal tests in QEMU"
	@echo "    test-freertos          - Build and run FreeRTOS tests in QEMU"
	@echo "    test-all               - Run tests on all platforms"
	@echo ""
	@echo "  Run locally:"
	@echo "    run-baremetal          - Run bare-metal in QEMU"
	@echo "    run-freertos           - Run FreeRTOS in QEMU"
	@echo ""
	@echo "  Docker:"
	@echo "    docker-build           - Build Docker image (PC)"
	@echo "    docker-build-baremetal - Build Docker image (bare-metal)"
	@echo "    docker-build-freertos  - Build Docker image (FreeRTOS)"
	@echo "    docker-run             - Run PC binary in Docker"
	@echo "    docker-run-baremetal   - Run bare-metal in Docker+QEMU"
	@echo "    docker-run-freertos    - Run FreeRTOS in Docker+QEMU"
	@echo ""
	@echo "  Utilities:"
	@echo "    format                 - Format code with clang-format"
	@echo "    clean                  - Remove build artifacts"
	@echo "    help                   - Show this help"
