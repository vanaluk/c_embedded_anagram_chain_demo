# Anagram Chain Finder - Makefile
#
# Targets:
#   all            - Build native PC binary (default)
#   arm-baremetal  - Build for ARM Cortex-M3 bare-metal (QEMU lm3s6965evb)
#   arm-freertos   - Build for ARM Cortex-M3 with FreeRTOS (QEMU lm3s6965evb)
#   debug          - Build PC binary with debug symbols
#   test           - Build and run unit tests
#   clean          - Remove build artifacts
#
# Implementation selection:
#   IMPL=ai        - Use AI implementation (default)
#   IMPL=human     - Use human implementation
#   IMPL=both      - Build both implementations (for benchmarking)
#
# Memory mode (for human implementation only):
#   MEM=static     - Static memory pools (default, for embedded)
#   MEM=dynamic    - Dynamic memory allocation (optimized bulk alloc)
#
# Examples:
#   make IMPL=human                      - Build human impl with static memory
#   make IMPL=human MEM=dynamic          - Build human impl with dynamic memory
#   make IMPL=human MEM=dynamic arm-baremetal  - ARM with dynamic memory
#   make IMPL=both                       - Build ai and human-static for benchmarking

# ==============================================================================
# Compilers
# ==============================================================================

CC = gcc
CC_ARM_NONE = arm-none-eabi-gcc

# ==============================================================================
# Implementation Selection
# ==============================================================================

# IMPL can be 'ai', 'human', or 'both' (default: ai)
IMPL ?= ai

# MEM can be 'static' or 'dynamic' (default: static)
# Only applies to human implementation; ai always uses dynamic
MEM ?= static

# Handle IMPL=both specially
ifeq ($(IMPL),both)
    BUILD_BOTH := 1
    # For directory resolution, default to ai (will be overridden in build-both target)
    IMPL_DIR_RESOLVED = $(SRC_DIR)/impl/ai
else
    BUILD_BOTH := 0
    IMPL_DIR_RESOLVED = $(SRC_DIR)/impl/$(IMPL)
endif

# Memory mode and implementation-specific flags
ifeq ($(IMPL),ai)
    # AI implementation marker
    IMPL_FLAGS = -DIMPL_AI
    MEM_FLAGS =
else
    # Human implementation - memory mode is configurable
    IMPL_FLAGS =
    ifeq ($(MEM),dynamic)
        MEM_FLAGS = -DUSE_DYNAMIC_MEMORY
    else
        MEM_FLAGS =
    endif
endif

# ==============================================================================
# Directories
# ==============================================================================

SRC_DIR = src
IMPL_DIR = $(IMPL_DIR_RESOLVED)
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
# _DEFAULT_SOURCE enables POSIX/BSD/System V extensions (includes clock_gettime)
# -mcmodel=large needed for static arrays > 2GB (human impl with 1M words)
CFLAGS = -Wall -Wextra -std=c11 -O2 -D_DEFAULT_SOURCE -mcmodel=large $(IMPL_FLAGS) $(MEM_FLAGS) $(INCLUDES)
CFLAGS_DEBUG = -Wall -Wextra -std=c11 -g -O0 -DDEBUG -D_DEFAULT_SOURCE -mcmodel=large $(IMPL_FLAGS) $(MEM_FLAGS) $(INCLUDES)

# ARM bare-metal flags
CFLAGS_ARM_BAREMETAL = -Wall -Wextra -std=c11 -O2 -g \
    -mcpu=cortex-m3 -mthumb -mfloat-abi=soft \
    -ffunction-sections -fdata-sections \
    -specs=nano.specs -specs=nosys.specs \
    -DPLATFORM_ARM $(IMPL_FLAGS) $(MEM_FLAGS) \
    $(INCLUDES_ARM)

# ARM bare-metal debug flags
CFLAGS_ARM_DEBUG = -Wall -Wextra -std=c11 -O0 -g3 -DDEBUG \
    -mcpu=cortex-m3 -mthumb -mfloat-abi=soft \
    -specs=nano.specs -specs=nosys.specs \
    $(IMPL_FLAGS) $(MEM_FLAGS) $(INCLUDES_ARM)

