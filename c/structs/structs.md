# Structs in C

### What is a Struct?

A **struct** (short for *structure*) is a user-defined data type in C that allows you to group variables of different types into a single unit. It’s ideal for representing real-world entities, like a pixel on the screen with `x` and `y` positions, a student with a name and ID, or a book with a title, author, and price.

---

### Why Use Structs?

- To group related data together.
- To make programs more modular and readable.
- To model real-world objects and data.
- To simplify data passing and manipulation in functions.

---

### Basic Syntax

```
struct StructName {
    data_type member1;
    data_type member2;
    ...
};
```

**Example:**

```
struct Pixel {
    int x;
    int y;
};
```

This defines a struct named `Pixel`, which could represent a coordinate on a screen where `x` is the horizontal position and `y` is the vertical position.

---

### Declaring and Initializing Struct Variables

You can declare a struct variable directly:

```
struct Pixel p1;
p1.x = 100;
p1.y = 200;
```

Or initialize it at the time of declaration:

```
struct Pixel p2 = {300, 400};
```

---

### Accessing Struct Members

You access struct members using the **dot operator (`.`)**:

```
printf("Pixel at (%d, %d)\n", p2.x, p2.y);
```

---

### Using Pointers with Structs

You can create a pointer to a struct:

```
struct Pixel *ptr = &p2;
```

Then use the **arrow operator (`->`)** to access members:

```
ptr->x = 500;
ptr->y = 600;
```

This is functionally the same as:

```
(*ptr).x = 500;
(*ptr).y = 600;
```

But the arrow syntax is cleaner and easier to read.

---

### Example: Struct with Functions

```
#include <stdio.h>

struct Student {
    int id;
    char name[50];
    float grade;
};

void print_student(struct Student s) {
    printf("ID: %d\n", s.id);
    printf("Name: %s\n", s.name);
    printf("Grade: %.2f\n", s.grade);
}

int main() {
    struct Student stu1 = {101, "Alice", 88.5};
    print_student(stu1);
    return 0;
}
```

---

### Typedef for Cleaner Syntax

You can use `typedef` to define an alias for a struct, so you don't have to type `struct` every time:

```
typedef struct {
    int x;
    int y;
} Pixel;
```

Now you can declare a variable like this:

```
Pixel p = {50, 75};
```

---

### Nested Structs

Structs can contain other structs:

```
struct Date {
    int day, month, year;
};

struct Student {
    int id;
    struct Date dob;  // date of birth
};
```

Access nested members using the dot operator twice:

```
struct Student s = {1001, {15, 3, 2002}};
printf("Birth year: %d\n", s.dob.year);
```

---

### Structs and Arrays

You can create an array of structs:

```
struct Pixel pixels[3] = {
    {0, 0},
    {100, 200},
    {300, 400}
};

int i = 0;
while(i < 3) {
    printf("Pixel %d: (%d, %d)\n", i, pixels[i].x, pixels[i].y);
    i++;
}
```

---

### Structs and Functions

You can pass structs to functions:

- By value: makes a copy.
- By pointer: more efficient, allows modification.

**By value:**

```
void display(struct Pixel p) {
    printf("Pixel at (%d, %d)\n", p.x, p.y);
}
```

**By pointer:**

```
void move(struct Pixel *p, int dx, int dy) {
    p->x += dx;
    p->y += dy;
}
```

---

### Memory Representation

Structs are stored in memory **sequentially**, with each member occupying space according to its type. There may be **padding** between members to align data correctly.

**Example:**

```
struct Example {
    char a;   // 1 byte
    int b;    // 4 bytes
};
```

Due to padding, this struct may take **8 bytes** in memory (1 + 3 padding + 4).

---

### Important Points to Remember

- Structs group data of different types.
- Use `.` for accessing members of a struct variable.
- Use `->` for accessing members through a pointer.
- Use `typedef` to simplify syntax.
- Padding may increase the size of a struct.
- Structs are often used with arrays and functions for organizing complex data.

---

### Applications of Structs

- Game development (positions, stats)
- Storing data records (linked lists, trees)
- Network packets, file formats, etc.

---
