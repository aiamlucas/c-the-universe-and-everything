# ft_printf – Theory and Prerequisites

> A theory-first guide to help you understand some key concepts behind making ft_printf.

---

## Table of Contents

- [Overview](#overview)
- [Variadic Functions and the Ellipsis ---](#variadic-functions-and-the-ellipsis-)
- [Macros and Preprocessor Directives](#macros-and-preprocessor-directives)
- [Parsing and Format Strings](#parsing-and-format-strings)
- [Intro to Number Bases](#intro-to-number-bases)
- [Octal Hex Binary – Visualizing Other Bases](#octal-hex-binary--visualizing-other-bases)
- [Base Conversion Logic](#base-conversion-logic)
- [State Tracking and Recursion](#state-tracking-and-recursion)
- [Unit Testing Basics](#unit-testing-basics)

---

## Overview

To implement `ft_printf`, you'll need to understand and apply several foundational C concepts.

- **Variadic Functions and the Ellipsis (`...`)**  
  Enables functions like `printf()` to accept a variable number of arguments.

- **`<stdarg.h>` Macros**  
  Tools to access the extra arguments in a variadic function:
  - ```va_list```: holds the state of the argument parser
  - ```va_start```: initializes the parser
  - ```va_arg```: retrieves the next argument
  - ```va_end```: cleans up

- **Preprocessor Macros and Directives**  
  Understanding ```#define```, ```#include```, and how macros expand during compilation.

- **Parsing Format Strings**  
  Extracting ```%d```, ```%s```, etc., from a raw string and mapping them to argument types.

- **Number Bases**  
  Converting integers to:
  - Binary (base 2)
  - Decimal (base 10)
  - Hexadecimal (base 16)
  - Octal (base 8)

- **State Tracking and Recursion**  
  Keeping track of how many characters were printed, using counters and return values, and sometimes recursion to print digits in reverse order.

- **Unit Testing Basics**  
  Writing simple tests to compare ```ft_printf()``` and the standard ```printf()``` to ensure output and return values match.


## Variadic Functions and the Ellipsis (`...`)

C allows certain functions to accept a variable number of arguments. These are called **variadic functions**.

You’ve seen this in action with functions like ```printf()``` or ```exit()``` they take a known number of fixed arguments, followed by a variable list of extra values.

This is enabled by the ellipsis: ```...```

---

### Example

```
#include <stdarg.h>
#include <stdio.h>

void log_values(const char *label, int count, ...)
{
    va_list args;                  // Declare a variable to hold the argument list
    va_start(args, count);        // Initialize args to retrieve arguments after 'count'

    printf("%s: ", label);        // Print the label before listing values

    int i = 0;
    while (i < count)
    {
        int value = va_arg(args, int);  // Get the next int argument
        printf("%d ", value);           // Print the value
        i++;
    }

    va_end(args);                // Clean up the va_list when done
    printf("\n");
}

int main(void)
{
    // Calling the function with different sets of values
    log_values("Set A", 3, 10, 20, 30);  // label = "Set A", count = 3, values = 10, 20, 30
    log_values("Set B", 2, 42, 84);      // label = "Set B", count = 2, values = 42, 84
    return 0;
}
```

Output:

```
Set A: 10 20 30 
Set B: 42 84
```

---

### Key Concepts

- ```...``` marks where variable arguments begin.
- ```va_list``` is the type used to iterate through the variable arguments.
- ```va_start``` initializes the argument list.
- ```va_arg``` fetches each argument.
- ```va_end``` cleans everything up when done.

---

### Requirements

When using ```...```, you must pass at least one named argument before it (like ```count``` above) so ```va_start``` knows where to begin scanning.

---

### Where This Matters in ```ft_printf()```

In your own ```ft_printf()```, you’ll need to:

- Accept a variable number of arguments
- Parse the format string to figure out how many arguments to pull
- Call ```va_arg()``` correctly with the right expected types


## Macros and Preprocessor Directives

In C, macros and preprocessor directives are instructions that run **before compilation**. They don’t produce code by themselves, but **transform your source** before the compiler sees it.

---

### What Is a Macro?

A macro is a **code snippet with a name**, defined using ```#define```.  
When the preprocessor sees that name, it **replaces it with the code** it represents.

---

### Example: Constant Macro

```
#define PI 3.14159

int main(void)
{
    double area = PI * 2 * 2;  // becomes: 3.14159 * 2 * 2
    return 0;
}
```

Macros can make code easier to read and update. If you change PI, the whole program updates.

---

### Example: Function-like Macro

```
#define SQUARE(x) ((x) * (x))

int main(void)
{
    int result = SQUARE(3 + 1); // becomes ((3 + 1) * (3 + 1)) → 16
    return 0;
}
```

Always wrap arguments and the whole macro to avoid bugs with operator precedence.

---

### Preprocessor Directives

Directives are commands to the preprocessor. They **don’t end with semicolons**.

- ```#define``` → define macros  
- ```#include``` → add code from another file  
- ```#ifndef / #define / #endif``` → conditional compilation (e.g., header guards)

---

### Macros in Variadic Functions

```<stdarg.h>``` uses macros to let you work with ```...``` arguments.

```
#include <stdarg.h>

va_list ap;              // macro-defined type
va_start(ap, last_arg);  // expands to compiler instructions
int n = va_arg(ap, int); // gets the next argument
va_end(ap);              // clean up
```

This lets you write flexible functions like ```printf()``` — accepting any number of arguments of any type (as long as you handle them properly).

---

### Summary

- Macros are **text replacements** handled before compilation.  
- Preprocessor directives shape the final code that gets compiled.  
- Variadic argument handling is built entirely on **macro magic**.  
- They can be powerful, or dangerous if used without care.

## Parsing and Format Strings

### What Is a Format String?

A **format string** is a string that contains both:
- **Fixed text** — printed or used as-is
- **Placeholders** — symbols that will be replaced by dynamic content

```
"Hello %s, your score is %d%%"
```

In this example:
- `%s` is a placeholder for a string
- `%d` is a placeholder for an integer
- `%%` is a way to print a literal `%`

---

### What Is Parsing?

**Parsing** is the act of analyzing structured data (like a format string) and breaking it down into components you can use.

Parsing lets programs:
- Extract values
- Make decisions based on structure
- Process commands, input, files, or even source code

---

### Key Concepts

- **Literal vs. dynamic content**  
  Parsing helps differentiate text to keep vs. text to process.

- **Tokenization**  
  Turning a long string into smaller meaningful pieces (tokens).

- **Control flow**  
  Decisions based on what kind of token was found (`%`, letter, digit, etc).

---

### General Parsing Flow

1. Loop through each character in the input
2. Check for special symbols (like `%`)
3. If special: read ahead and extract meaningful info (`%04d`)
4. Store or process what you found
5. Repeat until the end

---

### When to Use Parsing

- Reading config files or JSON
- Command-line input interpretation
- Lexers and compilers

Parsing is everywhere once you start building real tools or languages.

---

### Parsing in `ft_printf`

In `ft_printf`, parsing is used to:
- Identify format specifiers (`%c`, `%s`, `%d`, etc.)
- Interpret flags (like `-`, `+`, or `0`)
- Determine width and precision
- Match values from `va_list` to the right specifiers

---

## Intro to Number Bases

Different number bases are used in computing to represent and manipulate data more efficiently and meaningfully.

---

### Binary (Base 2)

Each digit (bit) is either `0` or `1`.  
Each position is a power of 2, increasing from right to left.

```
Binary:  1  1  0  0  1  0  1  0
Place:  128 64 32 16  8  4  2  1

Calculation:
(1×128) + (1×64) + (0×32) + (0×16) + (1×8) + (0×4) + (1×2) + (0×1)
= 128 + 64 + 8 + 2 = 202

So,
Binary: 11001010 = Decimal: 202
```

---

### Octal (Base 8)

Octal uses digits from `0` to `7`.  
Each position is a power of `8`.

```
Octal:  1   0   0
Place:  64  8   1

= (1×64) + (0×8) + (0×1) = 64
So,
Decimal: 64 = Octal: 100
```

#### Why Octal?

- Historically used in UNIX (e.g., `chmod 755`).
- Each **3 binary digits = 1 octal digit**, making conversion from binary easier.

```
Binary:  111 101 101 = Octal: 7 5 5
        └──┘└──┘└──┘
         |   |   └── others
         |   └────── group
         └────────── each
```

It provides a **compact** and **human-friendly** way to represent binary data.

---

### Decimal (Base 10)

What we use daily. Digits from `0` to `9`.  
Each place is a power of `10`.

Nothing to explain here — but in programming, we often **convert from decimal to other bases** to inspect memory or encode data.

---

### Hexadecimal (Base 16)

Digits: `0`–`9`, then `A`–`F` (for 10–15).  
Each digit = 4 bits (nibble).

```
Binary:      1010 1111
Hex:           A    F

So:
Binary: 10101111 = Hex: AF
```

#### Why Hex?

- **Compact representation** of binary (1 hex digit = 4 binary bits)
- Common in debugging (e.g., memory addresses: `0x7fffd12a`)
- Easy to convert to/from binary:
  - 4 binary bits = 1 hex digit
  - 8-bit byte = 2 hex digits

---

### Key Takeaways

- Binary is the base of all data in C and other systems languages.
- Octal is historically important and groups neatly into 3 binary digits.
- Hex is the most common for debugging and memory inspection.
- Understanding these bases helps when working with:
  - Bitwise operations
  - Permissions
  - Memory dumps
  - Manual parsing and format specifiers (`%x`, `%o`, `%b`, etc.)

---

## Base Conversion Logic

Once you understand number bases (binary, octal, hex), the next step is learning how to convert integers from decimal to those bases — **manually**, in C code.

This is useful for implementing `%x`, `%u`, `%o`, etc., in your `ft_printf`.

---

### Concept

Base conversion means expressing a number using a different base (radix).

#### Example: Decimal → Hexadecimal

```
Decimal: 255

Divide by 16:
255 ÷ 16 = 15 remainder 15 → F

15 ÷ 16 = 0 remainder 15 → F

Result: FF
So, 255 (decimal) = FF (hex)
```

---

### General Algorithm

To convert a number `n` to base `b`:

1. Repeatedly divide `n` by `b`
2. Store the remainders
3. Reverse the remainders to get the digits

```
Example: Decimal 42 → Binary (base 2)

42 ÷ 2 = 21 → remainder 0  
21 ÷ 2 = 10 → remainder 1  
10 ÷ 2 = 5  → remainder 0  
5  ÷ 2 = 2  → remainder 1  
2  ÷ 2 = 1  → remainder 0  
1  ÷ 2 = 0  → remainder 1

Reverse: 101010
So, 42 (decimal) = 101010 (binary)
```

---

### When To Use It

- Printing unsigned integers (`%u`)
- Hexadecimal output (`%x`, `%X`)
- Octal output (`%o`)
- Binary debug output (if implemented)

---

## State Tracking and Recursion

When implementing a function like `ft_printf`, you need to **track how many characters** have been printed.

This is important because `printf()` returns the **total number of characters written**.

---

### State Tracking

You typically use an integer variable, like `count`, and update it every time you:

- Print a character
- Print a string
- Print a number

Example:

```
count += write(1, "a", 1);  // Adds 1
count += ft_putstr("space"); // Adds 5
```

---

### Recursion

You can use recursion for number printing — especially for base conversions.

```
void putnbr(unsigned int n)
{
    if (n >= 10)
        putnbr(n / 10);
    ft_putchar(n % 10 + '0');
}
```

This prints each digit in the correct order (not reversed).

---

### When To Use It

- When printing numbers one digit at a time
- When converting numbers to different bases
- When keeping track of how much output was produced

---

## Unit Testing Basics

You’ll need to **test your `ft_printf()`** against the real `printf()` to ensure your output matches — including return values.

---

### Minimal Test Structure

```
#include <stdio.h>

int main(void)
{
    int x = 42;
    int mine = ft_printf("My val: %d\n", x);
    int real = printf("Real val: %d\n", x);

    printf("Mine: %d, Real: %d\n", mine, real);
    return 0;
}
```

---

### What To Compare

- Output on screen (visually or captured via redirection)
- Return values (number of characters printed)

---

### Tips

- Start with simple inputs: integers, characters, strings
- Add special cases later: null pointers, edge numbers
- Write reusable test cases and compare output side by side

---

### Tools

- Use `diff` on file outputs:
  ```
  ./a.out > mine.txt
  ./original > real.txt
  diff mine.txt real.txt
  ```

- Consider colorizing your output to make mismatches obvious

---

### Summary

Testing your code is as important as writing it.  
Use `printf()` itself as a reference — it’s your best validator.


## Unit Testing Basics

Unit testing is a development practice where **individual pieces of code** (usually functions) are tested in isolation to confirm they work as expected.

But unit testing is just one type of software testing.  

---

### What Is a Unit Test?

A **unit** is the smallest testable part of a program, typically a function.

A unit test:
- Calls the function with **specific inputs**
- Checks that the **output matches expectations**
- Reports **pass or fail**

---

### Example: Unit Test for `ft_strlen`

```
int result = ft_strlen("galaxy");
if (result != 6)
    printf("Test failed: expected 6, got %d\n", result);
else
    printf("Test passed\n");
```

---

### Why Unit Testing?

- Find bugs early
- Validate logic before full integration
- Make code easier to refactor with confidence
- Encourage modular design

---

### Other Types of Software Testing

| Test Type           | What It Tests                             | Example Use Case                                   |
|---------------------|-------------------------------------------|----------------------------------------------------|
| **Unit Test**       | One function in isolation                 | `ft_itoa(42)` returns `"42"`                       |
| **Integration Test**| Multiple components working together      | `ft_printf()` calling `ft_itoa()`, `write()`, etc. |
| **System Test**     | Entire program or feature end-to-end      | Running the full printf binary with various args   |
| **Regression Test** | Prevents old bugs from coming back        | Re-testing a case that previously caused a crash   |
| **Acceptance Test** | Validates user requirements are met       | Does the full `ft_printf` spec behave correctly?   |
| **Stress Test**     | Program behavior under extreme conditions | Printing very long strings or large buffers        |

You don’t need to write all of these in every project, but knowing what they are helps you think like a systems programmer.

---

### Where to Put Unit Tests?

Many projects organize tests into a separate folder (e.g. `/tests`) and run them via a custom `main()`.

Example file: `tests/test_printf.c`

---

### Simple `ft_printf` Unit Test Example

```
#include <stdio.h>
#include "ft_printf.h" // Your own printf

int main(void)
{
    int ret1, ret2;

    ret1 = ft_printf("Hello %s\n", "galaxy");
    ret2 = printf("Hello %s\n", "galaxy");

    printf("Return values: ft_printf = %d, printf = %d\n", ret1, ret2);

    ret1 = ft_printf("Hex: %x\n", 255);
    ret2 = printf("Hex: %x\n", 255);

    printf("Return values: ft_printf = %d, printf = %d\n", ret1, ret2);

    ret1 = ft_printf("Char: %c\n", 'Z');
    ret2 = printf("Char: %c\n", 'Z');

    printf("Return values: ft_printf = %d, printf = %d\n", ret1, ret2);

    return 0;
}
```

This approach uses the real `printf()` as a **baseline** for comparing output and return values.

---

### Tips for Testing

- Always test **typical inputs** and a few **edge cases**
- Don’t test everything at once — isolate parts
- If you fix a bug, **write a test for it**
- Use tools like `diff`, `valgrind`, or even your own macros for reporting

---

### Summary

- Unit tests focus on one function, giving you tight feedback loops
- Integration and system tests ensure that parts work **together**
- Even in small projects, testing helps avoid regressions and improve code quality
