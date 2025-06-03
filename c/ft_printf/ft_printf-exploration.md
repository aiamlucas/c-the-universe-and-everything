# ft_printf – Theory and Prerequisites

> A theory-first guide to help you understand some key concepts behind making ft_printf.

---

## Table of Contents

- [Overview](#overview)
- [What Is ```...``` in C?](#what-is--in-c)
- [Variadic Functions and ```<stdarg.h>```](#variadic-functions-and-stdargh)
- [Preprocessor Macros and Directives](#preprocessor-macros-and-directives)
- [Parsing Format Strings](#parsing-format-strings)
- [Intro to Number Bases](#intro-to-number-bases)
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
- They can be powerful — or dangerous — if used without care.


