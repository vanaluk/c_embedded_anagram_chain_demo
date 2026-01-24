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
- 🔧 **Cross-Platform** - Builds for x86_64 and ARM architectures
- 🐳 **Docker Support** - Containerized build and execution
- 🧪 **Unit Tests** - Comprehensive tests with time limits
- 📊 **Performance Measurement** - Built-in timing for benchmarking
- 🔄 **CI/CD** - Automated builds with GitHub Actions

## Quick Start

### Option 1: Using install.sh (Recommended)

```bash
./install.sh
./bin/anagram_chain tests/data/example.txt abck
```

### Option 2: Manual Build

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install build-essential

# Build
make

# Run
./bin/anagram_chain tests/data/example.txt abck
```

### Option 3: Docker

```bash
docker build -t anagram-chain -f docker/Dockerfile .
docker run --rm anagram-chain ./bin/anagram_chain /app/tests/data/example.txt abck
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
| Native x86_64 | `make` | `bin/anagram_chain` |
| ARM (cross-compile) | `make arm` | `bin/anagram_chain_arm` |
| Debug (with symbols) | `make debug` | `bin/anagram_chain_debug` |
| Docker | `make docker-build` | Docker image |

### ARM Cross-Compilation

```bash
# Install ARM toolchain
sudo apt-get install gcc-arm-linux-gnueabihf qemu-user

# Build
make arm

# Run with QEMU
qemu-arm -L /usr/arm-linux-gnueabihf ./bin/anagram_chain_arm tests/data/example.txt abck
```

## Testing

```bash
# Run all tests
make test

# Test output
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

### Test Categories

- **Unit Tests** - Function-level testing
- **Integration Tests** - End-to-end chain finding
- **Performance Tests** - Time limit enforcement

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

## Performance

| Dictionary Size | Expected Time |
|-----------------|---------------|
| 1,000 words | < 0.1 s |
| 100,000 words | < 30 s |
| 1,000,000 words | < 5 min |

## Project Structure

```
.
├── src/                    # Source files
│   └── anagram_chain.c     # Main implementation
├── tests/                  # Unit tests
│   ├── test_anagram.c      # Test suite
│   └── data/               # Test dictionaries
├── docker/                 # Docker configuration
│   ├── Dockerfile          # Multi-stage build
│   └── docker-compose.yml
├── docs/                   # Documentation
│   ├── tz.md               # Technical specification
│   ├── architecture.md     # System architecture
│   └── algorithm.md        # Algorithm documentation
├── .github/workflows/      # CI/CD
│   └── ci.yml              # GitHub Actions
├── .vscode/                # IDE configuration
├── Makefile                # Build configuration
├── install.sh              # Setup script
├── .clang-format           # Code formatting
├── .clangd                 # Language server config
└── README.md               # This file
```

## Development

### Prerequisites

- GCC 11+ or Clang 14+
- GNU Make
- Docker (optional)
- QEMU (optional, for ARM testing)

### IDE Setup

The project includes VSCode configuration:

1. Open folder in VSCode
2. Install C/C++ extension
3. Build: `Ctrl+Shift+B` → "Build"
4. Debug: `F5` → "Debug Anagram Chain"

### Code Formatting

```bash
# Format all source files
make format
```

## Tools Used

- **Language:** C (C11 standard)
- **Build System:** GNU Make
- **Containerization:** Docker
- **Cross-Compilation:** arm-linux-gnueabihf-gcc
- **Emulation:** QEMU (user-mode)
- **CI/CD:** GitHub Actions
- **Code Quality:** clang-format, clangd

## License

MIT License - see LICENSE file for details.

## Author

Developed as an Embedded Developer test task - 2026
