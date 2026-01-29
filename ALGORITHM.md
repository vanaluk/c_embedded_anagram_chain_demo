# Derived Anagram Chain Algorithm

## Problem Definition

**Derived anagram**: word B is a derived anagram of word A if B can be formed by adding exactly one letter to A and rearranging all letters.

**Example chain**: `abc → abck → abcek → abcelk → baclekt` (length 5)

**Input**: Dictionary file + start word  
**Output**: All longest chains starting from the start word

---

## Algorithm Overview

```mermaid
flowchart TD
    subgraph STEP1["STEP 1: DATA PREPARATION"]
        direction TB
        I1[/"Dictionary file (text)"/]
        P1_1["1.1 Read words from file"]
        P1_2["1.2 Compute signature for each word"]
        P1_3["1.3 Build hash index: signature → word list"]
        O1[("WordEntry[] + HashTable")]
        
        I1 --> P1_1 --> P1_2 --> P1_3 --> O1
    end
    
    subgraph STEP2["STEP 2: CHAIN SEARCH (DFS)"]
        direction TB
        I2[/"Start word + Index"/]
        P2_1["2.1 Find start word in dictionary"]
        P2_2["2.2 Recursive DFS: try adding each char (33-126)"]
        P2_3["2.3 Save longest chains found"]
        O2[("ChainResults")]
        
        I2 --> P2_1 --> P2_2 --> P2_3 --> O2
    end
    
    subgraph STEP3["STEP 3: OUTPUT RESULTS"]
        direction TB
        I3[/"Found chains"/]
        P3_1["3.1 Print chain count and max length"]
        P3_2["3.2 Print each chain: word1→word2→word3"]
        O3[/"Console output"/]
        
        I3 --> P3_1 --> P3_2 --> O3
    end
    
    STEP1 --> STEP2 --> STEP3
```

| Step | Input | Process | Output |
|------|-------|---------|--------|
| **STEP 1** | Dictionary file | Load → Signatures → Hash table | `entries[]` + `HashTable` |
| **STEP 2** | Index + start word | DFS search all paths | `ChainResults` |
| **STEP 3** | Found chains | Format and print | Console output |

---

## Data Structures

```mermaid
classDiagram
    class WordEntry {
        +WordBuffer word
        +WordBuffer signature
    }
    
    class DfsContext {
        +ChainPath path
        +WordBuffer candidate
        +WordBuffer temp_sig
    }
    
    class ChainStorage {
        +ChainPath indices
        +size_t length
    }
    
    class MemoryPools {
        +char* word_pool
        +char* sig_pool
        +size_t pool_size
        +size_t word_used
        +size_t sig_used
    }
    
    class StaticHashEntry {
        +WordBuffer signature
        +size_t[] word_indices
        +size_t word_count
        +int next_idx
    }
    
    class GlobalState {
        +WordEntry[] entries
        +size_t word_count
        +StaticHashEntry[] hash_entries
        +int[] hash_buckets
        +StaticChains chains
        +DfsContext dfs
    }
    
    class StaticChains {
        +ChainStorage[] storage
        +Chain[] api_chains
        +ChainResults results
    }
    
    GlobalState *-- WordEntry
    GlobalState *-- DfsContext
    GlobalState *-- StaticChains
    GlobalState *-- StaticHashEntry
    StaticChains *-- ChainStorage
```

---

## STEP 1: Data Preparation

### 1.1 Dictionary Loading

```
entries[0].word  = "abcdg"      entries[0].signature  = "abcdg"
entries[1].word  = "abcd"       entries[1].signature  = "abcd"
..
entries[8].word  = "bafced"     entries[8].signature  = "abcdef"   (sorted)
entries[9].word  = "akjpqwmn"   entries[9].signature  = "ajkmnpqw" (sorted)
entries[10].word = "abcelk"     entries[10].signature = "abcekl"   (sorted)
entries[11].word = "baclekt"    entries[11].signature = "abceklt"  (sorted)
```

### 1.2 Signature Computation (Counting Sort)

```c
static void sort_chars(char *s, size_t len)
{
    int counts[CHAR_COUNT_SIZE] = {0};
    size_t i;
    size_t pos;
    int c;

    ASSERT_NOT_NULL(s);

    for (i = 0; i < len; i++)
    {
        ASSERT_MSG((unsigned char)s[i] < CHAR_COUNT_SIZE, "char out of range");
        counts[(unsigned char)s[i]]++;
    }

    pos = 0;
    for (c = 0; c < CHAR_COUNT_SIZE; c++)
    {
        while (counts[c]-- > 0)
        {
            s[pos++] = (char)c;
        }
    }
}
```

