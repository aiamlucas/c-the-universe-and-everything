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
- [memset()](#memset)
- [memchr()](#memchr)
- [memcmp(()](#memcmp)
- [memmove()](#memmove)
- [strlcpy()](#strlcpy)
- [strnstr()](#strnstr)
- [strlcat()](#strlcat)
- [strchr()](#strchr)
- [strrchr()](#strrchr)
- [strncmp()](#strncmp)
- [strdup()](#strdup)
- [calloc()](#calloc)
- [ft_substr()](#ft_substr)
- [ft_strjoin()](#ft_strjoin)
- [ft_strtrim()](#ft_strtrim)
- [ft_split()](#ft_split)
- [ft_strmapi()](#ft_strmapi)
- [ft_striteri()](#ft_striteri)


# memcpy()

## Overview

```memcpy()``` is a fundamental C function used to **copy raw bytes** from one memory location to another.

It doesn't care about strings, null terminators or data types. Only the ***bytes*** matter.  
It copies exactly ```n``` bytes from ```src``` to ```dest```.

---

## Prototype

```void *memcpy(void *dest, const void *src, size_t n);```

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

---

## Downsides of memcpy()

- **Dangerous if misused**: copying past buffer bounds causes undefined behavior.
- If ```src``` and ```dest``` overlap, results are undefined, use ```memmove()``` instead.
- No "smart" handling: doesn't stop at null bytes, doesn't adjust for types.

---

## Example

```// Prototype: void *memcpy(void *dest, const void *src, size_t n);```

```
char src[] = "launch";
char dest[10];

memcpy(dest, src, 6);

// dest now contains: "launch"
```

# memset()

## Overview

```memset()``` is a C standard library function used to fill a block of memory with a single byte value.

It’s commonly used to initialize arrays, buffers, or structs to a known value, typically ```0```.

---

## Prototype

```void *memset(void *s, int c, size_t n);```

---

## How to Visualize It

Picture a row of memory boxes:

```
[?] [?] [?] [?] [?] [?] [?] [?]
```

You call:

```memset(ptr, 'A', 5)```

Result:

```
[A] [A] [A] [A] [A] [?] [?] [?]
```

The first 5 bytes are now filled with ```'A'```. The rest remain untouched.

---

## Key Concepts

- ```memset(ptr, value, n)``` fills the first ```n``` bytes at ```ptr``` with ```value``` (as an ```unsigned char```).
- Operates on raw memory, it doesn't know or care about the data type.
- Commonly used for:
  - Initializing buffers to zero
  - Setting flags or padding to known values
  - Wiping memory before reuse

---

## Analogy

You have a digital display of 1,000 LEDs still showing the last frame from a previous configuration.

- The LED grid is memory.
- The outdated display is leftover data.
- ```memset()``` sends a signal to clear the frame.
- All LEDs are instantly reset to the same value (like a full black or a solid blue).
- The result is a clean, uniform state and ready for the next update.

---

## When To Use It?

- Zeroing out arrays or structs before use.
- Resetting memory after free (optional, for safety or debugging).
- Creating fixed patterns in buffers (e.g., ```0xFF``` for testing).
- Filling memory with ```\0``` to prepare for string operations.

---

## Downsides of memset()

- Only sets memory at the byte level, it's not safe for assigning typed values.
- If used incorrectly (e.g., wrong size) can lead to data corruption.
- Not suitable for setting values in arrays of types larger than ```char``` (e.g., ```int arr[10]```), unless you’re just zeroing.

---

## Example

```// Prototype: void *memset(void *s, int c, size_t n);```

```
#include <stdio.h>
#include <string.h>

int main(void)
{
    int numbers[5];
    int i = 0;

    // Set all bytes in the array to 0
    memset(numbers, 0, sizeof(numbers));

    while (i < 5)
    {
        printf("%d ", numbers[i]);  // Output: 0 0 0 0 0
        i++;
    }

    return 0;
}
```

# memchr()

## Overview

`memchr()` is a C standard library function used to search through a specified number of bytes in memory, looking for the **first occurrence** of a particular byte value.

It operates on raw memory, not necessarily null-terminated strings.

---

## Prototype

```void *memchr(const void *s, int c, size_t n);```

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


## Downsides of memchr()

- Only finds the **first** occurrence — unlike `memcpy()` or `memmove()`, it doesn’t operate on the whole buffer.
- Works on **raw bytes** — unlike `strchr()`, it doesn't stop at null terminators (`\0`), so it's not safe for C strings unless length is controlled.
- No structural awareness — can't distinguish meaningful data from noise (e.g., matching a byte in the middle of a multi-byte value).
- Easy to misuse — passing the wrong size can cause undefined behavior, similar to other `mem*` functions but riskier than `str*`.

---

## Example

```// Prototype: void *memchr(const void *s, int c, size_t n);```

```
char data[] = "space is the place";
char *result = memchr(data, 'e', 10);

// result points to: &data[4]
// value at that position: 'e'
```

# memcmp()

## Overview

`memcmp()` is the function that compares **two blocks of raw memory**, byte by byte, for a defined number of bytes.

It doesn’t care about strings, null terminators or formatting.  
It's all about **binary truth**: equal, greater or less.

---

## Prototype

```int memcmp(const void *s1, const void *s2, size_t n);```

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

---

## Downsides of memcmp()

- Returns **only the difference of the first mismatch**, not how many bytes differ.
- Not safe for comparing structures with padding unless you're careful.
- Doesn't short-circuit early in predictable ways, every byte must be explicitly told to matter.
- Comparison is purely mechanical: it won't help with encoding issues or logical equivalence (e.g., different byte orders representing same value).

---

## Example

```// Prototype: int memcmp(const void *s1, const void *s2, size_t n);```

```
char a[] = {1, 2, 3, 4};
char b[] = {1, 2, 5, 4};

int result = memcmp(a, b, 4); 
// result will be negative: 3 - 5 = -2 (at index 2)
```
---

# memmove()

## Overview

```memmove()``` is a memory copying function like ```memcpy()```, but with a key difference: it handles overlapping memory regions safely.

It copies ```n``` bytes from ```src``` to ```dest``` and guarantees that no data is corrupted, even if ```src``` and ```dest``` overlap.

---

## Prototype

```void *memmove(void *dest, const void *src, size_t n);```

---

## Key Concepts

- Works like ```memcpy()```, but safely handles overlap.
- If ```src``` is before ```dest``` and regions overlap, it copies **backward** to avoid overwriting data.
- Returns ```dest```.

---

## How to Visualize It

You have this memory block:

```
Index:   0   1   2   3   4   5   6   7  
Memory: [A] [B] [C] [D] [E] [F] [G] [H]
```

Now you call:  
```memmove(&mem[2], &mem[0], 4);```  
→ Copy 4 bytes from index 0 to index 2

The memory regions overlap:
You're trying to copy ```[A][B][C][D]``` onto the area starting at ```mem[2]```, which already contains ```C, D, E...```.

If you used ```memcpy()```, it would copy left to right, which could overwrite data before it's been moved:

Step-by-step with ```memcpy()```:  

```
mem[2] = mem[0] → 'A'  
mem[3] = mem[1] → 'B'  
mem[4] = mem[2] → now mem[2] is 'A'  
mem[5] = mem[3] → now mem[3] is 'B'  
```

Result (corrupted):  

```
[A] [B] [A] [B] [C?] [D?] ...
```

```memmove()``` avoids this by copying backward:

Step-by-step with ```memmove()```:  

```
mem[5] = mem[3] → 'D'  
mem[4] = mem[2] → 'C'  
mem[3] = mem[1] → 'B'  
mem[2] = mem[0] → 'A'  
```

Result (correct):  

```
[A] [B] [A] [B] [C] [D] ...
```
---

## Analogy

You're in a lab, working with a strip that displays the visible spectrum: from violet to red.

You need to shift a section of the spectrum slightly forward to realign it with a new reference measurement.

- ```src``` is the original section of wavelengths you're adjusting.
- ```dest``` is the new position, slightly ahead on the same strip.
- If you copy left to right (from violet to red), you'll overwrite parts of the spectrum before you finish.
- ```memmove()``` avoids this by copying from the red end first, preserving every wavelength during the shift.

---

## When To Use It?

- Moving data within the same buffer or array.
- Shifting or reshuffling elements.
- Safer alternative to ```memcpy()``` when you're unsure about overlap.

---

## Downsides of memmove()

- Slightly slower than ```memcpy()``` because it checks for overlap.
- Doesn’t protect you from copying outside the bounds, undefined behavior still applies.
- Doesn’t stop at null terminators, works on raw memory only.

---

## Example


```// Prototype: void *memmove(void *dest, const void *src, size_t n);```

```
#include <stdio.h>
#include <string.h>

int main(void)
{
    char buffer[20] = "1234567890";

    // Overlapping copy — copy "12345" into position 2
    memmove(buffer + 2, buffer, 5);

    printf("%s\n", buffer);
    // Output: "1212345890"

    return 0;
}
```

# strlcpy()

## Overview

```strlcpy()``` copies a string from a source to a destination buffer, while ensuring the copy is **safe** and does not exceed the buffer's size.

It ensures that the destination string is **null-terminated** and that **no buffer overflows** occur, as long as ```size > 0```.

It’s a safer alternative to ```strcpy()```.

---

## Prototype

```size_t strlcpy(char *dst, const char *src, size_t size);```

---

## Key Concepts

- Copies up to ```size - 1``` characters from ```src``` into ```dst```.
- Always null-terminates the destination if ```size > 0```.
- Returns the **total length** of ```src``` (not the number of bytes copied).
- If the return value ≥ ```size```, it means **truncation** occurred.

---

## How to Visualize It

Imagine:

```
src: [h] [e] [l] [l] [o] [!] [\0]
dst: [x] [x] [x] [x] [x] [x] [x]
size = 6
```

After calling ```strlcpy(dst, src, 6)```:

```
dst: [h] [e] [l] [l] [o] [\0] [?]
```

- Copied 5 characters, added null terminator
- Return value: 6 (length of src)
- The return value tells you what *would* have been copied

---

## Analogy

You're pouring water into a bottle:

- ```src``` is the water source.
- ```dst``` is the bottle.
- ```size``` is the bottle's capacity.
- ```strlcpy()``` fills the bottle up to the safe limit and leaves a marker (null byte) at the end.
- It also tells you how much water there was in total, even if not all of it fit.

---

## When To Use It?

- Safer string copying into fixed-size buffers.
- When you want guaranteed null termination.
- When you care about whether truncation occurred.
- Alternative to ```strncpy()``` or ```strcpy()```.

---

## Downsides of strlcpy()

- Not part of ANSI C (commonly found on BSD systems and some Linux distros).
- Requires checking the return value to detect truncation.
- Slower than ```memcpy()``` due to null checking and length computation.

---

## Example

```
// Prototype: size_t strlcpy(char *dst, const char *src, size_t size);

#include <stdio.h>
#include <string.h>

int main(void)
{
    char buffer[6];
    size_t copied = strlcpy(buffer, "launch", sizeof(buffer));

    printf("buffer: \"%s\"\n", buffer);       // Output: "launc"
    printf("return: %zu\n", copied);          // Output: 6 (length of src)

    return 0;
}

---

## Note

If you want to know **how much space you needed**, compare the return value to your buffer size:

if (strlcpy(dst, src, size) >= size)
{
    // Truncation occurred
}
```

# strnstr()

## Overview

``` strnstr() ``` is a C library function that searches for the **first occurrence** of a substring (``` needle ```) within a larger string (``` haystack ```), but only up to a specified maximum length.

Unlike ``` strstr() ```, it doesn’t search beyond the given ``` len ```. It operates within strict bounds efficient, predictable and safe.

---

## Prototype

```char *strnstr(const char *haystack, const char *needle, size_t len);```

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

---

## Downsides of strnstr()

- Not standardized across all platforms (not part of ANSI C, but POSIX and BSD systems include it).
- Can silently return ``` NULL ``` if the pattern is just slightly beyond the ``` len ``` limit.
- May be slower than manual checks if you’re repeatedly searching small patterns in tight loops.
- Tricky edge cases — especially when ``` needle ``` or ``` haystack ``` are empty or shorter than ``` len ```.

---

## Example

```// Prototype: char *strnstr(const char *haystack, const char *needle, size_t len); ```

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

## Prototype

```size_t strlcat(char *dst, const char *src, size_t size);```

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

---

## Downsides of strlcat()

- Not part of ANSI C. It's mostly seen on BSD systems.
- The return value can be misinterpreted if you don’t know what it means.
- Requires ```dest``` to be properly null-terminated.
- Doesn't allocate memory, you manage the buffer.

---

## Example

```// Prototype: size_t strlcat(char *dst, const char *src, size_t size);```

```
char buffer[16] = "space ";
size_t result = strlcat(buffer, "is the place", 16);

// buffer now contains: "space is the pl"
// result is 19 — full length it tried to create
```

# strchr()

## Overview

```strchr()``` searches for the **first occurrence** of a character in a string.  
It returns a pointer to the first matching character or ```NULL``` if not found.

Unlike ```memchr()```, it works only on **null-terminated strings** and treats ```\0``` as a valid character to search for.

---

## Prototype

```char *strchr(const char *s, int c);```

---

## How to Visualize It

Imagine a string laid out like this:

```
[s] [p] [a] [c] [e] [ ] [i] [s] [ ] [t] [h] [e] [\0]
```

You search for ```'e'```.  
```strchr()``` returns a pointer to the **first** ```'e'```, at index 4.

---

## Key Concepts

- Scans a string from beginning to end.
- Returns a pointer to the first occurrence of the character ```c```.
- If the character isn’t found, returns ```NULL```.
- If ```c == '\0'```, it returns a pointer to the string's null terminator.
- Operates only on strings (null-terminated arrays of ```char```).

---

## Analogy

You're scanning a line of books on a shelf, looking for the first one with a red cover:

- The string is the shelf.
- The target character is the red cover.
- ```strchr()``` stops and points to the **first** match it finds.

---

## When To Use It?

- Finding the first appearance of a character in a string.
- Parsing strings — e.g., search for ':' or '=' in key-value pairs.
- Detecting newlines, delimiters, or control characters.
- Checking if a character exists before doing further logic.

---

## Downsides of strchr()

- Returns only the **first** match. Use ```strrchr()``` for the last match.
- Not suited for scanning binary data (must be null-terminated).
- Won’t work correctly if the string isn't properly terminated with ```\0```.

---

## Example

```// Prototype: char *strchr(const char *s, int c);```

```
#include <stdio.h>
#include <string.h>

int main(void)
{
    const char *text = "space is the place";
    char *first_e = strchr(text, 'e');

    if (first_e)
        printf("First 'e' found at: %ld\n", first_e - text);
    else
        printf("'e' not found\n");

    return 0;
}
```

# strrchr()

## Overview

```strrchr()``` is a C standard library function that searches for the **last occurrence** of a character in a string.

It works just like ```strchr()```, but instead of stopping at the first match, it scans the **entire string** and returns a pointer to the **last** match.

---

## Prototype

```char *strrchr(const char *s, int c);```

---

## How to Visualize It

Imagine scanning a sentence from left to right, but you're only interested in the **last time** a certain letter appears.

Example:

```
Text: [s] [p] [a] [c] [e] [ ] [i] [s] [ ] [t] [h] [e] [ ] [p] [l] [a] [c] [e] [\0]
Index: 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18
```
If you're looking for ```'e'```, the **first** one is at index 4, but ```strrchr()``` will return the one at index 17 — the **last** occurrence.

---

## Key Concepts

- Searches from **start to end**, but only remembers the **last match**.
- Returns a pointer to the last occurrence of the character.
- If the character is not found, returns ```NULL```.
- Includes the null terminator ```'\0'``` as a valid character to search for.

---

## Analogy

You're reading a book looking for the last mention of a character's name.

- You highlight every mention of "Lou" as you go.
- At the end, you flip back to the **last one you highlighted**.
- That’s what ```strrchr()``` gives you — the final appearance.

---

## When To Use It?

- You need to find a file extension: ```strrchr(filename, '.')```.
- Reversely parsing paths, emails, or suffixes.
- Looking for the last newline, space, or delimiter in a string.

---

## Downsides of strrchr()

- Slower than ```strchr()``` in long strings — it must scan the entire string.
- Only finds **one** match — the last one.
- Works only on null-terminated strings.

---

## Example

```// Prototype: char *strrchr(const char *s, int c);```

```
const char *text = "space is the place";
char *last_e = strrchr(text, 'e');

// last_e now points to &text[17] → the last 'e'
printf("%s\n", last_e); // prints: "e"

```

# strncmp()

## Overview

```strncmp()``` compares up to ```n``` characters of two strings.  
It checks if the strings are equal, or which one is greater (**byte by byte**, using ASCII values).

It stops comparing when it finds a difference, hits a null terminator, or reaches ```n```.

---

## Prototype

```int strncmp(const char *s1, const char *s2, size_t n);```

---

## Key Concepts

- Returns:
  - ```0``` if the strings are equal up to ```n``` characters
  - A **positive** value if ```s1 > s2``` at the first mismatch
  - A **negative** value if ```s1 < s2``` at the first mismatch
- Comparison is **lexicographic**, based on ASCII
- Safe for comparing substrings or prefixes
- Stops at ```\0``` in either string
- ```strncmp()``` is like ```memcmp()```, but for C-strings. It stops at ```\0``` or after ```n```, while ```memcmp()``` compares raw memory without regard for null terminators.

---

## How to Visualize It

Imagine two strings laid out like:

```
s1: [s] [p] [a] [c] [e] [X]
s2: [s] [p] [a] [c] [e] [Y]
```

If you call:

```strncmp(s1, s2, 5)```

It only checks the first 5 characters:

```
[s] = [s]
[p] = [p]
[a] = [a]
[c] = [c]
[e] = [e]
```

All equal → returns 0

---

If you call:

```strncmp(s1, s2, 6)```

It compares the sixth:

```
[X] < [Y] → returns negative
```
---

## Analogy

You’re scanning two barcodes side by side:

- Start comparing the stripes from left to right.
- You only check the first ```n``` segments.
- If all match, you treat them as identical.
- If one stripe is different, you immediately register which is “greater” or “less.”

---

## When To Use It?

- Comparing the beginnings of strings
- Sorting lexicographically up to a limit
- Implementing custom ```strncmp()```-based logic in parsers or filters

---

## Downsides of strncmp()

- Doesn’t tell you where the strings differ, only that they do
- Can give unexpected results with signed char inputs unless explicitly cast
- Stops on ```\0```, so not always suitable for binary data

---

## Example

```
// Prototype: int strncmp(const char *s1, const char *s2, size_t n);

#include <stdio.h>
#include <string.h>

int main(void)
{
    const char *a = "spaceA";
    const char *b = "spaceB";

    int result = strncmp(a, b, 5);

    if (result == 0)
        printf("The first 5 characters are the same.\n");
    else if (result < 0)
        printf("\"%s\" is less than \"%s\" in the first 5 characters.\n", a, b);
    else
        printf("\"%s\" is greater than \"%s\" in the first 5 characters.\n", a, b);

    return 0;
}
```

# strdup()

## Overview

```strdup()``` creates a **new copy of a string**, allocating just enough memory for it (including the null terminator).

It’s like saying:  
> “Give me a fresh copy of this string, living in its own space.”

You give it a ```const char *s``` and it returns a ```char *``` that points to a heap-allocated duplicate.

---

## Prototype

```char *strdup(const char *s);```

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
Pointer:                            ↑
```

You now have two separate pointers, pointing to the same content but living in different memory.

If you modify one, the other stays untouched.

---

## Key Concepts

- Allocates memory using ```malloc()```.
- Copies the content byte-by-byte, including the ```'\0'``` at the end.
- Returns ```NULL``` if memory allocation fails.
- You are responsible for freeing the result with ```free()```.

---

## Analogy

Think of a zine on the table.

- The original is one copy.
- You go to a photocopier and duplicate it.
- Now you can scribble on your copy, fold it or hand it to someone and the original remains untouched.

That’s what ```strdup()``` gives you: a fresh copy to do whatever you want with.

---

## When To Use It?

- You want to modify a string without touching the original.
- You need a copy that survives outside the scope of a function or stack frame.
- You’re building new structures that own their data (e.g., tokens, arguments, log lines).
- You need something that will outlive a temporary buffer

---

## Downsides of strdup()

- Uses ```malloc()```. So you must ```free()``` the result.
- Fails silently if memory is tight, so always check for ```NULL```.
- Doesn’t let you copy partial strings (use ```strndup()``` for that).

---

## Example

```// Prototype: char *strdup(const char *s);```

```
char *original = "launch";
char *copy = strdup(original);

printf("%s\n", copy);  // prints: launch

free(copy);
```

# calloc()

## Overview

```calloc()``` is a memory allocation function that not only reserves space, it **also clears it**.

Unlike ```malloc()```, which leaves memory uninitialized, ```calloc()``` ensures that all bytes are set to zero.

You pass in how many elements you want, and how big each one is.  
It multiplies the two and returns a pointer to a zeroed block of memory.

---

## Prototype

```void *calloc(size_t count, size_t size);```

---

## how to visualize it

Imagine allocating memory for 5 bytes.

With ```malloc(5)```:

```
Allocated:  [??] [??] [??] [??] [??]  
Values:     0x3C 0x9A 0x00 0xF1 0x77
```

The contents are unpredictable, the memory is uninitialized and may contain garbage values.

With ```calloc(5, 1)```:

```
Allocated:  [00] [00] [00] [00] [00]  
Values:     0x00 0x00 0x00 0x00 0x00
```

Each byte is set to ```0x00```, ensuring a clean and predictable starting state.

---

## Key Concepts

- ```calloc(count, size)``` allocates ```count × size``` bytes.
- All bytes are zero-initialized (```0x00```).
- Return value is ```NULL``` if the allocation fails.
- Often used to avoid explicitly calling ```memset()``` after allocation.
- Total size is calculated internally, it protects against some overflow bugs.

---

## Analogy

imagine a freshly printed page:

- ```malloc(100)``` gives you a sheet of paper, but it might have might have ink streaks or artifacts from past use.
- ```calloc(100, 1)``` gives you a **clean, white sheet**. Completely empty, no garbage data.

in memory terms, every byte is set to ```0x00```.

---

## When To Use It?

- You need zero-initialized memory.
- You're allocating space for arrays, structs or buffers that must start empty.
- You want to avoid garbage values in newly allocated space.
- You want extra safety: ```calloc()``` checks for multiplication overflow internally.

```void *calloc(size_t count, size_t size);```

---

## Downsides of calloc()

- May be slower than ```malloc()``` in performance-critical code (zeroing takes time).
- Still returns ```NULL``` on failure, so checking the result is essential.
- If you don't need the memory cleared, ```malloc()``` is usually faster.

---

## Example

```// Prototype: void *calloc(size_t count, size_t size);```

```
int *numbers = calloc(5, sizeof(int));

// numbers points to: [0] [0] [0] [0] [0]

if (!numbers)
{
    // handle allocation failure
}

free(numbers);
```

# ft_substr()

## Overview

```ft_substr()``` extracts a substring from a given string, starting at a specific index and copying up to a given length.

It returns a **newly allocated string** that contains the portion from ```s[start]``` up to ```len``` characters or less, if the string ends before that.

---

## Prototype

```char *ft_substr(char const *s, unsigned int start, size_t len);```

---

## How to Visualize It

Imagine this string laid out in memory:

```
s:   [s] [p] [a] [c] [e] [ ] [i] [s] [ ] [t] [h] [e] [\0]
idx:  0   1   2   3   4   5   6   7   8   9  10  11
```

You call:

```ft_substr("space is the", 6, 4)```

It starts at index 6 (the ```'i'```) and grabs 4 characters:

→ returns ```"is t"```

---

## Key Concepts

- Returns a freshly allocated substring.
- If ```start``` is past the end of the string, returns an empty string.
- If ```len``` exceeds the available characters from ```start```, it copies only what’s left.
- Uses ```malloc()``` — remember to ```free()``` it later.

---

## Analogy

You're producing music:

```s``` is the full audio track.  
```start``` is where you place the marker.  
```len``` is how many seconds you want.  
```ft_substr()``` extracts a sound bite for remix or sample use.

---

## When To Use It?

- Extracting parts of a string (e.g., word, sentence, token).
- Slicing strings in parsers or tokenizers.
- Copying a region of interest into a new buffer.

---

## Downsides of ft_substr()

- Requires memory allocation.
- Doesn’t modify original string.
- Must handle edge cases (e.g., ```start > strlen(s)```).

---

## Example


```// Prototype: char *ft_substr(char const *s, unsigned int start, size_t len);```

```
char *src = "space is the place";
char *cut = ft_substr(src, 6, 7);

// cut → "is the " (includes the space at the end)

printf("%s\n", cut); // prints: is the

free(cut);
```

# ft_strjoin()

## Overview

```ft_strjoin()``` creates a **new string** by joining two strings together, placing ```s1``` first, then ```s2``` immediately after.

It dynamically allocates memory for the result using ```malloc()``` and returns a pointer to the new combined string.

---

## Prototype

```char *ft_strjoin(char const *s1, char const *s2);```

---

## How to Visualize It

Imagine two strings laid out like train cars:

```
[s1]: [H] [e] [l] [l] [o]  
[s2]: [ ] [W] [o] [r] [l] [d]
```

```ft_strjoin(s1, s2)``` allocates a new track, then copies both trains onto it:
```
[joined]: [H] [e] [l] [l] [o] [ ] [W] [o] [r] [l] [d] [\0]
```

It includes both strings, end-to-end and adds a null terminator at the end.

---

## Key Concepts

- The function **allocates** enough memory to hold the full length of ```s1``` + ```s2``` + 1 (for ```'\0'```).
- It **copies** the content of ```s1``` and ```s2``` into the new memory.
- The result must be ```free()```d after use.
- If either input is ```NULL``` or if memory allocation fails, it returns ```NULL```.

---

## Analogy

Imagine making a mixtape:

- ```s1``` is your side A.
- ```s2``` is your side B.
- ```ft_strjoin()``` creates a new tape that plays both, one after the other.

You don’t overwrite the originals. You build a brand new track with both parts stitched together.

---

## When To Use It?

- Combining strings to form file paths or URLs.
- Appending suffixes, extensions or formatted text.
- Building log entries or error messages dynamically.
- Creating clean, heap-allocated strings without modifying inputs.

---

## Downsides of ft_strjoin()

- Uses ```malloc()``` and must ```free()``` the result to avoid memory leaks.
- If either input is ```NULL```, the behavior is undefined unless you check first.
- Not optimized for repeated joins in loops (consider ```strlcat()``` with buffers instead).

---

## Example

```// Prototype: char *ft_strjoin(char const *s1, char const *s2);```

```
char *prefix = "Hello, ";
char *suffix = "world!";
char *result = ft_strjoin(prefix, suffix);

// result points to: "Hello, world!"
// Don’t forget to free it!
free(result);
```

# ft_strtrim()

## Overview

```ft_strtrim()``` removes unwanted characters from the beginning and end of a string. You define the characters to strip using the ```set``` parameter.

It returns a newly allocated string with the trimmed content.

---

## Prototype

```char *ft_strtrim(char const *s1, char const *set);```

---

## Parameters

- ```s1```: The original string to be trimmed.
- ```set```: The set of characters to remove from the start and end of ```s1```.

---

## Key Concepts

- Characters in ```set``` are removed from both ends.
- The middle part of the string is preserved exactly as is.
- A new string is allocated using ```malloc()```.
- Returns ```NULL``` if memory allocation fails.

---

## How to Visualize It

Imagine your string is wrapped in padding you want to strip away:

```
s1: [' ']['\n'][' ']['H']['e']['l']['l']['o'][' ']['\t']
set: [' ', '\n', '\t']
```

The trimmed result becomes:

```
['H']['e']['l']['l']['o']
```

Only the unwanted characters at the start and end are removed, not those inside the string.

---


## Analogy

Imagine editing a film strip:

- ```s1``` is the raw footage.
- ```set``` is the slate and blank frames at the start and end.
- ```ft_strtrim()``` cuts off those unwanted parts, leaving just the meaningful scene.

---

## When To Use It?

- Cleaning up user input (e.g., remove whitespace).
- Stripping formatting or markers from strings.
- Trimming file names, command-line args, or text fields.
- Any time you want to sanitize the edges of a string.

---

## Example

```// Prototype: char *ft_strtrim(char const *s1, char const *set);```

```
#include <stdio.h>
#include <stdlib.h>

char *ft_strtrim(char const *s1, char const *set);

int main(void)
{
    char *original = " \n\t  Hello World! \t\t\n";
    char *trimmed = ft_strtrim(original, " \n\t");

    if (trimmed)
    {
        printf("Trimmed: '%s'\n", trimmed);
        free(trimmed);
    }
    return 0;
}
```

---

## Output

```Trimmed: 'Hello World!'```

---

## Notes

- Does not modify the original string.
- If ```s1``` is entirely made of ```set``` characters, it returns an empty string.
- Be sure to ```free()``` the result to avoid memory leaks.

# ft_split()

## Overview

```ft_split()``` takes a string and splits it into an array of substrings, using a specific delimiter character.
It allocates memory for each substring and returns a ```NULL``` terminated array of newly allocated strings.

---

## Prototype

```char **ft_split(char const *s, char c);```

---

## Key Concepts

- Parses the string ```s``` and breaks it into segments wherever ```c``` appears.
- Allocates memory for each substring using ```malloc()```.
- Returns an array of pointers, one for each substring, ending with a ```NULL``` pointer.
- Returns ```NULL``` if allocation fails at any point (and should free any already-allocated memory).

---

## How to Visualize It

Imagine a string like:

```
[s] [p] [a] [c] [e] [,] [i] [s] [,] [,] [t] [h] [e]
```

And your delimiter is ```','```.
The function scans the string, skipping over single or repeated commas and splits it wherever it finds content between them:

```
→ "space"
→ "is"
→ "the"
```

Each piece becomes a new string in the array. 
The array ends with a ```NULL``` pointer:

```
result[0]: [ "space" ]
result[1]: [  "is"   ]
result[2]: [  "the"  ]
result[3]: [  NULL   ]
```
---

Memory layout:

```
result:
  +--------+
  | ptr →  | → "space"
  +--------+
  | ptr →  | → "is"
  +--------+
  | ptr →  | → "the"
  +--------+
  | NULL   |
  +--------+
```

> Each ```ptr``` points to a null-terminated string allocated separately. 
> The array itself is also allocated. It's a dynamic list of string pointers ending with ```NULL```.

---

## Analogy

Splitting a Book into Articles
You have a large book containing multiple articles, separated by blank pages.

- ```s``` is the full book.
- ```c``` is the blank page, the marker between articles.
- Every time you find a blank page, you cut out the article that came before it.
- ```ft_split()``` gives you a stack of small books, each holding one article.
- The last slot is empty and it tells you: "there are no more articles left."

---

## When To Use It?

- Breaking CSV-like strings or command-line input into parts.
- Tokenizing strings based on spaces, commas, or other characters.
- Separating fields from logs or config lines.

---

## Downsides of ft_split()

- Memory-heavy: allocates a new string for every chunk.
- Needs careful cleanup if any ```malloc()``` call fails.
- Doesn’t handle escaped delimiters or nested structures, just raw split logic.
- Performs just a simple split at every delimiter. It doesn't recognize escape characters or groupings like quotes or parentheses.

---

## Example

```// Prototype: char **ft_split(char const *s, char c);```

```
#include <stdio.h>

int main(void)
{
    char **words = ft_split("launch,sequence,initiated", ',');

    if (!words)
        return (1);

    int i = 0;
    while (words[i])
    {
        printf("[%s]\n", words[i]);
		free(words[i]); // Free each substring
        i++;
    }
	free(words); // Free the array of pointers
    return 0;
}
```

## Strategies to Solve It

To implement ```ft_split()``` with input:

```
"space,,,is,,the"
```

and delimiter ```','```, we can break the problem down into structured steps.

---

### Step 1: Count how many substrings you’ll need

Scan through the string to count how many *non-empty* segments appear between delimiters.
Visual breakdown:

```
[s] [p] [a] [c] [e] [,] [,] [,] [i] [s] [,] [,] [t] [h] [e]
```

- First word: "space"
- Skip 3 commas
- Second word: "is"
- Skip 2 commas
- Third word: "the"

**Total: 3 substrings**

This tells us how many pointers we need to allocate in the ```char **``` array.  
Don’t forget to add ```1``` extra for the final ```NULL``` terminator.

---

### Step 2: Identify each word’s range

Walk through the string:

- Skip delimiters
- When you find a non-delimiter, mark it as the start of a word
- Continue until you hit the next delimiter or end of string
- That’s your end

From the input:

- Word 1: indices 0–4 → "space"
- Word 2: indices 8–9 → "is"
- Word 3: indices 12–14 → "the"

You now have the boundaries needed to extract substrings.

---

### Step 3: Allocate and copy each substring

For each word:

- Measure its length: ```len = end - start```
- Allocate ```len + 1``` bytes (to hold the word and ```\0```)
- Use ```ft_substr()``` to extract the slice
- Null-terminate it (which ```ft_substr()``` handles for you)

Then store that pointer in your ```char **array```.
Final result:

```
array[0] = "space"
array[1] = "is"
array[2] = "the"
array[3] = NULL
```

---

### Step 4: Finalize the array

Set the final element in the ```char **``` array to ```NULL``` to indicate the end.  
This makes iteration safe — similar to how ```argv[]``` works in ```main()```.

---

### Step 5: Handle memory allocation failures

At any point, if a ```malloc()``` fails:

- Free all previously allocated substrings
- Free the array itself
- Return ```NULL```

Use a helper like ```free_split(char **array)``` for this cleanup.

---

### Summary of Tools You Can Use

- ```ft_substr()``` — extract each word cleanly
- ```malloc()```, ```free()``` — for memory control
- ```ft_strlen()``` — optionally to measure word lengths
- ```free_split()``` — to release memory if something fails

This structured process — counting, locating, slicing, and cleaning up. Makes your ```ft_split()``` implementation both safe and readable.

---

### Implementation Tip: Break It Into Helpers

To keep your ```ft_split()``` readable and modular, consider writing small helper functions for each major step.

Some examples:

- ```int count_words(const char *s, char c);```  
  → Scans the string and returns how many substrings (words) will be created.
- ```int word_length(const char *s, char c);```  
  → Starting at ```s```, returns the length of the next word (number of characters until the next delimiter or ```\0```).
- ```void free_split(char **array);```  
  → Frees all memory previously allocated for the array of substrings in case of failure or cleanup.

Using these helpers makes your main ```ft_split()``` logic easier to write and debug.

### Behind the Scenes: What's ```char **```?

The return type of ```ft_split()``` is a **double pointer** → ```char **```.
Why? Because you're returning an array of strings.
Each ```char *``` points to a dynamically allocated word.
The ```char **``` itself is a pointer to the first element of this array (an array of pointers).
Here’s what it looks like in memory:

```
result → [ ptr ] → "space"
[ ptr ] → "is"
[ ptr ] → "the"
[ NULL ]
```

- Each word is a ```char *``` (a string)
- The array of those words is a ```char **``` (a pointer to a string pointer)

This is why ```ft_split()``` returns a double pointer:  
you need both the **structure** (the array) and the **content** (each word) to be accessible.
Just like ```argv[]``` in ```main()```, you can iterate over the array until you hit ```NULL```, which marks the end.
Since ```argv``` is also a ```char **```, you can access each word using ```argv[i]``` and each character with ```argv[i][j]```.

# ft_strmapi()

## Overview

```ft_strmapi()``` builds a new string by applying a function ```f()``` to each character of an input string ```s```.
The function receives both the character and its index in the string: so it can make changes based on **position** and **content**.
It returns a newly allocated string containing the results of each transformation.

---

## Prototype

```char *ft_strmapi(char const *s, char (*f)(unsigned int, char));```

---

## How to Visualize It

Input:

```
[s] [p] [a] [c] [e]
```

If your function capitalizes even indices:

```
Index: 0 1 2 3 4
Char: 's' 'p' 'a' 'c' 'e'
Output: 'S' 'p' 'A' 'c' 'E'
```

New string:

```
[S] [p] [A] [c] [E] [\0]
```

---

## Key Concepts

- Iterates over each character of ```s```
- Applies the function ```f(i, s[i])```:
  - ```i``` = the index
  - ```s[i]``` = the character
- Stores the result in a new string
- Uses ```malloc()``` to allocate memory
- Returns ```NULL``` if allocation fails

---

## Analogy

Imagine a synth processing a sequence of notes:

- Each character from ```s``` is a musical note entering the synth.
- The function ```f()``` acts like a programmable filter: it receives the note and its time step (index).
- Depending on the index and note, it alters the sound: changes pitch, applies modulation or adds an effect.
- The processed notes are recorded in sequence into a new track.

---

## When To Use It?

- You need to transform a string based on both character and position
- You want to avoid modifying the original string
- You're implementing your own version of ```map()``` for strings

---

## Downsides of ft_strmapi()

- No early stopping, every character is processed

---

## Example


```// Prototype: char *ft_strmapi(char const *s, char (*f)(unsigned int, char));```

```
#include <stdio.h>
#include <ctype.h>

char toggle_case(unsigned int i, char c)
{
    if (i % 2 == 0)
        return toupper(c);
    return tolower(c);
}

int main(void)
{
    char *original = "space";
    char *mapped = ft_strmapi(original, toggle_case);

    if (mapped)
        printf("%s\n", mapped);  // Output: SpAcE

    free(mapped);
    return 0;
}
```

# ft_striteri()

## Overview

```ft_striteri()``` iterates over a string and applies a function to each character **in-place**.
Unlike ```ft_strmapi()```, which builds a new string, this function modifies the original one by passing each character's **address** and its **index** to the callback.
It’s useful when you want to apply transformations directly to an existing string.

---

## Prototype

```void ft_striteri(char *s, void (*f)(unsigned int, char *));```

---

## How to Visualize It

Imagine each character in a string is a switch on a control panel:

```
[s] [p] [a] [c] [e]
 0   1   2   3   4
```

Your function ```f()``` visits each one and can flip it, label it, or reset it — because you’re handed a pointer to each character.

For example, you might:
- Capitalize even-indexed characters
- Replace all vowels
- Increment ASCII values

And the changes will affect the original panel.

---

## Key Concepts

- Applies a function ```f(index, &char)``` to every character in ```s```.
- The callback function gets:
  - The index of the character
  - A pointer to the character (so it can modify it)
- Operates **in-place** (modifies the original string).
- No return value.

---

## Analogy

You're controlling an RGB LED strip using an Arduino.

- Each LED holds a color made up of 3 bytes (RGB: 255 255 255)
- A function walks through the strip, one LED at a time.
- It receives both the LED's position and direct access to its color bytes.
- Each LED can be modified in place, turning red, dimming, flashing, etc... based on its index or current color.

---

## When To Use It?

- You want to apply character transformations *in-place*.
- You’re not building a new string, just adjusting an existing one.
- You need both the character and its index to apply logic.

---

## Downsides of ft_striteri()

- Not suitable for `const char *` input. The string must be mutable.
- Offers no way to stop midway. The function `f()` is applied to all characters.

## Example

```// Prototype: void ft_striteri(char *s, void (*f)(unsigned int, char *));```

```
#include <stdio.h>
#include <ctype.h>

void to_upper_if_even(unsigned int i, char *c)
{
	if (i % 2 == 0 && *c)
		*c = toupper(*c);
}

int main(void)
{
	char str[] = "space is the place";

	ft_striteri(str, to_upper_if_even);

	printf("%s\n", str);  // Output: "SpAcE Is ThE PlAcE"
	return 0;
}
```
