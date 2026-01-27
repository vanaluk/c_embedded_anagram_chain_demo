#!/usr/bin/env python3
"""
Benchmark script to compare AI and Human implementations.

Builds all implementations and runs them on the same dictionary,
comparing execution time and results.

Implementations:
  - ai: AI-generated, always uses dynamic memory
  - human-static: Human implementation with static memory pools
  - human-dynamic: Human implementation with optimized bulk allocation

Usage:
    python benchmark.py [dictionary] [start_word] [runs]

Example:
    python benchmark.py tests/data/stress.txt abc 5
"""

import subprocess
import sys
import os
import time
import statistics
from pathlib import Path


IMPLEMENTATIONS = [
    {"name": "ai", "impl": "ai", "mem": "dynamic", "label": "AI (dynamic)"},
    {"name": "human-static", "impl": "human", "mem": "static", "label": "Human (static)"},
    {"name": "human-dynamic", "impl": "human", "mem": "dynamic", "label": "Human (dynamic)"},
]


def run_command(cmd: list[str], timeout: int = 300) -> tuple[int, str, float]:
    """Run a command and return exit code, output, and elapsed time."""
    start = time.perf_counter()
    try:
        result = subprocess.run(
            cmd, 
            capture_output=True, 
            text=True, 
            timeout=timeout
        )
        elapsed = time.perf_counter() - start
        output = result.stdout + result.stderr
        return result.returncode, output, elapsed
    except subprocess.TimeoutExpired:
        return -1, "TIMEOUT", timeout


def build_implementation(impl_config: dict) -> bool:
    """Build an implementation and return success status."""
    name = impl_config["name"]
    impl = impl_config["impl"]
    mem = impl_config["mem"]
    
    print(f"Building {name} implementation...")
    
    dst = Path(f"bin/anagram_chain_{name}")
    
    # Build with proper flags
    result = subprocess.run(
        ["make", f"IMPL={impl}", f"MEM={mem}"],
        capture_output=True,
        text=True
    )
    
    if result.returncode != 0:
        print(f"  ERROR: Build failed for {name}")
        print(result.stderr)
        return False
    
    # Rename binary
    src = Path("bin/anagram_chain")
    
    if src.exists():
        if dst.exists():
            dst.unlink()
        src.rename(dst)
        print(f"  Built: {dst}")
        return True
    else:
        print(f"  ERROR: Binary not found: {src}")
        return False


def parse_timing(output: str) -> dict[str, float]:
    """Parse timing information from program output."""
    timings = {}
    
    for line in output.split('\n'):
        if ':' in line and ('ms' in line or ' s' in line):
            parts = line.split(':')
            if len(parts) >= 2:
                label = parts[0].strip()
                value_str = parts[1].strip()
                
                try:
                    if 'ms' in value_str:
                        value = float(value_str.replace('ms', '').strip())
                    elif ' s' in value_str:
                        value = float(value_str.replace('s', '').strip()) * 1000
                    else:
                        continue
                    timings[label] = value
                except ValueError:
                    pass
    
    return timings


def parse_result(output: str) -> tuple[int, int]:
    """Parse chain count and length from output."""
    chain_count = 0
    chain_length = 0
    
    for line in output.split('\n'):
        if 'Found' in line and 'chain(s)' in line:
            # "Found 1 chain(s) of length 4:"
            parts = line.split()
            for i, part in enumerate(parts):
                if part == 'Found' and i + 1 < len(parts):
                    try:
                        chain_count = int(parts[i + 1])
                    except ValueError:
                        pass
                if part == 'length' and i + 1 < len(parts):
                    try:
                        chain_length = int(parts[i + 1].rstrip(':'))
                    except ValueError:
                        pass
    
    return chain_count, chain_length


