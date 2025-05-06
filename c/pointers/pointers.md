# Pointers in C 
---

# Introduction and Basics

## What is a Pointer?

A **pointer** is a variable that stores the **memory address** of another variable. Pointers are powerful tools in C programming that allow direct memory access and efficient data manipulation.

## Address-of and Dereference Operators

- `&` (Address-of): Used to obtain the address of a variable.
- `*` (Dereference/Indirection): Used to access the value stored at a memory address.

### Example

```
int number = 10;
int *ptr = &number;   // ptr holds the address of number
int result = *ptr;    // result becomes 10 by dereferencing ptr
```

## Printing Addresses and Sizes

To print the address stored in a pointer:

```
printf("Address of number: %p\n", (void*)ptr);
```

To print the address of the pointer variable itself:

```
printf("Address of pointer ptr: %p\n", (void*)&ptr);
```

To get the size of a pointer:

```
printf("Size of pointer: %zu bytes\n", sizeof(ptr));
```

## Complete Example

```
#include <stdio.h>

int main(void) {
    int number = 12;
    int *ptr = NULL;
    ptr = &number;

    printf("Value pointed by ptr: %d\n", *ptr);
    printf("Address stored in ptr: %p\n", (void *)ptr);
    printf("Address of number: %p\n", (void *)&number);
    printf("Address of the actual pointer ptr: %p\n", (void *)&ptr);
    printf("Size of ptr: %zu bytes\n", sizeof(ptr));

    return 0;
}
```

## Output

```
Value pointed by ptr: 12
Address stored in ptr: 0x7ffe0dc685fc
Address of number: 0x7ffe0dc685fc
Address of the actual pointer ptr: 0x7ffe0dc68600
Size of ptr: 8 bytes
```

## Basic Pointer Operations

### Assigning an Address to a Pointer
```
ptr = &number;
```

### Dereferencing a Pointer
```
*ptr = 10; // Sets the value pointed to by ptr
```

### Getting Address of a Pointer
```
&ptr
```

### Pointer Arithmetic

```
ptr++    // Move to next memory location
ptr--    // Move to previous memory location
ptr + 1  // Address of next element
ptr - 1  // Address of previous element
```

> Always ensure your pointer arithmetic stays within valid memory bounds.

## Dereferencing a Pointer Safely

Before dereferencing a pointer, always check if it is not NULL:

```
if (ptr != NULL) {
    printf("Value: %d\n", *ptr);
}
```

## Pointers and Constants

### Pointer to Constant
```
const int *ptr;
int value = 10;
ptr = &value;
// *ptr = 20; // Error: read-only location
```

You can still modify the variable directly:

```
value = 20;
printf("%d\n", *ptr); // Prints 20
```

### Constant Pointer
```
int value = 10;
int *const ptr = &value;
*ptr = 20; // OK
// ptr = &another_value; // Error
```

### Constant Pointer to Constant
```
int value = 10;
const int *const ptr = &value;
// *ptr = 20; // Error
// ptr = &another_value; // Error
```

## Void Pointers

- A **void pointer** can hold the address of any data type.
- You must cast a void pointer before dereferencing.

### Example
```
void *vptr;
int x = 5;
vptr = &x;
printf("%d\n", *(int *)vptr);
```

## Summary

- Pointers store addresses, not values.
- Use `*` to dereference, `&` to take addresses.
- Pointer arithmetic requires caution.
- `const` with pointers controls mutability.
- Void pointers are generic but must be cast.


# Pointers and Arrays

## Relationship Between Pointers and Arrays

An **array** in C is a contiguous block of memory made up of elements of the same type. For example, an array of integers holds multiple `int` values placed one after another in memory.

In many expressions, the name of an array **decays** into a pointer to its first element. This means that writing `arr` is often treated the same as writing `&arr[0]`.

Because of this behavior, arrays and pointers can be used interchangeably in several scenarios, such as accessing elements with indexing or pointer arithmetic.

However, arrays and pointers are **not the same**:
- An array has a fixed size and address determined at compile time.
- A pointer is a variable that can point to any memory location and can be reassigned.

Understanding both their connection and their differences is key to writing effective C programs.
In C, the name of an array acts like a pointer to its first element. This close relationship allows pointers and arrays to be used interchangeably in many scenarios.

### Array Declaration

```
int arr[5] = {10, 20, 30, 40, 50};
int *ptr = arr; // ptr points to the first element of arr
```

This is equivalent to:

```
int *ptr = &arr[0];
```

