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

## Implementation Selection

The project has two implementations:

```bash
# AI implementation (default, fully working)
make IMPL=ai test

# Human implementation (TODO stubs for manual implementation)
make IMPL=human test
```

**Binary naming:**
- `make` or `make IMPL=ai` → `bin/anagram_chain` (AI implementation)
- `make IMPL=human` → `bin/anagram_chain` (Human implementation, same name)
- `make IMPL=both` → `bin/anagram_chain_ai` + `bin/anagram_chain_human` (both for benchmarking)

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