LDFLAGS_ARM_BAREMETAL = -T $(ARM_DIR)/linker.ld \
    -Wl,--gc-sections \
    -nostartfiles

# ==============================================================================
# Source Files
# ==============================================================================

# Implementation source files
# Human implementation is split into core (algorithm/memory) and io (timer/validation/output)
ifeq ($(IMPL),human)
IMPL_SRC = $(IMPL_DIR)/anagram_chain_core.c $(IMPL_DIR)/anagram_chain_io.c
else
IMPL_SRC = $(IMPL_DIR)/anagram_chain.c
endif

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

# Test flags inherit from implementation flags
TEST_CFLAGS = $(IMPL_FLAGS) $(MEM_FLAGS)

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
TARGET_PC_NAMED = $(BIN_DIR)/anagram_chain_$(IMPL)
TARGET_PC_DEBUG = $(BIN_DIR)/anagram_chain_debug
TARGET_ARM_ELF = $(BIN_DIR)/anagram_chain_baremetal.elf
TARGET_FREERTOS_ELF = $(BIN_DIR)/anagram_chain_freertos.elf
TARGET_TEST_PC = $(BUILD_DIR)/test_pc
TARGET_TEST_PC_NAMED = $(BUILD_DIR)/test_$(IMPL)
TARGET_TEST_ARM = $(BIN_DIR)/test_baremetal.elf
TARGET_TEST_FREERTOS = $(BIN_DIR)/test_freertos.elf

# ==============================================================================
# Default Target
# ==============================================================================

.PHONY: all
ifeq ($(BUILD_BOTH),1)
all: build-both
else
all: $(TARGET_PC)
endif

# ==============================================================================
# PC Native Build
# ==============================================================================

