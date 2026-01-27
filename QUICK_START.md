# Quick Start Guide

Get the Embedded Anagram Chain Demo running in under 1 minute.

## Prerequisites

Choose ONE of the following:
- **Option A:** Docker installed
- **Option B:** GCC and Make installed (Linux)

---

## Option A: Docker (Easiest)

```bash
# 1. Build Docker image
docker build -t anagram-chain --target runtime -f docker/Dockerfile .

# 2. Run with example dictionary
docker run --rm anagram-chain tests/data/example.txt abck
```

---

## Option B: Native Build

```bash
# 1. Run setup script (installs dependencies and builds)
./install.sh

# 2. Run with example dictionary
./bin/anagram_chain tests/data/example.txt abck
```

Or manually:

```bash
# 1. Install build tools (Ubuntu/Debian)
sudo apt-get install build-essential

# 2. Build
make

# 3. Run
./bin/anagram_chain tests/data/example.txt abck
```

---

## Expected Output

```
Loading dictionary: tests/data/example.txt
Dictionary loaded: 0.019 ms
Words loaded: 12

Building index...
Index built: 0.027 ms
Unique signatures: 12

Searching for longest chains starting from 'abck'...
Search completed: 0.102 ms

Found 1 chain(s) of length 4:
abck->abcek->abcelk->baclekt

Total execution time: Total: 0.187 ms
```

---

## Run Tests

```bash
make test
```

## Code Quality

```bash
# Run static analysis (clang-format, cppcheck, clang-tidy)
make lint

# Verify all 9 build configurations compile and tests pass
make check
```

## Implementation Selection

The project has two implementations with different memory modes:

```bash
# AI implementation (default, dynamic memory)
make IMPL=ai test

# Human implementation (static memory, for embedded)
make IMPL=human test

# Human implementation (dynamic memory, optimized)
make IMPL=human MEM=dynamic test
```

**Build commands:**
- `make` or `make IMPL=ai` → AI implementation (dynamic memory)
- `make IMPL=human` → Human implementation (static memory)
- `make IMPL=human MEM=dynamic` → Human implementation (dynamic memory)
- `make build-all` → All three for benchmarking

## Benchmarking

See [BENCHMARK.md](BENCHMARK.md) for detailed guide.

```bash
make generate-stress
make benchmark ARGS='tests/data/stress.txt fu 3'
```

---

## Next Steps

- See [README.md](README.md) for full documentation
- See [BENCHMARK.md](BENCHMARK.md) for benchmark guide
- See [docs/algorithm.md](docs/algorithm.md) for algorithm details
- Try with your own dictionary file!