## Accessing Array Elements with Pointers

You can access elements using either array indexing or pointer arithmetic.

### Using Array Indexing
```
printf("%d\n", arr[2]);
```

### Using Pointer Arithmetic
```
printf("%d\n", *(ptr + 2));
```

Both statements above access the third element of the array.

---

## Traversing Arrays with Pointers

Using `while` loop and pointer arithmetic:

```
#include <stdio.h>

int main() {
    int arr[5] = {10, 20, 30, 40, 50};
    int *ptr = arr;
    int i = 0;

    while (i < 5) {
        printf("arr[%d] = %d\n", i, *(ptr + i));
        i++;
    }

    return 0;
}
```

---

## Modifying Array Elements via Pointers

```
*(ptr + 1) = 25; // Changes arr[1] to 25
ptr[2] = 35;     // Changes arr[2] to 35
```

---

## Pointer Arithmetic Details

When performing pointer arithmetic:

- `ptr + 1` moves the pointer to the next element of the type it points to.
- If `ptr` is an `int *`, `ptr + 1` increases the address by `sizeof(int)`.

Example:

```
int arr[3] = {1, 2, 3};
int *ptr = arr;

printf("%p\n", (void*)ptr);       // address of arr[0]
printf("%p\n", (void*)(ptr + 1)); // address of arr[1]
```

---

## Arrays as Function Arguments

When arrays are passed to functions, they decay to pointers.

This means the function does not receive the full array, just the memory address of its first element.

As a result, the following two function signatures are equivalent:

```
int sum(int *arr, int size);
int sum(int arr[], int size); // Treated as int *arr
```
### Example: Sum of Elements

```
#include <stdio.h>

int sum(int *arr, int size) {
    int total = 0;
    int i = 0;
    while (i < size) {
        total += arr[i];
        i++;
    }
    return total;
}

int main() {
    int data[5] = {1, 2, 3, 4, 5};
    printf("Sum = %d\n", sum(data, 5));
    return 0;
}
```

---

## Pointer to an Array

You can also declare a pointer to an entire array.

```
int arr[5];
int (*ptr)[5] = &arr; // pointer to an array of 5 integers
```

Access elements via:

```
(*ptr)[0] = 10;
```

---

## Multidimensional Arrays and Pointers (Preview)

Although this will be covered in depth later, here's a glimpse:

```
int matrix[2][3] = {
    {1, 2, 3},
    {4, 5, 6}
};
int *ptr = &matrix[0][0]; // Flat pointer
```

You can traverse the matrix using pointer arithmetic.

---

## Common Pitfalls

- Do not go out of bounds with pointer arithmetic.
- Ensure that pointer values are valid before dereferencing.
- Remember: `arr[i]` is equivalent to `*(arr + i)`.

---

## Summary

- Arrays and pointers are tightly related in C.
- Pointer arithmetic enables flexible traversal and modification of arrays.
- You can use pointers as function parameters to avoid array copying.
- Pointer to array and multidimensional array pointers offer more control over memory.


# Pointers and Strings

## Strings in C

A string in C is an array of characters terminated by a null character (`\0`).

### Declaration Methods

```
char str1[] = "Hello";       // Array of characters
char *str2 = "World";        // Pointer to string literal
```

- `str1` is mutable (can be modified).
- `str2` points to a string literal, which is typically stored in read-only memory (should not be modified).

---

## Accessing Characters in Strings

```
char name[] = "Alice";
int i = 0;

while (name[i] != '\0') {
    printf("Character at index %d: %c\n", i, name[i]);
    i++;
}
```

Or using a pointer:

```
char *ptr = name;

while (*ptr != '\0') {
    printf("%c\n", *ptr);
    ptr++;
}
```

---

## String Copy Function (Pointer Version)

```
void copy_string(char *dest, const char *src) {
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
}

int main() {
    char source[] = "C programming";
    char destination[50];

    copy_string(destination, source);
    printf("Copied: %s\n", destination);
    return 0;
}
```

---

## Comparing `char[]` and `char *`

### `char[]` (Character Array)
- Declares an array in stack memory.
- Modifiable.

```
char str[] = "change me";
str[0] = 'C'; // OK
```

### `char *` (Pointer to Literal)
- Points to a string literal in read-only memory.
- Modifying it leads to undefined behavior.

```
char *str = "immutable";
// str[0] = 'I'; // Error (do not modify literals)
```

---

## Using Standard Library String Functions with Pointers