$(TARGET_PC): $(MAIN_PC_SRC) $(IMPL_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(MAIN_PC_SRC) $(IMPL_SRC)
	@echo "Built: $@ (PC native, impl=$(IMPL), mem=$(MEM))"

# ==============================================================================
# PC Debug Build
# ==============================================================================

.PHONY: debug
debug: $(TARGET_PC_DEBUG)

$(TARGET_PC_DEBUG): $(MAIN_PC_SRC) $(IMPL_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $(MAIN_PC_SRC) $(IMPL_SRC)
	@echo "Built: $@ (PC debug, impl=$(IMPL), mem=$(MEM))"

# ==============================================================================
# ARM Bare-metal Build
# ==============================================================================

.PHONY: arm-baremetal
arm-baremetal: $(TARGET_ARM_ELF)

$(TARGET_ARM_ELF): $(MAIN_ARM_SRC) $(IMPL_SRC) $(ARM_SRCS) | $(BIN_DIR)
	$(CC_ARM_NONE) $(CFLAGS_ARM_BAREMETAL) $(LDFLAGS_ARM_BAREMETAL) \
		-o $@ $(ARM_SRCS) $(MAIN_ARM_SRC) $(IMPL_SRC)
	@echo "Built: $@ (ARM Cortex-M3 bare-metal, impl=$(IMPL), mem=$(MEM))"
	@arm-none-eabi-size $@ 2>/dev/null || true

# ==============================================================================
# ARM FreeRTOS Build
# ==============================================================================

.PHONY: arm-freertos
arm-freertos: $(TARGET_FREERTOS_ELF)

$(TARGET_FREERTOS_ELF): $(MAIN_FREERTOS_SRC) $(IMPL_SRC) $(ARM_SRCS) $(FREERTOS_SRCS) | $(BIN_DIR)
	$(CC_ARM_NONE) $(CFLAGS_ARM_BAREMETAL) $(LDFLAGS_ARM_BAREMETAL) \
		$(FREERTOS_INCLUDES) \
		-o $@ $(ARM_DIR)/startup.s $(ARM_DIR)/uart.c $(ARM_DIR)/syscalls.c \
		$(FREERTOS_SRCS) $(MAIN_FREERTOS_SRC) $(IMPL_SRC)
	@echo "Built: $@ (ARM Cortex-M3 + FreeRTOS, impl=$(IMPL), mem=$(MEM))"
	@arm-none-eabi-size $@ 2>/dev/null || true

# ==============================================================================
# Test Build and Run - PC
# ==============================================================================

.PHONY: test
test: $(TARGET_TEST_PC)
	@echo "Running PC tests..."
	@./$(TARGET_TEST_PC)

$(TARGET_TEST_PC): $(TEST_PC_SRC) $(TEST_CORE_SRC) $(IMPL_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_INCLUDES) $(TEST_CFLAGS) -o $@ $(TEST_PC_SRC) $(TEST_CORE_SRC) $(IMPL_SRC)
	@echo "Built: $@ (PC tests, impl=$(IMPL))"

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
# Check All Builds - verify all implementation/platform combinations compile
# ==============================================================================

.PHONY: check
check:
	@echo "========================================"
	@echo "  Checking all build configurations"
	@echo "========================================"
	@echo ""
	@rm -rf build bin
	@# AI implementation (dynamic memory only)
	@echo "[1/9] AI - PC..."
	@$(MAKE) --no-print-directory IMPL=ai test > /dev/null 2>&1 && echo "  PASS" || (echo "  FAIL"; exit 1)
	@rm -rf build bin
	@echo "[2/9] AI - ARM bare-metal..."
	@$(MAKE) --no-print-directory IMPL=ai arm-baremetal > /dev/null 2>&1 && echo "  PASS" || (echo "  FAIL"; exit 1)
	@rm -rf build bin
	@echo "[3/9] AI - ARM FreeRTOS..."
	@$(MAKE) --no-print-directory IMPL=ai arm-freertos > /dev/null 2>&1 && echo "  PASS" || (echo "  FAIL"; exit 1)
	@rm -rf build bin
	@# Human implementation - static memory
	@echo "[4/9] Human (static) - PC..."
	@$(MAKE) --no-print-directory IMPL=human MEM=static test > /dev/null 2>&1 && echo "  PASS" || (echo "  FAIL"; exit 1)
	@rm -rf build bin
	@echo "[5/9] Human (static) - ARM bare-metal..."
	@$(MAKE) --no-print-directory IMPL=human MEM=static arm-baremetal > /dev/null 2>&1 && echo "  PASS" || (echo "  FAIL"; exit 1)
	@rm -rf build bin
	@echo "[6/9] Human (static) - ARM FreeRTOS..."
	@$(MAKE) --no-print-directory IMPL=human MEM=static arm-freertos > /dev/null 2>&1 && echo "  PASS" || (echo "  FAIL"; exit 1)
	@rm -rf build bin
	@# Human implementation - dynamic memory
	@echo "[7/9] Human (dynamic) - PC..."
	@$(MAKE) --no-print-directory IMPL=human MEM=dynamic test > /dev/null 2>&1 && echo "  PASS" || (echo "  FAIL"; exit 1)
	@rm -rf build bin
	@echo "[8/9] Human (dynamic) - ARM bare-metal..."
	@$(MAKE) --no-print-directory IMPL=human MEM=dynamic arm-baremetal > /dev/null 2>&1 && echo "  PASS" || (echo "  FAIL"; exit 1)
	@rm -rf build bin
	@echo "[9/9] Human (dynamic) - ARM FreeRTOS..."
	@$(MAKE) --no-print-directory IMPL=human MEM=dynamic arm-freertos > /dev/null 2>&1 && echo "  PASS" || (echo "  FAIL"; exit 1)
	@rm -rf build bin
	@echo ""
	@echo "========================================"
	@echo "  All 9 configurations PASSED"
	@echo "========================================"

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
	docker build -t anagram-chain --target runtime -f docker/Dockerfile .

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

.PHONY: docker-lint
docker-lint:
	docker build -t anagram-chain-lint --target lint -f docker/Dockerfile .
	docker run --rm anagram-chain-lint

# ==============================================================================
# Run Targets (local QEMU)
# ==============================================================================

.PHONY: debug-baremetal
debug-baremetal: $(TARGET_ARM_ELF)
	@echo "Starting QEMU with GDB server on port 1234..."
	@echo "Connect with: arm-none-eabi-gdb -ex 'target remote :1234' $(TARGET_ARM_ELF)"
	qemu-system-arm -M lm3s6965evb -nographic -S -gdb tcp::1234 -kernel $(TARGET_ARM_ELF)

.PHONY: debug-freertos
debug-freertos: $(TARGET_FREERTOS_ELF)
	@echo "Starting QEMU with GDB server on port 1234..."
	@echo "Connect with: arm-none-eabi-gdb -ex 'target remote :1234' $(TARGET_FREERTOS_ELF)"
	qemu-system-arm -M lm3s6965evb -nographic -S -gdb tcp::1234 -kernel $(TARGET_FREERTOS_ELF)

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
	find src tests -name '*.c' -o -name '*.h' | xargs clang-format -i

# ==============================================================================
# Benchmark Targets
# ==============================================================================

.PHONY: build-both
build-both: | $(BIN_DIR)
	@echo "Building both implementations..."
	@$(MAKE) --no-print-directory IMPL=ai
	@mv $(TARGET_PC) $(BIN_DIR)/anagram_chain_ai
	@$(MAKE) --no-print-directory IMPL=human
	@mv $(TARGET_PC) $(BIN_DIR)/anagram_chain_human
	@echo "Built: bin/anagram_chain_ai and bin/anagram_chain_human"

# Build all three implementations for benchmarking
.PHONY: build-all
build-all: | $(BIN_DIR)
	@echo "Building all three implementations..."
	@$(MAKE) --no-print-directory IMPL=ai
	@mv $(TARGET_PC) $(BIN_DIR)/anagram_chain_ai
	@$(MAKE) --no-print-directory IMPL=human MEM=static
	@mv $(TARGET_PC) $(BIN_DIR)/anagram_chain_human-static
	@$(MAKE) --no-print-directory IMPL=human MEM=dynamic
	@mv $(TARGET_PC) $(BIN_DIR)/anagram_chain_human-dynamic
	@echo "Built: bin/anagram_chain_ai, bin/anagram_chain_human-static, bin/anagram_chain_human-dynamic"

.PHONY: benchmark
benchmark:
	@echo "Running benchmark (builds all implementations automatically)..."
	@python3 benchmark.py $(ARGS)

# Stress test parameters (can be overridden)
STRESS_CHAINS ?= 5000
STRESS_LENGTH ?= 15
STRESS_FILE ?= tests/data/stress.txt
STRESS_WORD ?= fu
STRESS_RUNS ?= 3

.PHONY: generate-stress
generate-stress:
	@echo "Generating stress test dictionary..."
	@echo "  Chains: $(STRESS_CHAINS), Max length: $(STRESS_LENGTH)"
	python3 tests/data/generate_stress_dict.py $(STRESS_FILE) $(STRESS_CHAINS) $(STRESS_LENGTH)
	@echo ""
	@echo "Usage: make benchmark ARGS='$(STRESS_FILE) <start_word> <runs>'"

# Run stress test with current IMPL (ai or human)
.PHONY: stress
stress: $(TARGET_PC)
	@echo "Running stress test with IMPL=$(IMPL)..."
	@echo "Dictionary: $(STRESS_FILE), Start: $(STRESS_WORD), Runs: $(STRESS_RUNS)"
	@echo ""
	@for i in $$(seq 1 $(STRESS_RUNS)); do \
		echo "=== Run $$i/$(STRESS_RUNS) ==="; \
		./$(TARGET_PC) $(STRESS_FILE) $(STRESS_WORD); \
		echo ""; \
	done

# ==============================================================================
# Code Quality / Lint
# ==============================================================================

.PHONY: lint
lint:
	@echo "========================================"
	@echo "  Running static analysis"
	@echo "========================================"
	@echo ""
	@echo "=== clang-format (check only) ==="
	@find src tests -name '*.c' -o -name '*.h' | xargs clang-format --dry-run --Werror 2>&1 || echo "  Formatting issues found"
	@echo ""
	@echo "=== cppcheck ==="
	@cppcheck --enable=warning,style,performance,portability \
		--suppress=missingIncludeSystem \
		--suppress=unusedFunction \
		--suppress=variableScope \
		--error-exitcode=0 \
		--inline-suppr \
		-I src/include \
		src/ tests/ 2>&1 || true
	@echo ""
	@echo "=== clang-tidy (human implementation) ==="
	@clang-tidy src/impl/human/*.c src/main/main_pc.c \
		-checks='-*,bugprone-*,cert-*,clang-analyzer-*,misc-*,performance-*,portability-*,-bugprone-easily-swappable-parameters,-cert-err33-c,-misc-no-recursion,-bugprone-narrowing-conversions,-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling' \
		-- -I src/include -I src/impl/human -std=c11 -D_POSIX_C_SOURCE=199309L 2>&1 || true
	@echo ""
	@echo "=== clang-tidy (AI implementation) ==="
	@clang-tidy src/impl/ai/*.c \
		-checks='-*,bugprone-*,cert-*,clang-analyzer-*,misc-*,performance-*,portability-*,-bugprone-easily-swappable-parameters,-cert-err33-c,-misc-no-recursion,-bugprone-narrowing-conversions,-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling' \
		-- -I src/include -std=c11 -DIMPL_AI -D_POSIX_C_SOURCE=199309L 2>&1 || true
	@echo ""
	@echo "========================================"
	@echo "  Static analysis complete"
	@echo "========================================"

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
	@echo "    debug-baremetal        - Start QEMU+GDB server for bare-metal"
	@echo "    debug-freertos         - Start QEMU+GDB server for FreeRTOS"
	@echo ""
	@echo "  Docker:"
	@echo "    docker-build           - Build Docker image (PC)"
	@echo "    docker-build-baremetal - Build Docker image (bare-metal)"
	@echo "    docker-build-freertos  - Build Docker image (FreeRTOS)"
	@echo "    docker-run             - Run PC binary in Docker"
	@echo "    docker-run-baremetal   - Run bare-metal in Docker+QEMU"
	@echo "    docker-run-freertos    - Run FreeRTOS in Docker+QEMU"
	@echo "    docker-lint            - Run static analysis in Docker"
	@echo ""
	@echo "  Utilities:"
	@echo "    format                 - Format code with clang-format"
	@echo "    lint                   - Run static analysis (cppcheck, clang-tidy)"
	@echo "    check                  - Build and test all 9 configurations"
	@echo "    clean                  - Remove build artifacts"
	@echo "    help                   - Show this help"
	@echo ""
	@echo "  Benchmark:"
	@echo "    generate-stress        - Generate stress test dictionary"
	@echo "      STRESS_CHAINS=5000   - Number of chain groups (default: 5000)"
	@echo "      STRESS_LENGTH=15     - Max chain length (default: 15)"
	@echo "      STRESS_FILE=...      - Output file (default: tests/data/stress.txt)"
	@echo "    stress                 - Run stress test with current IMPL"
	@echo "      STRESS_WORD=fu       - Start word (default: fu)"
	@echo "      STRESS_RUNS=3        - Number of runs (default: 3)"
	@echo "    benchmark              - Build both and compare AI vs Human"
	@echo ""
	@echo "  Implementation selection:"
	@echo "    IMPL=ai                - Use AI implementation (default)"
	@echo "    IMPL=human             - Use human implementation"
	@echo "    IMPL=both              - Build both: anagram_chain_ai + anagram_chain_human"
	@echo ""
	@echo "  Examples:"
	@echo "    make                   - Build with AI (default)"
	@echo "    make IMPL=human        - Build with human implementation"
	@echo "    make IMPL=both         - Build both binaries for comparison"
	@echo "    make IMPL=human test   - Test human implementation"
	@echo "    make generate-stress   - Generate default stress dictionary"
	@echo "    make generate-stress STRESS_CHAINS=1000 STRESS_LENGTH=10  - Small dict"
	@echo "    make IMPL=human stress - Run stress test with human impl"
	@echo "    make IMPL=ai stress    - Run stress test with AI impl"
	@echo "    make benchmark ARGS='tests/data/stress.txt fu 3' - Compare both"
