# Embedded Anagram Chain Demo

[![CI](https://github.com/vanaluk/c_embedded_anagram_chain_demo/actions/workflows/ci.yml/badge.svg)](https://github.com/vanaluk/c_embedded_anagram_chain_demo/actions/workflows/ci.yml)

## Introduction

Hi!

I understand that a simple solution algorithm was expected, but in this project I wanted to demonstrate not just the solution to the Anagram Chain problem but also my skills in the embedded environment. So, I created a project in which:
1. **Cross-compilation** (both locally and via Docker) was implemented for **PC**, **ARM bare-metal** and **ARM FreeRTOS**;
2. **Debugging** was configured in **VS Code** for **PC**, **ARM bare-metal** and **ARM FreeRTOS** using **QEMU** and **Docker**;
3. **Github Action CI/CD** was configured for all build types, running **unit tests** and various **static analyzers**, as well as checking **code formatting**;
4. **AI agent** integration for **code review**;
5. **Clangd** was configured for easier coding;
6. Scripts for automatic deployment of the development environment (**install.sh**) were implemented;
7. Various solution options have been implemented: an **AI-generated solution**, **my solution** (supports both **dynamic memory** allocation and **static memory** allocation, which is more preferable for embedded development).
8. **Git** development and the **branch-based approach**;
9. A **benchmark**, which shows that the AI ​​solution is still not perfect and that a human with a good understanding of the context of the problem can write a better one because:
    - **AI** does not allocate memory efficiently;
    - **AI** selects a sorting algorithm that does not take into account the specific type of data (AI selects quick sort when counting sort can be used for ASCII);
    - **AI** may re-sort an already sorted string;
    - **AI** selects a hash function without considering the nature of the data;
    - **AI** uses an unnecessary visited array;

## What is a Derived Anagram?

A **derived anagram** is a word formed by adding exactly one letter to another word and rearranging all letters. For example:

```
sail → nails (added 'n', rearranged to "nails")
nails → aliens (added 'e', rearranged to "aliens")
```

The program finds the **longest chain** of such derived anagrams starting from a given word.

## The Algorithm

See [ALGORITHM.md](ALGORITHM.md)

## Quick Start

### Option 1: Using install.sh (Recommended)

```bash
# Clone with submodules
git clone --recursive https://github.com/vanaluk/c_embedded_anagram_chain_demo.git
cd c_embedded_anagram_chain_demo

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
git clone https://github.com/vanaluk/c_embedded_anagram_chain_demo.git
cd c_embedded_anagram_chain_demo

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
docker build -t anagram-chain --target runtime -f docker/Dockerfile .
docker run --rm anagram-chain --help

# Build and run ARM bare-metal version (QEMU)
docker build -t anagram-chain-baremetal --target baremetal -f docker/Dockerfile .
docker run --rm anagram-chain-baremetal

# Build and run ARM FreeRTOS version (QEMU)
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
Dictionary loaded: 0.019 ms
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
| Docker (Bare-metal) | `make docker-build-baremetal` | Docker image |
| Docker (FreeRTOS) | `make docker-build-freertos` | Docker image |

### Implementation Selection

The project has two implementations with different memory modes:

| Implementation | Memory Mode | Description |
|----------------|-------------|-------------|
| `ai` | Dynamic only | AI-generated, uses malloc/free per word |
| `human` | Static (default) | Pre-allocated pools, no malloc at runtime |
| `human` | Dynamic | Optimized bulk allocation, fewer malloc calls |

**Note:** AI implementation only supports dynamic memory allocation.

**Build commands:**

| Command | Output Binary | Description |
|---------|---------------|-------------|
| `make` | `bin/anagram_chain` | Default build (AI impl, dynamic) |
| `make IMPL=ai` | `bin/anagram_chain` | Same as default |
| `make IMPL=human` | `bin/anagram_chain` | Human impl, static memory |
| `make IMPL=human MEM=static` | `bin/anagram_chain` | Human impl, static (explicit) |
| `make IMPL=human MEM=dynamic` | `bin/anagram_chain` | Human impl, dynamic memory |
| `make build-all` | 3 binaries | All implementations for benchmarking |

```bash
# Build with AI implementation (default, dynamic memory)
make

# Build with human implementation (static memory, for embedded)
make IMPL=human

# Build with human implementation (dynamic memory, optimized)
make IMPL=human MEM=dynamic

# Test with human implementation
make IMPL=human test

# Build all three for benchmarking
make build-all
```

### Benchmarking

See [BENCHMARK.md](BENCHMARK.md) for detailed benchmark guide.

```bash
# Quick start - compare all 3 implementations
make generate-stress
make benchmark ARGS='tests/data/stress.txt fu 3'

# Run single implementation
make IMPL=human stress                  # static memory
make IMPL=human MEM=dynamic stress      # dynamic memory
```

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
  Embedded Anagram Chain Demo - Unit Tests
======================================

Running unit tests...

Signature Tests:
  [PASS] compute_signature
  [PASS] is_derived_signature

Validation Tests:
  [PASS] is_valid_word

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

## Project Structure

```
.
├── src/
│   ├── include/                    # Public headers
│   │   └── anagram_chain.h         # API definitions
│   ├── impl/                       # Implementation files
│   │   ├── ai/                     # AI-generated implementation
│   │   │   └── anagram_chain.c     # Dynamic memory only
│   │   └── human/                  # Human-optimized implementation
│   │       ├── anagram_chain_core.c  # Core algorithm (DFS, hash table)
│   │       ├── anagram_chain_io.c    # I/O (timer, validation, output)
│   │       ├── assert.h            # Custom assert macros
│   │       ├── config.h            # Memory pool configuration
│   │       ├── global.h            # Common includes and macros
│   │       └── trace.h             # Debug tracing support
│   └── main/                       # Entry points
│       ├── main_pc.c               # PC main
│       ├── main_arm.c              # ARM bare-metal main
│       └── main_freertos.c         # FreeRTOS main
├── arm/                            # ARM support files
│   ├── startup.s                   # Cortex-M3 startup code
│   ├── linker.ld                   # Linker script
│   ├── uart.c/h                    # UART driver
│   ├── syscalls.c                  # Newlib syscalls
│   └── freertos/                   # FreeRTOS configuration
│       ├── FreeRTOSConfig.h        # FreeRTOS settings
│       └── FreeRTOS-Kernel/        # FreeRTOS (git submodule)
├── tests/                          # Unit tests
│   ├── test_runner.h               # Test framework
│   ├── test_core.c                 # Test implementations
│   ├── test_main_pc.c              # PC test runner
│   ├── test_main_arm.c             # ARM test runner
│   ├── test_main_freertos.c        # FreeRTOS test runner
│   └── data/                       # Test dictionaries
├── docker/                         # Docker configuration
│   ├── Dockerfile                  # Multi-stage build
│   └── docker-compose.yml
├── .github/workflows/              # CI/CD
│   └── ci.yml                      # GitHub Actions
├── .vscode/                        # IDE configuration
│   ├── tasks.json                  # Build tasks (9 configurations)
│   └── launch.json                 # Debug configurations
├── Makefile                        # Build configuration
├── install.sh                      # Setup script
├── benchmark.py                    # Performance comparison script
├── ALGORITHM.md                    # Algorithm description
├── OPTIMIZATION.md                 # Optimization details
├── BENCHMARK.md                    # Benchmarking guide
├── QUICK_START.md                  # Quick start guide
├── .clang-format                   # Code formatting
├── .clangd                         # Language server config
└── README.md                       # This file
```

## Git Submodules

This project uses git submodules for external dependencies:

| Submodule | Path | Description |
|-----------|------|-------------|
| FreeRTOS-Kernel | `arm/freertos/FreeRTOS-Kernel` | FreeRTOS kernel for ARM |

### Cloning with Submodules

```bash
# Clone with all submodules
git clone --recursive https://github.com/vanaluk/c_embedded_anagram_chain_demo.git

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

### Code Quality

```bash
# Format all source files
make format

# Run static analysis (clang-format, cppcheck, clang-tidy)
make lint

# Verify all 9 build configurations compile and tests pass
make check
```

The `check` target builds and tests all combinations:
- AI × (PC, ARM bare-metal, ARM FreeRTOS) = 3
- Human static × (PC, ARM bare-metal, ARM FreeRTOS) = 3
- Human dynamic × (PC, ARM bare-metal, ARM FreeRTOS) = 3

## Tools Used

- **Language:** C (C11 standard)
- **Build System:** GNU Make
- **Containerization:** Docker + QEMU
- **Cross-Compilation:** arm-none-eabi-gcc
- **RTOS:** FreeRTOS V11.1.0
- **Emulation:** QEMU (lm3s6965evb)
- **CI/CD:** GitHub Actions
- **Code Quality:** clang-format, cppcheck, clang-tidy, clangd

## License

MIT License - see LICENSE file for details.

## Author

**Ivan Lukianenko**

- 📧 Email: [vanaluk@gmail.com](mailto:vanaluk@gmail.com)
- 💼 LinkedIn: [linkedin.com/in/ivan-lukianenko-31502894](https://www.linkedin.com/in/ivan-lukianenko-31502894/)
