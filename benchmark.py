#!/usr/bin/env python3
"""
Benchmark script to compare AI and Human implementations.

Builds both implementations and runs them on the same dictionary,
comparing execution time and results.

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


def build_implementation(impl: str) -> bool:
    """Build an implementation and return success status."""
    print(f"Building {impl} implementation...")
    
    dst = Path(f"bin/anagram_chain_{impl}")
    
    # Build (don't clean - we want to keep both binaries)
    result = subprocess.run(
        ["make", f"IMPL={impl}"],
        capture_output=True,
        text=True
    )
    
    if result.returncode != 0:
        print(f"  ERROR: Build failed for {impl}")
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
    """Run benchmark comparing AI and Human implementations."""
    
    print("=" * 60)
    print("ANAGRAM CHAIN BENCHMARK")
    print("=" * 60)
    print(f"Dictionary: {dictionary}")
    print(f"Start word: {start_word}")
    print(f"Runs: {runs}")
    print()
    
    # Build both implementations
    print("-" * 60)
    print("BUILDING")
    print("-" * 60)
    
    # Clean first
    subprocess.run(["make", "clean"], capture_output=True)
    
    ai_built = build_implementation("ai")
    human_built = build_implementation("human")
    
    if not ai_built and not human_built:
        print("ERROR: No implementations built successfully")
        return 1
    
    print()
    
    # Run benchmarks
    results = {}
    
    for impl in ["ai", "human"]:
        binary = Path(f"bin/anagram_chain_{impl}")
        
        if not binary.exists():
            print(f"Skipping {impl}: binary not found")
            continue
        
        print("-" * 60)
        print(f"BENCHMARKING: {impl.upper()}")
        print("-" * 60)
        
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
            
            results[impl] = {
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
    print("=" * 60)
    print("RESULTS")
    print("=" * 60)
    
    if not results:
        print("No results to compare")
        return 1
    
    # Table header
    print(f"{'Metric':<25} {'AI':>15} {'Human':>15} {'Diff':>15}")
    print("-" * 70)
    
    ai = results.get("ai", {})
    human = results.get("human", {})
    
    def format_row(label, ai_val, human_val, unit=""):
        if ai_val is not None and human_val is not None:
            if ai_val > 0:
                diff = ((human_val - ai_val) / ai_val) * 100
                diff_str = f"{diff:+.1f}%"
            else:
                diff_str = "N/A"
            print(f"{label:<25} {ai_val:>12.2f}{unit:>3} {human_val:>12.2f}{unit:>3} {diff_str:>15}")
        elif ai_val is not None:
            print(f"{label:<25} {ai_val:>12.2f}{unit:>3} {'N/A':>15} {'N/A':>15}")
        elif human_val is not None:
            print(f"{label:<25} {'N/A':>15} {human_val:>12.2f}{unit:>3} {'N/A':>15}")
    
    format_row("Avg time", ai.get("avg"), human.get("avg"), "ms")
    format_row("Min time", ai.get("min"), human.get("min"), "ms")
    format_row("Max time", ai.get("max"), human.get("max"), "ms")
    format_row("Std dev", ai.get("stddev"), human.get("stddev"), "ms")
    
    print("-" * 70)
    
    ai_chains = ai.get("chain_count", 0)
    human_chains = human.get("chain_count", 0)
    ai_len = ai.get("chain_length", 0)
    human_len = human.get("chain_length", 0)
    
    print(f"{'Chain count':<25} {ai_chains:>15} {human_chains:>15} {'MATCH' if ai_chains == human_chains else 'DIFFER':>15}")
    print(f"{'Chain length':<25} {ai_len:>15} {human_len:>15} {'MATCH' if ai_len == human_len else 'DIFFER':>15}")
    
    print()
    
    # Verdict
    if ai.get("avg") and human.get("avg"):
        speedup = ai["avg"] / human["avg"] if human["avg"] > 0 else float('inf')
        if speedup > 1.1:
            print(f"Human is {speedup:.2f}x FASTER than AI")
        elif speedup < 0.9:
            print(f"AI is {1/speedup:.2f}x FASTER than Human")
        else:
            print("Both implementations have similar performance")
    
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
