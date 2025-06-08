# Makefiles What, How and Why

> This documentation explores what Makefiles are, why they exist, and how they power the compilation process in C projects.  
> **Chapter 1: The Basics of Makefiles**  
> *(Followed later by Chapter 2: Intermediate Features and Chapter 3: Advanced Patterns)*

---

## The Basics of Makefiles

### What Is a Makefile?

A Makefile is a build automation tool that:
- Defines rules for compiling programs
- Specifies file dependencies
- Automates rebuilding only what changed
- Standardizes the build process

---

### Why Use a Makefile?

Imagine a C project with many source files:

```
cc -Wall -Wextra -Werror -c file1.c
cc -Wall -Wextra -Werror -c file2.c
cc -Wall -Wextra -Werror -c file3.c
cc file1.o file2.o file3.o -o my_program
```

Typing that manually every time is repetitive and error-prone. A Makefile automates that.

Even better: if only `file2.c` changes, `make` will rebuild **only** that file, saving time and effort.

---

### How Make Works

When you run ``` make ```, it:
1. Looks for a file named ``` Makefile ``` (or ``` makefile ```).
2. Parses its rules and dependencies.
3. Builds targets in the correct order — **only if their dependencies changed**.

---

### Basic Syntax of a Rule

A rule in a Makefile has this format:

```
target: dependencies
<TAB> command
```

- `target` → the file to build (e.g., an object file or executable)
- `dependencies` → files required to build the target
- `command` → the shell command to run (MUST start with a **TAB**, not spaces)

---

### A Simple Example

Suppose you have:

```
main.c
math.c
math.h
```

Your Makefile might look like:

```
# Makefile

my_program: main.o math.o
	cc main.o math.o -o my_program

main.o: main.c math.h
	cc -Wall -Wextra -Werror -c main.c

math.o: math.c math.h
	cc -Wall -Wextra -Werror -c math.c

clean:
	rm -f *.o my_program
```

---

### Running It

- `make` → builds `my_program` if any source file is updated.
- `make clean` → deletes compiled files.
- `make math.o` → builds just that file.

---

### Default Target

The **first rule** in the file is the default. It’s what runs when you just type:

```
make
```

That’s why `my_program` is the first rule in the example.

---

### Key Terms

|       Term        |            Meaning                                                    |
|-------------------|-----------------------------------------------------------------------|
| **Target**        | What you want to build (e.g., `main.o`, `my_program`)                 |
| **Dependency**    | What the target needs to be up to date (e.g., `.c` and `.h` files)    |
| **Command**       | What action `make` takes to build the target                          |
| **Phony Target**  | A name like `clean` that’s not a real file                            |

---

### Special Targets: `.PHONY`

When you have targets like `clean` that aren't files, declare them as phony to avoid conflicts:

```
.PHONY: clean
```

Otherwise, if a file named `clean` exists in your folder, `make clean` will do nothing (it thinks the target is already built).

---

# Intermediate Features of Makefiles

> Now that you know the basics, let’s unlock more powerful features of Makefiles:  
> reusable variables, pattern rules, implicit behaviors and better organization.

---

## Variables in Makefiles

Instead of repeating values like compiler flags or filenames, store them in variables:

```
CC      = gcc
CFLAGS  = -Wall -Wextra -Werror
NAME    = my_program
SRC     = main.c math.c
OBJ     = $(SRC:.c=.o)
```

You can then use them like this:

```
$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
```

---

### Common Variables

| Variable  | What It Means                                    |
|-----------|--------------------------------------------------|
| `CC`      | The compiler to use (e.g. cc)                    |
| `CFLAGS`  | Flags passed to the compiler                     |
| `SRC`     | Source files                                     |
| `OBJ`     | Object files                                     |
| `$@`      | The name of the current target (e.g. my_program) |
| `$<`      | The first dependency  (usually the .c file)      |
| `$^`      | All dependencies  (e.g., list of .o files)       |

---

## Pattern Rules

Pattern rules let you describe *many targets* with one rule.

```
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
```

This tells `make` how to build any `.o` file from a `.c` file with the same base name.
> $< = main.c
> $@ = main.o

