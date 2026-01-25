#!/usr/bin/env python3
"""
Generate stress test dictionary with long anagram chains.

Creates a dictionary optimized for stress testing the anagram chain finder.
The dictionary contains words that form long chains of derived anagrams.

Usage:
    python generate_stress_dict.py [output_file] [chain_count] [max_chain_length]

Example:
    python generate_stress_dict.py stress.txt 1000 15
"""

import sys
import random
import string
from itertools import permutations


def generate_chain(start_word: str, max_length: int) -> list[str]:
    """Generate a chain of derived anagrams starting from a word."""
    chain = [start_word]
    current = start_word
    
    # Use printable ASCII chars (33-126), excluding some problematic ones
    chars = string.ascii_lowercase + string.digits
    
    for _ in range(max_length - 1):
        # Add a random character
        new_char = random.choice(chars)
        new_word = current + new_char
        
        # Shuffle to create anagram
        word_list = list(new_word)
        random.shuffle(word_list)
        new_word = ''.join(word_list)
        
        chain.append(new_word)
        current = new_word
    
    return chain


def generate_branching_chains(base: str, branch_count: int, branch_length: int) -> list[str]:
    """Generate multiple chains branching from a base word."""
    words = [base]
    
    for _ in range(branch_count):
        chain = generate_chain(base, branch_length)
        words.extend(chain[1:])  # Skip the base word (already added)
    
    return words


def generate_stress_dictionary(chain_count: int = 500, 
                                max_chain_length: int = 12,
                                branch_factor: int = 3) -> set[str]:
    """
    Generate a dictionary with many anagram chains.
    
    Args:
        chain_count: Number of independent chain groups
        max_chain_length: Maximum length of each chain
        branch_factor: Number of branches from some words
    
    Returns:
        Set of unique words
    """
    words = set()
    
    # Generate base words of different lengths (2-4 chars)
    base_words = []
    for length in range(2, 5):
        for _ in range(chain_count // 3):
            base = ''.join(random.choices(string.ascii_lowercase, k=length))
            base_words.append(base)
    
    print(f"Generating {len(base_words)} chain groups...", file=sys.stderr)
    
    for i, base in enumerate(base_words):
        # Generate main chain
        chain = generate_chain(base, max_chain_length)
        words.update(chain)
        
        # Add branches from middle of chain
        for j in range(1, len(chain) - 1, 2):
            if random.random() < 0.3:  # 30% chance of branching
                branch_chains = generate_branching_chains(
                    chain[j], branch_factor, max_chain_length - j
                )
                words.update(branch_chains)
        
        if (i + 1) % 100 == 0:
            print(f"  Generated {i + 1}/{len(base_words)} groups ({len(words)} words)", 
                  file=sys.stderr)
    
    # Add some random words to increase dictionary size
    extra_count = len(words) // 2
    print(f"Adding {extra_count} random words...", file=sys.stderr)
    
    for _ in range(extra_count):
        length = random.randint(3, 12)
        word = ''.join(random.choices(string.ascii_lowercase + string.digits, k=length))
        words.add(word)
    
    return words


def find_good_start_word(words: set[str]) -> str:
    """Find a short word that likely starts a long chain."""
    # Sort by length, prefer shorter words
    sorted_words = sorted(words, key=len)
    
    # Return one of the shortest words
    short_words = [w for w in sorted_words if len(w) <= 3]
    if short_words:
        return random.choice(short_words[:10])
    return sorted_words[0]


def main():
    output_file = sys.argv[1] if len(sys.argv) > 1 else "stress.txt"
    chain_count = int(sys.argv[2]) if len(sys.argv) > 2 else 500
    max_chain_length = int(sys.argv[3]) if len(sys.argv) > 3 else 12
    
    print(f"Generating stress dictionary:", file=sys.stderr)
    print(f"  Chain count: {chain_count}", file=sys.stderr)
    print(f"  Max chain length: {max_chain_length}", file=sys.stderr)
    
    words = generate_stress_dictionary(chain_count, max_chain_length)
    
    # Find a good starting word
    start_word = find_good_start_word(words)
    
    # Write dictionary
    with open(output_file, 'w') as f:
        for word in sorted(words):
            f.write(word + '\n')
    
    print(f"\nGenerated {len(words)} words to {output_file}", file=sys.stderr)
    print(f"Suggested start word: {start_word}", file=sys.stderr)
    print(f"\nUsage:", file=sys.stderr)
    print(f"  ./bin/anagram_chain {output_file} {start_word}", file=sys.stderr)


if __name__ == "__main__":
    main()