### 1.3 Hash Table

```mermaid
graph LR
    subgraph HashTable["Hash Table (bucket_count=12)"]
        B0["bucket[0]"]
        B2["bucket[2]"]
        B3["bucket[3]"]
        B4["bucket[4]"]
        B6["bucket[6]"]
        B7["bucket[7]"]
        B11["bucket[11]"]
    end
    
    B0 --> E0["ajkmnpqw<br/>[9]"]
    E0 --> N0([NULL])
    
    B2 --> E2a["abceklt<br/>[11]"]
    E2a --> E2b["abcdg<br/>[0]"]
    E2b --> N2([NULL])
    
    B3 --> E3a["abcekl<br/>[10]"]
    E3a --> E3b["abcdef<br/>[8]"]
    E3b --> E3c["abcd<br/>[1]"]
    E3c --> N3([NULL])
    
    B4 --> E4["abcdghi<br/>[7]"]
    E4 --> N4([NULL])
    
    B6 --> E6["abcdp<br/>[6]"]
    E6 --> N6([NULL])
    
    B7 --> E7a["abcek<br/>[3]"]
    E7a --> E7b["abc<br/>[5]"]
    E7b --> N7([NULL])
    
    B11 --> E11a["abcdgh<br/>[2]"]
    E11a --> E11b["abck<br/>[4]"]
    E11b --> N11([NULL])
```

**FNV-1a Hash Function:**
```c
static unsigned long hash_fnv1a(const char *s)
{
    unsigned long h;

    ASSERT_NOT_NULL(s);

    h = FNV_OFFSET_BASIS;

    while (*s)
    {
        h ^= (unsigned char)*s++;
        h *= FNV_PRIME;
    }

    return h;
}
```

---

## STEP 2: DFS Chain Search

```mermaid
flowchart TD
    START([Start DFS]) --> INIT["path[0] = start_index<br/>depth = 1"]
    INIT --> CALL["dfs(current_index, depth)"]
    
    CALL --> GETSIG["sig = entries[current].signature"]
    GETSIG --> LOOP{"For each char<br/>c = 33..126"}
    
    LOOP -->|Next char| INSERT["candidate = insert_sorted(sig, c)"]
    INSERT --> FIND{"hashtable_find<br/>(candidate)?"}
    
    FIND -->|Not found| LOOP
    FIND -->|Found entry| FOREACH{"For each word_idx<br/>in entry"}
    
    FOREACH -->|Next word| SAVEPATH["path[depth] = word_idx<br/>found = true"]
    SAVEPATH --> RECURSE["dfs(word_idx, depth+1)"]
    RECURSE --> FOREACH
    
    FOREACH -->|Done| LOOP
    
    LOOP -->|All chars done| CHECK{"found_continuation?"}
    
    CHECK -->|Yes| RETURN([Return])
    CHECK -->|No: LEAF node| SAVE["save_chain_if_longest(path, depth)"]
    SAVE --> RETURN
```

### DFS Trace Example (start: "abck")

```
dfs(4, depth=1) sig="abck"
│
├─ char='e' → candidate="abcek" → FOUND [3]
│  └─ dfs(3, depth=2) sig="abcek"
│     │
│     └─ char='l' → candidate="abcekl" → FOUND [10]
│        └─ dfs(10, depth=3) sig="abcekl"
│           │
│           └─ char='t' → candidate="abceklt" → FOUND [11]
│              └─ dfs(11, depth=4) sig="abceklt"
│                 │
│                 └─ No continuation found → LEAF
│                    Save chain: [4,3,10,11] length=4
│
└─ Result: "abck" → "abcek" → "abcelk" → "baclekt"
```

---

## Memory Modes

### Static Mode (Embedded)

```c
typedef struct {
    WordEntry entries[POOL_MAX_WORDS];
    StaticHashEntry hash_entries[POOL_MAX_HASH_ENTRIES];
    int hash_buckets[POOL_HASH_BUCKETS];
    StaticChains chains;
    DfsContext dfs;
} GlobalState;
```

### Dynamic Mode (PC)

```c
typedef struct {
    MemoryPools pools;
    Dictionary *dict;
    HashTable *ht;
    ChainResults *results;
    DfsContext dfs;
} GlobalState;
```

---

## Implementation Files

| File | Purpose |
|------|---------|
| `anagram_chain_core.c` | DFS, hash table, dictionary |
| `anagram_chain_io.c` | I/O, timers, validation |
| `config.h` | Memory pool constants |
| `global.h` | Type definitions |
| `trace.h` | Debug tracing |
| `assert.h` | Runtime checks |
| `anagram_chain.h` | Public API |
