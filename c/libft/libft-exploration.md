# Libft Exploration

Libft is the first satellite in the 42 galaxy.

→ assembled from nothing but *pointers, patience and manual pages.

It orbits the raw memory sector from `0x00` to `0xFF`, traversing RAM, entering registers, moving through `rdx`, `rsi`, `rdi` >> one byte at a time.

100% handcraft.  
↳ or better: 100% fingercraft → launched into the universe via **fingertips**, as Vilém Flusser might have said! 

*Ins Universum der technischen Bilder*, Göttingen, 1985.  
→ [English edition](https://www.goodreads.com/book/show/9785668-into-the-universe-of-technical-images)

Brought into being at t = 0, via Makefile.

```
[█████████████████████████████████████░░░░░░] 85% — compiling...
```

## Table of Contents
- [memcpy()](#memcpy)
- [memchr()](#memchr)
- [memcmp(()](#memcmp)
- [strnstr()](#strnstr)
- [strlcat()](#strlcat)
- [strdup()](#strdup)

# memcpy()

## Overview

```memcpy()``` is a fundamental C function used to **copy raw bytes** from one memory location to another.

It doesn't care about strings, null terminators, or data types. Only the ***bytes*** matter.  
It copies exactly ```n``` bytes from ```src``` to ```dest```.

---

## How to Visualize It

Picture two memory rows, side by side. You're moving values from the top row into the bottom one:

Before ```memcpy()```:
```
src:   [A] [B] [C] [D] [E]  
dest:  [x] [x] [x] [x] [x]
```
You call:  
```memcpy(dest, src, 3)```

Afterward:
```
src:   [A] [B] [C] [D] [E]  
dest:  [A] [B] [C] [x] [x]
```
Only the first 3 bytes were copied. No interpretation, just raw byte movement.

---

## Key Concepts

- ```memcpy(dest, src, n)``` copies ```n``` bytes from ```src``` to ```dest```.
- You must make sure:
  - ```dest``` has enough space
  - The source and destination **do not overlap**
- It returns ```dest``` (the pointer to the destination).
- It is faster than most loops and often implemented in assembly or supported by hardware acceleration.

---

## Analogy

Imagine copying books from one shelf to another, one by one:

- ```src``` is the source shelf.
- ```dest``` is the destination shelf.
- ```n``` is the number of books to move.

You move them exactly in order.  
You don’t read them, interpret them or check the titles, you just move the physical items as it is.

---

## When To Use It?

- Duplicating binary data.
- Copying arrays, structs or raw memory blocks.
- Filling packet buffers, framebuffers or file chunks.
- Any situation where precision and speed matter more than interpretation.

```void *memcpy(void *dest, const void *src, size_t n);```

---

## Downsides of memcpy()

- **Dangerous if misused**: copying past buffer bounds causes undefined behavior.
- If ```src``` and ```dest``` overlap, results are undefined, use ```memmove()``` instead.
- No "smart" handling: doesn't stop at null bytes, doesn't adjust for types.

---

## Example

```
char src[] = "launch";
char dest[10];

memcpy(dest, src, 6);

// dest now contains: "launch"
```

# memchr()

## Overview

`memchr()` is a C standard library function used to search through a specified number of bytes in memory, looking for the **first occurrence** of a particular byte value.

It operates on raw memory, not necessarily null-terminated strings.

---

## How to Visualize It

Imagine memory as a sequence of boxes, each holding a value:

```
[A] [B] [C] [D] [E] [F] [G]
```

Each box represents a byte in memory. You can think of it like this:

```
Byte index: 0   1   2   3   4   5   6  
Values:     A   B   C   D   E   F   G
```

If you're using `memchr()` to search for the value `'D'` and tell it to search the first 5 bytes, it will go through:

```
[A] [B] [C] [D] [E]
```

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

# memcmp()

## Overview

`memcmp()` is the function that compares **two blocks of raw memory**, byte by byte, for a defined number of bytes.

It doesn’t care about strings, null terminators or formatting.  
It's all about **binary truth**: equal, greater or less.

---

## How to Visualize It

Picture two memory strips, side by side:

```
s1:  [A] [B] [C] [D] [E]  
s2:  [A] [B] [C] [X] [E]
```

Byte index: 0   1   2   3   4  
Compare up to `n = 5`.

- Index 0 → equal  
- Index 1 → equal  
- Index 2 → equal  
- Index 3 → `D` vs `X` → difference found  
→ `memcmp()` stops here and returns the **difference** between the bytes: `(D - X)`

If all compared bytes are identical, it returns `0`.

---

## Key Concepts

- `memcmp()` compares raw memory, treating bytes as `unsigned char`.
- It returns:
  - `0` if memory blocks are equal for `n` bytes.
  - A **positive** number if the first differing byte in `s1` is greater than in `s2`.
  - A **negative** number if it's smaller.
- It doesn’t stop at `\0`, every byte is significant.

---

## Analogy

Imagine two DNA sequences laid out side by side.

A scanner compares them base by base — A, T, C, G — one position at a time.

- As long as both sequences match at each position, the scanner continues.
- The moment it finds a mismatch (say, ```G``` vs ```C```), it halts and reports the *difference*,the chemical delta.
- If it finishes the comparison across ```n``` bases with no differences, it concludes the sequences are *identical* for that range.

Just like ```memcmp()```, it doesn’t try to interpret the meaning of the data. It just checks if the building blocks match, byte-for-base.

---

## When To Use It?

- Comparing binary files or network packets.
- Checking if two memory regions are identical.
- Comparing structures (when layout and padding are known).
- Quick equality checks at a low level, faster than iterating manually.

```
// int memcmp(const void *s1, const void *s2, size_t n);
```

---

## Downsides of memcmp()

- Returns **only the difference of the first mismatch**, not how many bytes differ.
- Not safe for comparing structures with padding unless you're careful.
- Doesn't short-circuit early in predictable ways, every byte must be explicitly told to matter.
- Comparison is purely mechanical: it won't help with encoding issues or logical equivalence (e.g., different byte orders representing same value).

---

## Example

```
char a[] = {1, 2, 3, 4};
char b[] = {1, 2, 5, 4};

int result = memcmp(a, b, 4); 
// result will be negative: 3 - 5 = -2 (at index 2)
```
---

# strnstr()

## Overview

``` strnstr() ``` is a C library function that searches for the **first occurrence** of a substring (``` needle ```) within a larger string (``` haystack ```), but only up to a specified maximum length.

Unlike ``` strstr() ```, it doesn’t search beyond the given ``` len ```. It operates within strict bounds efficient, predictable and safe.

---

## How to Visualize It

Imagine a text or song from Sun Ra.

```"space is the place"```

We can visualize it like a row of boxes, each one holds a character:

```
Text:   [s] [p] [a] [c] [e] [ ] [i] [s] [ ] [t] [h] [e] [ ] [p] [l] [a] [c] [e]  
Index:   0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17
```

Now suppose you're using ```strnstr()``` to search for ```"is"```  
and you set ```len = 6```.

It scans from index 0 to 5:
- ```"space "``` → no match for ```"is"```

The substring ```"is"``` begins at index 6, but that’s outside the limit → not found.

Increase ```len = 8```:
- Now it includes indexes 6 and 7
- Finds ```"is"``` starting at index 6 → match found, return pointer to that position.

---

## Key Concepts

- Compares a substring (``` needle ```) against portions of a larger string (``` haystack ```) up to ``` len ``` bytes.
- If ``` needle ``` is an empty string, ``` strnstr() ``` returns ``` haystack ```.
- If ``` needle ``` isn't found in the first ``` len ``` bytes, it returns ``` NULL ```.
- The search stops at the first match, not at the first difference.

---

## Analogy

You're listening to the beginning of a song, trying to catch the chorus (```needle```), which is the most familiar part.

But your headphones cut off after ```len``` seconds.

If the chorus starts and finishes entirely within that snippet, you recognize the song.  
But if it begins too late and gets cut off, it goes unnoticed.

---

## When To Use It?

- Looking for substrings safely within a limited range (e.g., bounded buffers).
- Avoiding overreads in potentially non-null-terminated strings.
- Working with untrusted input where length matters more than contents.
- Parsing protocols, headers or chunks of structured text without risking memory violations.

``` char *strnstr(const char *haystack, const char *needle, size_t len); ```

---

## Downsides of strnstr()

- Not standardized across all platforms (not part of ANSI C, but POSIX and BSD systems include it).
- Can silently return ``` NULL ``` if the pattern is just slightly beyond the ``` len ``` limit.
- May be slower than manual checks if you’re repeatedly searching small patterns in tight loops.
- Tricky edge cases — especially when ``` needle ``` or ``` haystack ``` are empty or shorter than ``` len ```.

---

## Example

```
const char *haystack = "launch sequence initiated";
const char *needle = "sequence";

char *found = strnstr(haystack, needle, 15);

// found will be NULL — "sequence" starts at index 7, but ends past 15.
```

# strlcat()

## Overview

```strlcat()``` appends one string to the end of another, but with an important constraint: it respects a fixed buffer size.

It’s like ```strcat()```, but safer. It won’t overflow the destination and it always null-terminates the result (as long as ```size > 0```).

It also returns the total length the final string *would have had*, letting you detect if any truncation occurred.

---

## How to Visualize It

You’re building a phrase in memory using a fixed-size buffer.

You start with:

```dest = "space "```  
```src = "is the place"```  
```size = 16```

We can visualize the buffer like this:

```
Buffer: [s] [p] [a] [c] [e] [ ] [i] [s] [ ] [t] [h] [e] [ ] [p] [l] [\0]  
Index:   0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
```

Only 10 extra bytes fit after ```"space "``` (which is 6 characters), so ```strlcat()``` appends as much of ```src``` as possible until the total buffer size (16) is reached.

The final string in ```dest``` becomes:  
```"space is the pl"```

It’s truncated, but null-terminated.  
The return value will be ```strlen(src) + strlen(original dest) = 13 + 6 = 19``` → telling you what the full string would have been if the buffer were big enough.

---

## Key Concepts

- ```strlcat()``` appends ```src``` to ```dest```, but only up to ```size - strlen(dest) - 1``` bytes.
- Ensures the result is null-terminated if ```size > 0```.
- Returns the total length it *tried* to create.
- If return value >= ```size```, truncation occurred.

---

## Analogy

You’re sending a message to an old-school pager that can only hold a limited number of characters.

- ```dest``` is already set to: ```"I "```  
- ```src``` is the new message: ```"love you"```  
- ```size``` is the pager’s maximum character limit

Let’s say ```size = 6```. That includes space for the null terminator, so only 5 visible characters will fit in total.

The pager adds as much of the new message as it can:

```"I love"``` → message gets cut off at ```"I lov"``` and null-terminated.

Even though only part of the message fits on the pager, for example, ```"I lov"``` the system still knows how many characters the *full* message would have required.

That’s enough to detect that something was cut off, even if it can’t tell what exactly.  
Was it ```"I love art"```? ```"I love sun"```? ```"I love cat"```? Or even ```"I love you"```?

---

## When To Use It?

- Safely appending strings to fixed-size buffers.
- Avoiding the dangers of ```strcat()``` (buffer overflows).
- Writing log lines, packet data or UI strings where bounds matter.
- Needing to know if truncation occurred, based on return value.

```size_t strlcat(char *dst, const char *src, size_t size);```

---

## Downsides of strlcat()

- Not part of ANSI C. It's mostly seen on BSD systems.
- The return value can be misinterpreted if you don’t know what it means.
- Requires ```dest``` to be properly null-terminated.
- Doesn't allocate memory, you manage the buffer.

---

## Example

```
char buffer[16] = "space ";
size_t result = strlcat(buffer, "is the place", 16);

// buffer now contains: "space is the pl"
// result is 19 — full length it tried to create
```

# strdup()

## Overview

```strdup()``` creates a **new copy of a string**, allocating just enough memory for it (including the null terminator).

It’s like saying:  
> “Give me a fresh copy of this string, living in its own space.”

You give it a ```const char *s``` and it returns a ```char *``` that points to a heap-allocated duplicate.

---

## How to Visualize It

Imagine this original string in memory:

```
Original:  [s] [p] [a] [c] [e] [ ] [i] [s] [\0]  
Pointer:    ↑
```

When you call ```strdup()```, it allocates space for a new copy:

```
Duplicate: [s] [p] [a] [c] [e] [ ] [i] [s] [\0]  
Pointer:                           ↑
```

You now have two separate pointers, pointing to the same content but living in different memory.

If you modify one, the other stays untouched.

---

## Key Concepts

- Allocates memory using ```malloc()```
- Copies the content byte-by-byte, including the ```'\0'``` at the end
- Returns ```NULL``` if memory allocation fails
- You are responsible for freeing the result with ```free()```

---

## Analogy

Think of a zine on the table.

- The original is one copy.
- You go to a photocopier and duplicate it.
- Now you can scribble on your copy, fold it or hand it to someone and the original remains untouched.

That’s what ```strdup()``` gives you: a fresh copy to do whatever you want with.

---

## When To Use It?

- You want to modify a string without touching the original
- You need a copy that survives outside the scope of a function or stack frame
- You’re building new structures that own their data (e.g., tokens, arguments, log lines)
- You need something that will outlive a temporary buffer

```char *strdup(const char *s);```

---

## Downsides of strdup()

- Uses ```malloc()```. So you must ```free()``` the result
- Fails silently if memory is tight, so always check for ```NULL```
- Doesn’t let you copy partial strings (use ```strndup()``` for that)

---

## Example

```
char *original = "launch";
char *copy = strdup(original);

printf("%s\n", copy);  // prints: launch

free(copy); // don't forget to clean up
```