def run_benchmark(dictionary: str, start_word: str, runs: int = 5):
    """Run benchmark comparing all implementations."""
    
    print("=" * 80)
    print("ANAGRAM CHAIN BENCHMARK")
    print("=" * 80)
    print(f"Dictionary: {dictionary}")
    print(f"Start word: {start_word}")
    print(f"Runs: {runs}")
    print()
    
    # Build all implementations
    print("-" * 80)
    print("BUILDING")
    print("-" * 80)
    
    # Clean first
    subprocess.run(["make", "clean"], capture_output=True)
    
    built = {}
    for impl_config in IMPLEMENTATIONS:
        built[impl_config["name"]] = build_implementation(impl_config)
    
    if not any(built.values()):
        print("ERROR: No implementations built successfully")
        return 1
    
    print()
    
    # Run benchmarks
    results = {}
    
    for impl_config in IMPLEMENTATIONS:
        name = impl_config["name"]
        label = impl_config["label"]
        binary = Path(f"bin/anagram_chain_{name}")
        
        if not binary.exists():
            print(f"Skipping {name}: binary not found")
            continue
        
        print("-" * 80)
        print(f"BENCHMARKING: {label}")
        print("-" * 80)
        
        times = []
        last_output = ""
        
        for i in range(runs):
            cmd = [str(binary), dictionary, start_word]
            code, output, elapsed = run_command(cmd)
            
            if code == 0:
                times.append(elapsed * 1000)  # Convert to ms
                last_output = output
                print(f"  Run {i+1}: {elapsed*1000:.2f} ms")
            elif code == -1:
                print(f"  Run {i+1}: TIMEOUT")
            else:
                print(f"  Run {i+1}: ERROR (exit code {code})")
                if output:
                    print(f"    {output[:200]}")
        
        if times:
            chain_count, chain_length = parse_result(last_output)
            timings = parse_timing(last_output)
            
            results[name] = {
                "label": label,
                "times": times,
                "min": min(times),
                "max": max(times),
                "avg": statistics.mean(times),
                "stddev": statistics.stdev(times) if len(times) > 1 else 0,
                "chain_count": chain_count,
                "chain_length": chain_length,
                "internal_timings": timings
            }
        
        print()
    
    # Print comparison
    print("=" * 80)
    print("RESULTS")
    print("=" * 80)
    
    if not results:
        print("No results to compare")
        return 1
    
    # Table header with all implementations
    header = f"{'Metric':<20}"
    for impl_config in IMPLEMENTATIONS:
        name = impl_config["name"]
        if name in results:
            header += f" {impl_config['label']:>18}"
    print(header)
    print("-" * 80)
    
    def format_row(label, values, unit=""):
        row = f"{label:<20}"
        for impl_config in IMPLEMENTATIONS:
            name = impl_config["name"]
            if name in results:
                val = values.get(name)
                if val is not None:
                    row += f" {val:>15.2f}{unit:>3}"
                else:
                    row += f" {'N/A':>18}"
        print(row)
    
    # Timing metrics
    format_row("Avg time", {n: r["avg"] for n, r in results.items()}, "ms")
    format_row("Min time", {n: r["min"] for n, r in results.items()}, "ms")
    format_row("Max time", {n: r["max"] for n, r in results.items()}, "ms")
    format_row("Std dev", {n: r["stddev"] for n, r in results.items()}, "ms")
    
    print("-" * 80)
    
    # Chain results
    chain_counts = {n: r["chain_count"] for n, r in results.items()}
    chain_lengths = {n: r["chain_length"] for n, r in results.items()}
    
    row = f"{'Chain count':<20}"
    for impl_config in IMPLEMENTATIONS:
        name = impl_config["name"]
        if name in results:
            row += f" {chain_counts[name]:>18}"
    print(row)
    
    row = f"{'Chain length':<20}"
    for impl_config in IMPLEMENTATIONS:
        name = impl_config["name"]
        if name in results:
            row += f" {chain_lengths[name]:>18}"
    print(row)
    
    # Verify results match
    all_counts = list(chain_counts.values())
    all_lengths = list(chain_lengths.values())
    if len(set(all_counts)) == 1 and len(set(all_lengths)) == 1:
        print("\nAll implementations produce MATCHING results")
    else:
        print("\nWARNING: Results DIFFER between implementations!")
    
    print()
    
    # Speedup comparison (relative to AI)
    if "ai" in results:
        ai_avg = results["ai"]["avg"]
        print("-" * 80)
        print("SPEEDUP vs AI")
        print("-" * 80)
        for impl_config in IMPLEMENTATIONS:
            name = impl_config["name"]
            if name in results and name != "ai":
                other_avg = results[name]["avg"]
                if other_avg > 0:
                    speedup = ai_avg / other_avg
                    if speedup > 1.0:
                        print(f"  {impl_config['label']}: {speedup:.2f}x FASTER than AI")
                    else:
                        print(f"  {impl_config['label']}: {1/speedup:.2f}x SLOWER than AI")
    
    # Find fastest
    if len(results) > 1:
        fastest = min(results.items(), key=lambda x: x[1]["avg"])
        slowest = max(results.items(), key=lambda x: x[1]["avg"])
        overall_speedup = slowest[1]["avg"] / fastest[1]["avg"]
        print()
        print(f"FASTEST: {results[fastest[0]]['label']} ({fastest[1]['avg']:.2f} ms)")
        print(f"SLOWEST: {results[slowest[0]]['label']} ({slowest[1]['avg']:.2f} ms)")
        print(f"Overall speedup: {overall_speedup:.2f}x")
    
    return 0


def main():
    # Default values
    dictionary = "tests/data/stress.txt"
    start_word = None
    runs = 5
    
    # Parse arguments
    if len(sys.argv) > 1:
        dictionary = sys.argv[1]
    if len(sys.argv) > 2:
        start_word = sys.argv[2]
    if len(sys.argv) > 3:
        runs = int(sys.argv[3])
    
    # Check dictionary exists
    if not Path(dictionary).exists():
        print(f"Dictionary not found: {dictionary}")
        print("Generate one with: python tests/data/generate_stress_dict.py")
        return 1
    
    # If no start word, read first word from dictionary
    if start_word is None:
        with open(dictionary) as f:
            words = [line.strip() for line in f if line.strip()]
        # Find shortest word
        start_word = min(words, key=len) if words else "abc"
        print(f"Using start word: {start_word}")
    
    return run_benchmark(dictionary, start_word, runs)


if __name__ == "__main__":
    sys.exit(main())
