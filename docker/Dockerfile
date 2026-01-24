# Multi-stage Dockerfile for Anagram Chain Finder
#
# Stages:
#   builder     - Build environment with compilers
#   runtime     - Minimal runtime for native binary
#   arm-runtime - Runtime with QEMU for ARM binary

# ==============================================================================
# Stage 1: Builder
# ==============================================================================
FROM ubuntu:22.04 AS builder

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install build tools
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-arm-linux-gnueabihf \
    make \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source files
COPY src/ ./src/
COPY tests/ ./tests/
COPY Makefile ./

# Build native binary
RUN make clean 2>/dev/null || true && make

# Build ARM binary
RUN make arm

# ==============================================================================
# Stage 2: Native Runtime
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
# Stage 3: ARM Runtime with QEMU
# ==============================================================================
FROM ubuntu:22.04 AS arm-runtime

# Install QEMU user-mode emulation
RUN apt-get update && apt-get install -y \
    qemu-user \
    libc6-arm64-cross \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy ARM binary and test data from builder
COPY --from=builder /app/bin/anagram_chain_arm ./bin/
COPY --from=builder /app/tests/data/ ./tests/data/

# Note: ARM binary is statically linked, no need for ARM libraries
ENTRYPOINT ["qemu-arm", "./bin/anagram_chain_arm"]
CMD ["--help"]
