# Benchmark Guide

Compare AI and Human implementations performance.

## Implementations

| Implementation | Memory Mode | Description |
|----------------|-------------|-------------|
| `ai` | Dynamic only | AI-generated, uses malloc/free per word |
| `human-static` | Static | Pre-allocated pools, no malloc at runtime |
| `human-dynamic` | Dynamic | Optimized bulk allocation, fewer malloc calls |

**Note:** AI implementation only supports dynamic memory allocation.

## Quick Start

```bash
# 1. Generate stress dictionary
make generate-stress

# 2. Compare all three implementations
make benchmark ARGS='tests/data/stress.txt fu 3'
```

## Build Individual Implementations

```bash
# AI (always dynamic memory)
make IMPL=ai

# Human with static memory (default)
make IMPL=human
make IMPL=human MEM=static

# Human with dynamic memory
make IMPL=human MEM=dynamic

# Build all three for comparison
make build-all
```

## Run Single Implementation

```bash
# AI implementation
make IMPL=ai stress

# Human static implementation
make IMPL=human stress

# Human dynamic implementation
make IMPL=human MEM=dynamic stress

# With custom parameters
make IMPL=human stress STRESS_WORD=abc STRESS_RUNS=5
```

## Generate Dictionary

```bash
# Default (~400k words, ~15-20 sec execution)
make generate-stress

# Small (~20k words, ~0.5 sec)
make generate-stress STRESS_CHAINS=500 STRESS_LENGTH=10

# Large (~1M words, ~60+ sec)
make generate-stress STRESS_CHAINS=10000 STRESS_LENGTH=20
```

| Parameter | Default | Description |
|-----------|---------|-------------|
| `STRESS_CHAINS` | 5000 | Number of chain groups |
| `STRESS_LENGTH` | 15 | Max chain length |
| `STRESS_FILE` | `tests/data/stress.txt` | Output file |

## Run Stress Test

```bash
make IMPL=<ai|human> [MEM=<static|dynamic>] stress
```

| Parameter | Default | Description |
|-----------|---------|-------------|
| `STRESS_FILE` | `tests/data/stress.txt` | Dictionary file |
| `STRESS_WORD` | `fu` | Start word |
| `STRESS_RUNS` | `3` | Number of runs |

## Compare All Implementations

```bash
# Using make (recommended)
make benchmark ARGS='tests/data/stress.txt fu 3'

# Using Python directly
python3 benchmark.py tests/data/stress.txt fu 3
```

Example output:
```
================================================================================
ANAGRAM CHAIN BENCHMARK
================================================================================
Dictionary: tests/data/stress.txt
Start word: fu
Runs: 3

--------------------------------------------------------------------------------
BUILDING
--------------------------------------------------------------------------------
Building ai implementation...
  Built: bin/anagram_chain_ai
Building human-static implementation...
  Built: bin/anagram_chain_human-static
Building human-dynamic implementation...
  Built: bin/anagram_chain_human-dynamic

--------------------------------------------------------------------------------
BENCHMARKING: AI (dynamic)
--------------------------------------------------------------------------------
  Run 1: 1542.31 ms
  Run 2: 1498.76 ms
  Run 3: 1521.05 ms

--------------------------------------------------------------------------------
BENCHMARKING: Human (static)
--------------------------------------------------------------------------------
  Run 1: 1083.45 ms
  Run 2: 1102.33 ms
  Run 3: 1091.28 ms

--------------------------------------------------------------------------------
BENCHMARKING: Human (dynamic)
--------------------------------------------------------------------------------
  Run 1: 412.56 ms
  Run 2: 398.12 ms
  Run 3: 405.89 ms

================================================================================
RESULTS
================================================================================
Metric                 AI (dynamic)     Human (static)    Human (dynamic)
--------------------------------------------------------------------------------
Avg time                   1520.71 ms        1092.35 ms         405.52 ms
Min time                   1498.76 ms        1083.45 ms         398.12 ms
Max time                   1542.31 ms        1102.33 ms         412.56 ms
Std dev                      21.83 ms           9.47 ms           7.23 ms
--------------------------------------------------------------------------------
Chain count                     777               777               777
Chain length                     17                17                17

All implementations produce MATCHING results

--------------------------------------------------------------------------------
SPEEDUP vs AI
--------------------------------------------------------------------------------
  Human (static): 1.39x FASTER than AI
  Human (dynamic): 3.75x FASTER than AI

FASTEST: Human (dynamic) (405.52 ms)
SLOWEST: AI (dynamic) (1520.71 ms)
Overall speedup: 3.75x
```

## Memory Mode Comparison

| Mode | Pros | Cons | Best For |
|------|------|------|----------|
| Static | Predictable memory, no fragmentation, fast | Fixed limits, large binary | Embedded, bare-metal |
| Dynamic (AI) | Flexible, no limits | Many malloc calls, slow | Reference implementation |
| Dynamic (Human) | Flexible, bulk alloc, fast | Requires heap | PC, FreeRTOS with heap |

## Scripts

| Script | Usage |
|--------|-------|
| `tests/data/generate_stress_dict.py` | `python3 <script> <output> <chains> <length>` |
| `benchmark.py` | `python3 benchmark.py <dictionary> <start_word> <runs>` |

## Workflow Example

```bash
# 1. Generate dictionary
make generate-stress STRESS_CHAINS=500 STRESS_LENGTH=10

# 2. Test human implementation
make IMPL=human test

# 3. Run stress test for human static
make IMPL=human stress STRESS_WORD=yg

# 4. Run stress test for human dynamic
make IMPL=human MEM=dynamic stress STRESS_WORD=yg

# 5. Compare all implementations
make benchmark ARGS='tests/data/stress.txt yg 3'
```