It saves you from writing:

```
main.o: main.c
	cc -c main.c

math.o: math.c
	cc -c math.c
```

---

## Wildcards and Automatic File Discovery

To automatically include all `.c` files in a folder:

```
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
```

This works well for small projects where files don’t change names often.

---

## Organizing Your Makefile

Make it easier to read and maintain:

- Use **sections**: variables, compilation rules, cleaning, etc.
- Add comments.
- Group rules logically.

Example:

```
# Compiler
CC     = cc
CFLAGS = -Wall -Wextra -Werror

# Files
NAME   = ft_printf
SRC    = $(wildcard src/*.c)
OBJ    = $(SRC:.c=.o)

# Rules
all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
```

---

## Implicit Rules

`make` already knows how to compile `.c` into `.o` using the default rule:

```
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
```

If you use conventional names like `main.c`, `main.o`, you can omit these rules completely. But being explicit helps with clarity and control.

---

## Target Aliases

You can create convenience targets:

```
debug:
	$(MAKE) CFLAGS="-Wall -g"

run: $(NAME)
	./$(NAME)
```

---

# Deep Dive and Advanced Makefile Techniques for Larger Projects

> In this final chapter, we explore advanced patterns used in real-world projects,  
> including header tracking, parallel builds, modularization, and maintainability tips.

---

## Header File Dependency Tracking

If your `.c` files include `.h` headers and one of them changes, your `.o` files should recompile.  
You can handle this automatically using **dependency files**.

### Manual Header Dependency

You could hardcode it (not ideal):

```
main.o: main.c main.h utils.h
```

But for many files, this becomes hard to manage.

---

### Auto-Generated Dependencies


Many C compilers (including cc) support options for creating .d files (dependency files) that list header file dependencies.

```
%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@
```

This tells CC to:
- `-MMD`: generate a `.d` file for each .c file
- `-MP`: prevent build errors if headers are deleted

Then include them at the top of the Makefile:

```
-include $(OBJ:.o=.d)
```

This way, your build system tracks header changes automatically.

---

## Parallel Compilation (`make -j`)

On large projects, compiling all files one by one is slow.  
You can use parallel compilation with:

```
make -j
```

This speeds up builds by running jobs in parallel (using multiple CPU cores).

You can also specify how many jobs:

```
make -j4  # 4 jobs in parallel
```

**Caution:** Your Makefile must avoid race conditions.  
Always make sure each target builds independently.

---

## Splitting Your Makefile

As your project grows, the Makefile can get messy. Split it into logical components:

```
Makefile
config.mk # variables
rules.mk # compilation logic
deps.mk # dependency includes
```

Then in your main Makefile:

```
include config.mk
include rules.mk
-include deps.mk
```

This modular setup makes it easier to reuse or customize parts for different environments.

---

## Out-of-Source Builds

Instead of cluttering your source directory with `.o` files, you can build in a separate directory:

1. Create a `build/` folder
2. Compile object files into it:

```
OBJDIR = build
OBJ = $(addprefix $(OBJDIR)/, $(SRC:.c=.o))

$(OBJDIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
```

This keeps the source tree clean and separates output from logic.

---

## Handling Multiple Targets

For projects with multiple binaries or libraries, define rules per component:

```
NAME1 = app1
NAME2 = app2
SRC1  = main1.c
SRC2  = main2.c utils.c

OBJ1  = $(SRC1:.c=.o)
OBJ2  = $(SRC2:.c=.o)

all: $(NAME1) $(NAME2)

$(NAME1): $(OBJ1)
	$(CC) $(OBJ1) -o $(NAME1)

$(NAME2): $(OBJ2)
	$(CC) $(OBJ2) -o $(NAME2)
```

You can also namespace the build logic with variables or even external scripts.

---

## Logging and Debugging

Add logging output to help users and yourself:

```
@echo "Compiling $<..."
```

Or make debug modes verbose:

```
ifeq ($(DEBUG),1)
    CFLAGS += -g -DDEBUG
endif
```

Then run with:

```
make DEBUG=1
```

---

