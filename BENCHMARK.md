# Benchmark Guide

Compare AI and Human implementations performance.

## Quick Start

```bash
# 1. Generate stress dictionary
make generate-stress

# 2. Compare both implementations
make benchmark ARGS='tests/data/stress.txt fu 3'
```

## Run Single Implementation

```bash
# AI implementation (default)
make stress

# Human implementation
make IMPL=human stress

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
make IMPL=<ai|human> stress
```

| Parameter | Default | Description |
|-----------|---------|-------------|
| `STRESS_FILE` | `tests/data/stress.txt` | Dictionary file |
| `STRESS_WORD` | `fu` | Start word |
| `STRESS_RUNS` | `3` | Number of runs |

## Compare Implementations

```bash
# Using make
make benchmark ARGS='tests/data/stress.txt fu 3'

# Using Python directly
make IMPL=both
python3 benchmark.py tests/data/stress.txt fu 3
```

Example output:
```
============================================================
ANAGRAM CHAIN BENCHMARK
============================================================
Dictionary: tests/data/stress.txt
Start word: fu
Runs: 3

Metric                       AI           Human            Diff
----------------------------------------------------------------------
Avg time              22819.91 ms     18500.00 ms         -19.0%
Chain count                7346           7346           MATCH
Chain length                 17             17           MATCH

Human is 1.23x FASTER than AI
```

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

# 3. Run stress test for human
make IMPL=human stress STRESS_WORD=yg

# 4. Compare with AI
make benchmark ARGS='tests/data/stress.txt yg 3'
```
