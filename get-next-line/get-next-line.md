> Some previous lines before getting the next line!

# "Everything Is a File"

Understanding file descriptors is essential to working with low-level system calls in C and understanding how Linux treats input/output. This chapter dives into how Linux manages files, terminals, and communication channels using file descriptors and how you can use them, especially via the ```read()``` function.

---

## What Is a File Descriptor?

A **file descriptor (FD)** is a non-negative integer used by the operating system to reference an open file or input/output resource.

In Linux, **everything is treated as a file** not just files on disk. This includes:

- Regular files (text files, binaries, etc.)
- Directories
- Terminals (TTY)
- Pipes
- Sockets
- Devices (e.g., `/dev/null`, `/dev/tty`)
- Process communication channels

When a process opens a file (or anything treated like a file), the kernel returns an **integer ID** — the file descriptor.

---

## Standard File Descriptors

When a process starts, it automatically gets three file descriptors opened by default:

| FD  | Name           | Purpose               |
|-----|----------------|-----------------------|
| 0   | stdin          | Standard Input        |
| 1   | stdout         | Standard Output       |
| 2   | stderr         | Standard Error Output |

These can be redirected independently.

Example:

```
echo "hello" > file.txt     # stdout redirected
cat < file.txt              # stdin redirected
ls nothere 2> error.log     # stderr redirected
```

---

## Why "Everything Is a File"?

In Unix-like systems (Linux, macOS, etc.), all I/O is unified through the file interface.

You can use ```read()```, ```write()```, ```open()``` for:

- Reading a file
- Getting user input from the keyboard
- Receiving a message over a socket
- Reading from a pipe

This design gives extreme **flexibility and composability**, especially in shell scripting, system calls, and process management.

---

## Reading from a File Descriptor

### ```ssize_t read(int fd, void *buf, size_t count);```

Reads up to ```count``` bytes from the file descriptor ```fd``` into the buffer ```buf```.

- **fd**: File descriptor to read from
- **buf**: Pointer to memory where data should be stored
- **count**: Maximum number of bytes to read

### Returns:

- **> 0**: Number of bytes read
- **0**: End of file
- **-1**: Error (check ```errno```)

---

### Simple Example

```
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
    char buffer[128];
    int fd = open("notes.txt", O_RDONLY);
    if (fd == -1)
        return 1;

    ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes > 0)
    {
        buffer[bytes] = '\0';
        write(1, buffer, bytes); // Write to stdout
    }

    close(fd);
    return 0;
}
```

---

## File Descriptors and the Terminal (TTY)

Your terminal (or terminal emulator like GNOME Terminal or iTerm) is a **TTY device**, which stands for “teletype.” Even modern terminals maintain this name and act as interactive input/output devices.

- `/dev/tty` refers to the controlling terminal of a process.
- You can open and write to it like a file:
  ```echo "Hello" > /dev/tty```

When you press keys in the terminal, input is piped into ```stdin (FD 0)```.  
When you see output, it's being written via ```stdout (FD 1)```.

```
          ┌────────────────────────────┐
          │       Your Terminal        │
          │     (e.g., /dev/tty)       │
          └────────────────────────────┘
                     ▲  ▲  ▲
     Keyboard Input  │  │  │  Terminal Output
                     │  │  │
         ┌───────────┘  │  └─────────────┐
         │              │                │
     ┌───┴────┐     ┌───┴────┐      ┌─────┴─────┐
     │  stdin │     │ stdout │      │ stderr   │
     │   FD 0 │     │  FD 1  │      │   FD 2    │
     └────────┘     └────────┘      └───────────┘
            \          |               /
             \         |              /
              \        v             /
               └──→ File Descriptor Table
                        (per process)
```

---

## Processes and File Descriptors

Each **process** has its own **file descriptor table**, maintained by the kernel.  This table maps small integers (FDs) to kernel file objects.

- When a process calls ```open()```, it gets the next available file descriptor.
- When a file is closed with ```close(fd)```, that FD becomes reusable.
- File descriptors can be **duplicated** or **inherited** (via ```dup()``` or ```fork()```).

This allows for advanced features like:

- Redirection (`>`, `<`, `2>`)
- Piping (`cmd1 | cmd2`)
- Logging
- Inter-process communication

---

## How Shells Use File Descriptors

In Bash, you can create and use FDs manually:

```
exec 3> log.txt     # Create FD 3 for writing to log.txt
echo "Hello" >&3    # Write to FD 3
exec 3>&-           # Close FD 3
```