```
#include <stdio.h>
#include <string.h>

int main() {
    char str1[] = "hello";
    char str2[] = "world";

    printf("Length of str1: %zu\n", strlen(str1));
    printf("Copying str2 to str1...\n");
    strcpy(str1, str2);
    printf("Now str1 is: %s\n", str1);
    return 0;
}
```

---

## Pointer-based String Traversal

```
char *s = "Pointers";

while (*s) {
    printf("%c\n", *s);
    s++;
}
```

---

## Summary

- Strings in C are character arrays terminated by `\0`.
- You can manipulate strings using pointers or indices.
- `char[]` is mutable; `char *` to literals should not be modified.
- Pointer-based string operations are efficient and flexible.
- Functions like `strlen`, `strcpy`, and `strcmp` work with string pointers.

# Functions and Pointers

## Passing by Value vs Passing by Reference

In C, all function arguments are passed by value. To allow a function to modify a variable, you pass the address (a pointer) instead.

### Passing by Value Example

```
void setZero(int x) {
    x = 0;
}

int main() {
    int a = 10;
    setZero(a);
    printf("a = %d\n", a); // Output: a = 10
    return 0;
}
```

### Passing by Reference Example

```
void setZero(int *x) {
    *x = 0;
}

int main() {
    int a = 10;
    setZero(&a);
    printf("a = %d\n", a); // Output: a = 0
    return 0;
}
```

---

## Modifying Multiple Variables with Pointers

### Swap Function

```
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int main() {
    int x = 5, y = 10;
    swap(&x, &y);
    printf("x = %d, y = %d\n", x, y); // x = 10, y = 5
    return 0;
}
```

---

## `const` Pointer Parameters

Use `const` to prevent the function from modifying the value being pointed to.

```
void printValue(const int *ptr) {
    printf("Value: %d\n", *ptr);
}

int main() {
    int x = 42;
    printValue(&x);
    return 0;
}
```

This ensures safety when the function should not alter the data.

---

## Returning Pointers from Functions

### Valid Example (pointer to static/local static variable)

```
int *getStaticInt() {
    static int val = 100;
    return &val;
}

int main() {
    int *p = getStaticInt();
    printf("%d\n", *p);
    return 0;
}
```

### Invalid Example (pointer to local variable)

```
int *badFunction() {
    int x = 10;
    return &x; // Dangerous: x goes out of scope
}
```

---

## Pointers to Functions

A function pointer can store the address of a function, allowing dynamic selection of functions.

### Declaration
```
return_type (*pointer_name)(parameter_types);
```

### Example

```
#include <stdio.h>

void greet() {
    printf("Hello!\n");
}

int main() {
    void (*func_ptr)() = greet;
    func_ptr(); // Calls greet
    return 0;
}
```

---

## Callback Functions

Passing a function pointer to another function is a common C pattern, useful for sorting, comparison, or dynamic behavior.

### Example

```
#include <stdio.h>

void execute(void (*func)()) {
    func();
}

void sayHi() {
    printf("Hi there!\n");
}

int main() {
    execute(sayHi); // pass sayHi as a callback
    return 0;
}
```

---

## Summary

- Use pointers to modify variables inside functions.
- Use `const` to enforce immutability of data.
- Avoid returning addresses of local variables.
- Function pointers enable flexible function selection and callbacks.

# Dynamic Memory Allocation

## Introduction

Dynamic memory allocation allows you to request memory during runtime, giving you control over how much and when memory is used. This is essential for creating flexible and efficient programs in C.

---

## Stack vs Heap Memory

| Feature       | Stack                       | Heap                             |
|---------------|------------------------------|----------------------------------|
| Allocation    | Static (at compile time)     | Dynamic (at runtime)            |
| Management    | Automatic (by compiler)      | Manual (by programmer)          |
| Size          | Limited, predefined          | Typically larger and flexible   |
| Lifetime      | Function scope (short-term)  | Until explicitly freed          |

- Stack: used for function calls, local variables.
- Heap: used for dynamic memory allocation via `malloc`, `calloc`, etc.

---

## Functions for Dynamic Memory Allocation

### `malloc`

Allocates memory block of given size (in bytes).

```
#include <stdlib.h>

int *ptr = (int *)malloc(sizeof(int) * 10); // allocate array of 10 ints
if (ptr == NULL) {
    // handle allocation failure
}
```

### `calloc`

Like `malloc`, but initializes memory to zero.

```
int *ptr = (int *)calloc(10, sizeof(int)); // array of 10 zero-initialized ints
if (ptr == NULL) {
    // handle allocation failure
}
```

