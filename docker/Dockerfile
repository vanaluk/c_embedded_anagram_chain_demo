# Multi-stage Dockerfile for Embedded Anagram Chain Demo
#
# Stages:
#   builder       - Build environment with compilers and static analysis tools
#   runtime       - Minimal runtime for PC binary
#   baremetal     - ARM Cortex-M3 bare-metal with QEMU system emulation
#   freertos      - ARM Cortex-M3 with FreeRTOS and QEMU system emulation
#   test-baremetal - ARM bare-metal tests with QEMU
#   test-freertos  - ARM FreeRTOS tests with QEMU
#   lint          - Static analysis (clang-format, clang-tidy, cppcheck)

# ==============================================================================
# Stage 1: Builder
# ==============================================================================
FROM ubuntu:22.04 AS builder

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install build tools (including ARM bare-metal toolchain and static analysis)
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-arm-none-eabi \
    libnewlib-arm-none-eabi \
    make \
    git \
    clang-format \
    clang-tidy \
    cppcheck \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source files (including FreeRTOS-Kernel submodule)
COPY src/ ./src/
COPY arm/ ./arm/
COPY tests/ ./tests/
COPY Makefile ./

# Note: FreeRTOS-Kernel is expected to be in arm/freertos/FreeRTOS-Kernel
# It should be initialized via git submodule before docker build
# If submodule is not initialized, clone it here as fallback
RUN if [ ! -f "arm/freertos/FreeRTOS-Kernel/tasks.c" ]; then \
        git clone --depth 1 --branch V11.1.0 \
            https://github.com/FreeRTOS/FreeRTOS-Kernel.git \
            arm/freertos/FreeRTOS-Kernel; \
    fi

# Build PC native binary
RUN make clean 2>/dev/null || true && make

# Build ARM bare-metal binary
RUN make arm-baremetal

# Build ARM FreeRTOS binary
RUN make arm-freertos

# Build PC tests
RUN make build/test_pc || true

# Build ARM bare-metal tests
RUN make bin/test_baremetal.elf

# Build ARM FreeRTOS tests
RUN make bin/test_freertos.elf

# ==============================================================================
# Stage 2: PC Runtime
# ==============================================================================
FROM ubuntu:22.04 AS runtime

WORKDIR /app

# Copy binaries and test data from builder
COPY --from=builder /app/bin/anagram_chain ./bin/
COPY --from=builder /app/tests/data/ ./tests/data/

# Default command
ENTRYPOINT ["./bin/anagram_chain"]
CMD ["--help"]

# ==============================================================================
# Stage 3: ARM Bare-metal with QEMU System Emulation
# ==============================================================================
FROM ubuntu:22.04 AS baremetal

# Install QEMU system emulation for ARM
RUN apt-get update && apt-get install -y \
    qemu-system-arm \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy bare-metal ELF binary from builder
COPY --from=builder /app/bin/anagram_chain_baremetal.elf ./bin/

# Run in QEMU lm3s6965evb machine
# -nographic: no GUI, serial output to stdout
# Use Ctrl+A X to exit QEMU
ENTRYPOINT ["qemu-system-arm", "-M", "lm3s6965evb", "-nographic", "-kernel", "./bin/anagram_chain_baremetal.elf"]

# ==============================================================================
# Stage 4: ARM FreeRTOS with QEMU System Emulation
# ==============================================================================
FROM ubuntu:22.04 AS freertos

# Install QEMU system emulation for ARM
RUN apt-get update && apt-get install -y \
    qemu-system-arm \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy FreeRTOS ELF binary from builder
COPY --from=builder /app/bin/anagram_chain_freertos.elf ./bin/

# Run in QEMU lm3s6965evb machine
# -nographic: no GUI, serial output to stdout
# Use Ctrl+A X to exit QEMU
ENTRYPOINT ["qemu-system-arm", "-M", "lm3s6965evb", "-nographic", "-kernel", "./bin/anagram_chain_freertos.elf"]

# ==============================================================================
# Stage 5: ARM Bare-metal Tests with QEMU
# ==============================================================================
FROM ubuntu:22.04 AS test-baremetal

RUN apt-get update && apt-get install -y \
    qemu-system-arm \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/bin/test_baremetal.elf ./bin/

ENTRYPOINT ["qemu-system-arm", "-M", "lm3s6965evb", "-nographic", "-kernel", "./bin/test_baremetal.elf"]

# ==============================================================================
# Stage 6: ARM FreeRTOS Tests with QEMU
# ==============================================================================
FROM ubuntu:22.04 AS test-freertos

RUN apt-get update && apt-get install -y \
    qemu-system-arm \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/bin/test_freertos.elf ./bin/

ENTRYPOINT ["qemu-system-arm", "-M", "lm3s6965evb", "-nographic", "-kernel", "./bin/test_freertos.elf"]

# ==============================================================================
# Stage 7: Lint/Static Analysis
# ==============================================================================
FROM builder AS lint

WORKDIR /app

# Run static analysis
CMD ["make", "lint"]