You can also duplicate FDs:

```
command 2>&1        # Redirect stderr to stdout
```

---

## Checking Open File Descriptors

You can inspect file descriptors of a process via:

```
ls -l /proc/$$/fd
```

This shows all open FDs for your current shell process.

---


# Static Variables

In C programming, `static` is one of the most versatile storage class specifiers.  
It affects **variable lifetime**, **visibility**, and **linkage**, depending on context.  

---

## 1. What Does `static` Mean?

The `static` keyword can be used in:

- Global scope (outside functions)
- Local scope (inside functions)
- Inside headers or `.c` files
- On variables or functions

In **all cases**, `static` changes **duration** (how long a variable lives) or **linkage** (visibility across files).

---

## 2. Static Local Variables

When you declare a variable inside a function with `static`, it:

- **Maintains its value** between function calls
- Is initialized **only once**
- Has **function scope**, but **lives for the entire program**

### Example

```
#include <stdio.h>

void counter(void)
{
static int count = 0; // This variable keeps its value between calls
count++;
printf("count = %d\n", count);
}

int main(void)
{
counter(); // count = 1
counter(); // count = 2
counter(); // count = 3
return 0;
}
```

Without `static`, `count` would reset to 0 on each call.

---

## 3. Static Global Variables

Declaring a global variable as `static` means:

- It’s only **visible to the file it’s defined in**
- It can’t be accessed from other `.c` files
- It’s useful for **encapsulation** in modular programs

### Example

In `module.c`:

```
static int internal_flag = 0; // Only accessible in this file

void set_flag(void) 
{
internal_flag = 1;
}
```

In another file, trying to `extern int internal_flag;` will fail.

---

## 4. Static Functions

Marking a function as `static` restricts its visibility:

- The function can **only be used in the same file**
- It’s **not linked externally**, preventing symbol clashes

### Example

```
static void hidden_function(void) 
{
// Only callable in this file
}
```

This is good practice for internal helpers in libraries or modules.

---

## 5. Static Initialization

Static variables are:

- **Zero-initialized** by default (if not explicitly set)
- Initialized **only once**
- Safe to use without uninitialized memory bugs

Example:

```
void foo(void) 
{
static int x; // Initialized to 0 automatically
x++;
}
```

---

## 6. Static vs Global

| Feature        | `static` local var     | `static` global var       | Global var (no static) |
|----------------|------------------------|---------------------------|------------------------|
| Scope          | Only within function   | Only within file          | Everywhere             |
| Lifetime       | Whole program          | Whole program             | Whole program          |
| Initial value  | 0 (unless set)         | 0 (unless set)            | 0 (unless set)         |
| Linkage        | None                   | Internal                  | External               |

---

## 7. Use Cases of Static Variables

- Preserve state in **repeated function calls** (like in parsers or iterators)
- Limit scope of **internal configuration** variables
- Avoid **naming collisions** in large codebases
- Hide **helper functions** and private APIs

---

## 8. Gotchas and Best Practices

- **Don’t overuse static** — it can make code harder to test and reason about
- Beware of **reentrancy** and **thread safety**: static local variables are shared across function calls
- Use static functions and globals to **enforce modular design**
- In libraries, **expose only what’s needed** — hide everything else with `static`

---


# Buffers – Understanding Data Storage and Flow

A **buffer** is a temporary storage area used to hold data while it's being transferred between two locations, such as from disk to memory, or from memory to the screen. Buffers play a crucial role in C programming, particularly in input/output (I/O) operations.

This guide covers what buffers are, how they work, and how they appear in your C programs.

---

## 1. What Is a Buffer?

A **buffer** is essentially a **block of memory** allocated to temporarily hold data. It helps coordinate the speed difference between devices, functions, or layers in your program.

### Why use buffers?

- Reduce the number of I/O operations (which are expensive)
- Allow smooth, chunked data processing
- Prevent data loss during transfer
- Improve efficiency

---

## 2. Common Uses of Buffers in C

- Reading from a file
- Writing to a file
- Processing data streams
- Collecting user input
- Networking (socket buffers)
- System calls like ``` read() ``` and ``` write() ```

---

## 3. Example: Buffer with ``` read() ```

```
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
int fd = open("file.txt", O_RDONLY);
if (fd == -1)
return 1;

char buffer[1024];
ssize_t bytes_read;

while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
{
    // Process data in buffer
    write(1, buffer, bytes_read);  // Output to stdout
}

close(fd);
return 0;
}
```

