# Anagram Chain Finder

[![CI](https://github.com/YOUR_USERNAME/anagram-chain/actions/workflows/ci.yml/badge.svg)](https://github.com/YOUR_USERNAME/anagram-chain/actions/workflows/ci.yml)

A high-performance C application that finds the longest chains of derived anagrams in a dictionary. Developed as a test task demonstrating Embedded Developer skills.

## What is a Derived Anagram?

A **derived anagram** is a word formed by adding exactly one letter to another word and rearranging all letters. For example:

```
sail → nails (added 'n', rearranged to "nails")
nails → aliens (added 'e', rearranged to "aliens")
```

The program finds the **longest chain** of such derived anagrams starting from a given word.

## Features

- ⚡ **Efficient Algorithm** - O(n) dictionary indexing with hash table
- 🔧 **Multi-Platform** - Builds for PC, ARM bare-metal, and ARM FreeRTOS
- 🐳 **Docker Support** - Containerized build and execution with QEMU
- 🧪 **Unit Tests** - Tests run on all platforms (PC, ARM bare-metal, FreeRTOS)
- 📊 **Performance Measurement** - Built-in timing for benchmarking
- 🔄 **CI/CD** - Automated builds and tests with GitHub Actions

## Quick Start

### Option 1: Using install.sh (Recommended)

```bash
# Clone with submodules
git clone --recursive https://github.com/YOUR_USERNAME/anagram-chain.git
cd anagram-chain

# Or if already cloned without --recursive:
git submodule update --init --recursive

# Install dependencies and build
./install.sh
make
./bin/anagram_chain tests/data/example.txt abck
```

### Option 2: Manual Build

```bash
# Clone repository
git clone https://github.com/YOUR_USERNAME/anagram-chain.git
cd anagram-chain

# Initialize submodules (required for FreeRTOS)
git submodule update --init --recursive

# Install dependencies (Ubuntu/Debian)
sudo apt-get install build-essential gcc-arm-none-eabi libnewlib-arm-none-eabi \
    qemu-system-arm gdb-multiarch clang-format

# Build
make

# Run
./bin/anagram_chain tests/data/example.txt abck
```

### Option 3: Docker

```bash
# Build and run PC version
docker build -t anagram-chain -f docker/Dockerfile .
docker run --rm anagram-chain --help

# Build and run ARM FreeRTOS version
docker build -t anagram-chain-freertos --target freertos -f docker/Dockerfile .
docker run --rm anagram-chain-freertos
```

## Usage

```bash
./bin/anagram_chain <dictionary_file> <starting_word>
```

### Arguments

| Argument | Description |
|----------|-------------|
| `dictionary_file` | Path to dictionary file (one word per line) |
| `starting_word` | Word to start the chain from (must be in dictionary) |

### Example Output

```
Loading dictionary: tests/data/example.txt
Dictionary loaded: 0.123 ms
Words loaded: 12

Building index...
Index built: 0.045 ms
Unique signatures: 12

Searching for longest chains starting from 'abck'...
Search completed: 0.089 ms

Found 1 chain(s) of length 4:
abck->abcek->abcelk->baclekt

Total execution time: Total: 0.257 ms
```

## Building

| Target | Command | Output |
|--------|---------|--------|
| Native PC | `make` | `bin/anagram_chain` |
| PC Debug | `make debug` | `bin/anagram_chain_debug` |
| ARM Bare-metal | `make arm-baremetal` | `bin/anagram_chain_baremetal.elf` |
| ARM FreeRTOS | `make arm-freertos` | `bin/anagram_chain_freertos.elf` |
| Docker (PC) | `make docker-build` | Docker image |
| Docker (FreeRTOS) | `make docker-build-freertos` | Docker image |

### ARM Bare-metal Build

```bash
# Install ARM toolchain
sudo apt-get install gcc-arm-none-eabi libnewlib-arm-none-eabi qemu-system-arm

# Build
make arm-baremetal

# Run with QEMU (Ctrl+A X to exit)
make run-baremetal
```

### ARM FreeRTOS Build

Requires FreeRTOS-Kernel submodule:

```bash
# Initialize submodule (if not done already)
git submodule update --init --recursive

# Build
make arm-freertos

# Run with QEMU (Ctrl+A X to exit)
make run-freertos
```

## Testing

Tests run on all three platforms:

```bash
# PC tests
make test

# ARM bare-metal tests (requires QEMU)
make test-arm

# ARM FreeRTOS tests (requires QEMU)
make test-freertos

# All platforms
make test-all
```

### Test Output

```
======================================
  Anagram Chain Finder - Unit Tests
======================================

Running unit tests...

Signature Tests:
  [PASS] compute_signature
  [PASS] is_derived_signature

...

======================================
  All tests passed!
======================================
```

### Docker Tests

```bash
# Build and run ARM bare-metal tests
make docker-build-test-baremetal
make docker-run-test-baremetal

# Build and run FreeRTOS tests
make docker-build-test-freertos
make docker-run-test-freertos
```

## Debugging

### PC Debugging (VSCode)

1. Open folder in VSCode
2. Press `F5` → Select "Debug PC"

### ARM Debugging (VSCode + QEMU)

1. Press `F5` → Select "Debug ARM Bare-metal (QEMU GDB)" or "Debug ARM FreeRTOS (QEMU GDB)"
2. QEMU will start automatically, debugger will connect and stop at `main()`

### ARM Debugging (Manual)

```bash
# Terminal 1: Start QEMU with GDB server
make debug-baremetal   # or make debug-freertos

# Terminal 2: Connect GDB
gdb-multiarch -ex "target remote :1234" bin/anagram_chain_baremetal.elf
```

## Algorithm

See [docs/algorithm.md](docs/algorithm.md) for detailed algorithm documentation.

### Overview

1. **Load Dictionary** - Read words from file, validate ASCII (33-126)
2. **Compute Signatures** - Sort characters of each word (e.g., "sail" → "ails")
3. **Build Index** - Hash table mapping signatures to word indices
4. **DFS Search** - Find all longest chains using depth-first search

### Complexity

| Operation | Time | Space |
|-----------|------|-------|
| Load & Index | O(n × m × log m) | O(n × m) |
| Chain Search | O(chains × depth) | O(depth) |

Where n = word count, m = average word length

## Project Structure

```
.
├── src/
│   ├── include/                # Public headers
│   │   └── anagram_chain.h     # API definitions
│   ├── implementation/         # Implementation files
│   │   └── anagram_chain.c     # Core algorithm
│   └── main/                   # Entry points
│       ├── main_pc.c           # PC main
│       ├── main_arm.c          # ARM bare-metal main
│       └── main_freertos.c     # FreeRTOS main
├── arm/                        # ARM support files
│   ├── startup.s               # Cortex-M3 startup code
│   ├── linker.ld               # Linker script
│   ├── uart.c/h                # UART driver
│   ├── syscalls.c              # Newlib syscalls
│   └── freertos/               # FreeRTOS configuration
│       ├── FreeRTOSConfig.h    # FreeRTOS settings
│       └── FreeRTOS-Kernel/    # FreeRTOS (git submodule)
├── tests/                      # Unit tests
│   ├── test_runner.h           # Test framework
│   ├── test_core.c             # Test implementations
│   ├── test_main_pc.c          # PC test runner
│   ├── test_main_arm.c         # ARM test runner
│   ├── test_main_freertos.c    # FreeRTOS test runner
│   └── data/                   # Test dictionaries
├── docker/                     # Docker configuration
│   ├── Dockerfile              # Multi-stage build
│   └── docker-compose.yml
├── docs/                       # Documentation
├── .github/workflows/          # CI/CD
│   └── ci.yml                  # GitHub Actions
├── .vscode/                    # IDE configuration
├── Makefile                    # Build configuration
├── install.sh                  # Setup script
├── .clang-format               # Code formatting
├── .clangd                     # Language server config
└── README.md                   # This file
```

## Git Submodules

This project uses git submodules for external dependencies:

| Submodule | Path | Description |
|-----------|------|-------------|
| FreeRTOS-Kernel | `arm/freertos/FreeRTOS-Kernel` | FreeRTOS kernel for ARM |

### Cloning with Submodules

```bash
# Clone with all submodules
git clone --recursive https://github.com/YOUR_USERNAME/anagram-chain.git

# Or initialize submodules after clone
git submodule update --init --recursive
```

## Development

### Prerequisites

- GCC 11+ or Clang 14+
- GNU Make
- ARM GCC (`arm-none-eabi-gcc`)
- QEMU (`qemu-system-arm`)
- GDB Multiarch (`gdb-multiarch`)
- Docker (optional)

### IDE Setup (VSCode)

1. Open folder in VSCode
2. Install C/C++ extension
3. Build: `Ctrl+Shift+B` → Select target
4. Debug: `F5` → Select configuration

### Code Formatting

```bash
# Format all source files
make format
```

## Tools Used

- **Language:** C (C11 standard)
- **Build System:** GNU Make
- **Containerization:** Docker + QEMU
- **Cross-Compilation:** arm-none-eabi-gcc
- **RTOS:** FreeRTOS V11.1.0
- **Emulation:** QEMU (lm3s6965evb)
- **CI/CD:** GitHub Actions
- **Code Quality:** clang-format, clangd

## License

MIT License - see LICENSE file for details.

## Author

Developed as an Embedded Developer test task - 2026
