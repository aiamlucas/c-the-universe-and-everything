# Libft Exploration

Libft is the first satellite in the 42 galaxy.

→ assembled from nothing but *pointers, patience and manual pages.

It orbits the raw memory sector from `0x00` to `0xFF`, traversing RAM, entering registers, moving through `rdx`, `rsi`, `rdi` >> one byte at a time.

100% handcraft.  
↳ or better: 100% fingercraft → launched into the universe via **fingertips**, as Vilém Flusser might have called it, echoing *Ins Universum der technischen Bilder*, Göttingen, 1985.  
→ [English edition](https://www.goodreads.com/book/show/9785668-into-the-universe-of-technical-images)

Brought into being at t = 0, via Makefile.

```
[█████████████████████████████████████░░░░░░] 85% — compiling...
```

## Table of Contents

- [memchr()](#memchr)

# memchr()

## Overview

`memchr()` is a C standard library function used to search through a specified number of bytes in memory, looking for the **first occurrence** of a particular byte value.

It operates on raw memory, not necessarily null-terminated strings.

---

## How to Visualize It

Imagine memory as a sequence of boxes, each holding a value:

[A] [B] [C] [D] [E] [F] [G]

Each box represents a byte in memory. You can think of it like this:

Byte index: 0   1   2   3   4   5   6  
Values:     A   B   C   D   E   F   G

If you're using `memchr()` to search for the value `'D'` and tell it to search the first 5 bytes, it will go through:

[A] [B] [C] [D] [E]

It finds `'D'` at byte index 3 and stops there.

If `'D'` wasn’t within the first 5 bytes, it would return a null result, indicating the value wasn't found in that range.

---

## Key Concepts

- `memchr()` examines memory byte-by-byte.
- It stops when:
  - The target byte (`c`) is found, or
  - The maximum number of bytes (`n`) has been searched.
- It interprets all bytes and the target value as `unsigned char` (values from 0 to 255).
- It returns a pointer to the matching byte or NULL if not found.

---

## Analogy

Think of it like scanning a row of lockers for a specific item:

- You say: "Check the first 6 lockers."
- You're looking for a red backpack.
- If the backpack is in locker 4, the scan stops there.
- If it's not in the first 6 lockers, you report "not found."

---

## When To Use It?

- Searching within binary data (not just text).
- Finding specific markers or delimiters in a buffer.
- Working with network packets, image data, or files where string functions won’t help.
- Anytime you need fast, low-level memory inspection.

```
// void *memchr(const void *s, int c, size_t n);
```

## Downsides of memchr()

- Only finds the **first** occurrence — unlike `memcpy()` or `memmove()`, it doesn’t operate on the whole buffer.
- Works on **raw bytes** — unlike `strchr()`, it doesn't stop at null terminators (`\0`), so it's not safe for C strings unless length is controlled.
- No structural awareness — can't distinguish meaningful data from noise (e.g., matching a byte in the middle of a multi-byte value).
- Easy to misuse — passing the wrong size can cause undefined behavior, similar to other `mem*` functions but riskier than `str*`.
