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