### What’s Happening?

- ``` buffer ``` is a local array (the buffer)
- ``` read() ``` fills it with up to 1024 bytes
- ``` write() ``` outputs that buffer to the terminal

The buffer allows you to process the file in chunks — ideal for big files or streams.

---

## 4. Buffer Size

Choosing the right buffer size is a balance:

| Size                   | Pros                              | Cons                          |
|------------------------|-----------------------------------|-------------------------------|
| Small (e.g. 32B)       | Low memory usage                  | More system calls             |
| Medium (e.g. 512B–4KB) | Good performance for general use  | May still hit slowdowns       |
| Large (e.g. >8KB)      | Fewer calls, better throughput    | Higher memory usage, latency  |

Typical default: 1024 or 4096 bytes (matches common OS page size)

---

## 5. Buffered vs Unbuffered I/O

The C Standard Library (stdio) uses **buffered I/O** functions like ``` fread() ``` and ``` fwrite() ```. These work on top of file descriptors and manage their own internal buffers.

```
FILE *f = fopen("file.txt", "r");
char line[512];
fgets(line, sizeof(line), f);
```

Here, `fgets()` uses a hidden buffer managed by `f`. You don’t call `read()` directly — the buffer is refilled automatically.

You can also control buffering behavior using:

- ``` setvbuf() ``` — manually set the buffer
- ``` fflush() ``` — flush the buffer (force write)
- ``` setbuf() ``` — simpler version of ``` setvbuf() ```

---

## 6. Manual Buffering and Chunked Processing

Buffers are essential when writing custom I/O logic, like in the `get_next_line` project.

```
#define BUFFER_SIZE 1024

char *buffer = malloc(BUFFER_SIZE + 1);
ssize_t bytes = read(fd, buffer, BUFFER_SIZE);
buffer[bytes] = '\0';
```

You'll likely:

- Append to a larger string
- Search for a newline
- Store leftover data between calls (possibly using ``` static ```)

---

## 7. Real-Life Buffer Examples

| Area                 | Example Buffer Use                             |
|----------------------|------------------------------------------------|
| Filesystem           | Page cache buffers data before read/write      |
| Networking           | TCP send/receive buffers                       |
| Video/audio streaming| Frame or sample buffering                      |
| User interfaces      | Keyboard/mouse input buffering                 |
| Terminal I/O         | Line buffering in stdin/stdout                 |

---

## 8. Types of Buffering in stdio

| Type         | Description                          | When It Happens                      |
|--------------|--------------------------------------|--------------------------------------|
| Full         | Buffer is filled before flushing     | Disk files (default)                 |
| Line         | Flushed on newline                   | Terminals                            |
| Unbuffered   | Data is written immediately          | stderr (by default), or explicitly   |

Change behavior with:

```
setvbuf(stdout, NULL, _IONBF, 0); // disable buffering
```

---

## 9. Buffer Overflow (Caution!)

A **buffer overflow** occurs when you write more data into a buffer than it can hold.

```
char buf[5];
strcpy(buf, "too long!"); // Undefined behavior!
```

Always use safe functions like ``` strncpy() ``` or carefully track lengths to avoid corruption or security issues.

---

# Summary – File Descriptors, Static Variables, and Buffers

---

## File Descriptors

- Every process has a **file descriptor table** mapping small integers (FDs) to file-like objects.
- Common descriptors:
  - **0**: `stdin` (input)
  - **1**: `stdout` (output)
  - **2**: `stderr` (error output)
- Descriptors apply not only to files, but also **terminals, sockets, pipes, devices**, etc.
- System calls like `read()`, `write()`, `open()` use these descriptors to perform I/O.

---

## Static Variables

- `static` in C **preserves variable state** across function calls and **limits scope** in multi-file programs.
- Key use cases:
  - Track state in functions like `get_next_line()`
  - Hide internal helpers in modular programs
- A static variable is **initialized once**, lives for the **entire runtime**, but may only be **accessible within a certain scope** (function or file).

---

## Buffers

- Buffers are temporary memory blocks used to store data during I/O.
- Reading/writing in **chunks** (e.g., 1024 bytes) improves performance and reduces system calls.
- Use in:
  - File I/O
  - Terminals
  - Streams and networks
- Know the difference between:
  - **Buffered I/O** (e.g., `fgets()`)
  - **Unbuffered I/O** (e.g., `read()`)