### `realloc`

Resizes a previously allocated memory block.

```
int *new_ptr = (int *)realloc(ptr, sizeof(int) * 20); // resize to 20 ints
if (new_ptr == NULL) {
    // handle reallocation failure
}
ptr = new_ptr;
```

### `free`

Releases previously allocated memory.

```
free(ptr);
ptr = NULL; // avoid dangling pointer
```

---

## Example: Allocating and Using Dynamic Array

```
#include <stdio.h>
#include <stdlib.h>

int main() {
    int n = 5;
    int *arr = (int *)malloc(n * sizeof(int));

    if (arr == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    int i = 0;
    while (i < n) {
        arr[i] = (i + 1) * 10;
        i++;
    }

    i = 0;
    while (i < n) {
        printf("arr[%d] = %d\n", i, arr[i]);
        i++;
    }

    free(arr);
    arr = NULL;
    return 0;
}
```

---

## Memory Leaks and Best Practices

### Memory Leak
Occurs when allocated memory is never freed, leading to wasted resources.

### Preventing Leaks
- Always `free` allocated memory.
- Set pointers to `NULL` after freeing.
- Avoid losing references to allocated memory.

### Dangling Pointer
Pointer that refers to freed memory.

```
int *p = (int *)malloc(sizeof(int));
free(p);
// *p = 10; // !!!  undefined behavior (dangling)
p = NULL; //  safe
```

---

## Summary

- Dynamic memory is managed using `malloc`, `calloc`, `realloc`, and `free`.
- Memory must be freed manually to avoid leaks.
- Use `NULL` to prevent dangling pointer issues.
- Prefer `calloc` when zero-initialization is needed.


# Other Concepts and Best Practices

## Pointers to Pointers

A pointer to a pointer is a variable that stores the address of another pointer.

### Declaration
```
int **ptr;
```

### Example
```
int value = 5;
int *p = &value;
int **pp = &p;

printf("Value: %d\n", **pp);
```

---

## Arrays of Pointers

You can create arrays where each element is a pointer.

### Example
```
char *fruits[] = {"Apple", "Banana", "Cherry"};

int i = 0;
while (i < 3) {
    printf("%s\n", fruits[i]);
    i++;
}
```

---

## Pointer to an Array vs Array of Pointers

### Pointer to an Array
```
int arr[5];
int (*ptr)[5] = &arr;
```

### Array of Pointers
```
int *ptrs[5]; // array of 5 int pointers
```

---

## Pointers and Structures

You can access structure members via pointers.

### Example
```
struct Point {
    int x, y;
};

struct Point p = {3, 4};
struct Point *ptr = &p;

printf("x = %d, y = %d\n", ptr->x, ptr->y);
```

---

## Dynamic Struct Allocation

```
struct Point {
    int x, y;
};

struct Point *p = (struct Point *)malloc(sizeof(struct Point));
p->x = 10;
p->y = 20;

printf("Point: (%d, %d)\n", p->x, p->y);

free(p);
```

---

## Buffer Overflows and Pointer Safety

- Always ensure you write within allocated bounds.
- Validate all pointer inputs.
- Initialize pointers to `NULL`.
- Avoid returning pointers to local variables.

---

## Common Mistakes

- Using uninitialized pointers
- Dereferencing NULL pointers
- Memory leaks
- Invalid memory access (dangling pointers)

---

## Best Practices

- Initialize all pointers to `NULL`.
- Use `const` to enforce read-only access when applicable.
- Always `free` allocated memory.
- Check results of `malloc` and `calloc`.
- Use pointer arithmetic cautiously and only within bounds.

---

## Final Summary

This guide has covered:
1. **Basics of Pointers** – What pointers are, how to declare and use them, and how to safely perform pointer operations.
2. **Pointers and Arrays** – The close relationship between arrays and pointers, including traversal, arithmetic, and function arguments.
3. **Pointers and Strings** – How pointers interact with character arrays and C strings, including manual string manipulation and library functions.
4. **Functions and Pointers** – Using pointers to pass values by reference, modify multiple variables, return pointers, and handle function pointers.
5. **Dynamic Memory Allocation** – Managing memory manually with `malloc`, `calloc`, `realloc`, and `free`, along with best practices to avoid leaks and errors.
6. **Other Concepts and Best Practices** – Pointers to pointers, arrays of pointers, struct pointers, and essential safety tips for writing reliable low-level code.
