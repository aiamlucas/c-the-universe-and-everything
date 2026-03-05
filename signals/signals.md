# Signals 

## The Kernel

The kernel is the core of the operating system. 
It's the one piece of software that has direct access to the hardware. 
Everything else runs in **userspace**: 
a restricted mode where programs cannot touch hardware, other processes memory, 
or critical system resources directly.

```
hardware
    |
kernel (ring 0 — privileged, can do anything)
    |
    +-- manages memory
    +-- manages processes (scheduling, creating, killing)
    +-- manages files and filesystems
    +-- manages devices (keyboard, disk, network)
    +-- manages signals
    |
userspace (ring 3 — restricted)
    |
    +-- your shell
    +-- your programs
    +-- system libraries (libc, readline...)
```

The CPU enforces this separation in hardware using **privilege rings**.
x86 has four rings (0-3) but modern operating systems only use two:

```
ring 0   kernel      full privileges: can execute any instruction
ring 1   (unused)
ring 2   (unused)
ring 3   userspace   restricted: cannot access hardware or kernel memory
```

Rings 1 and 2 were designed in the 1980s for OS layers but were never
adopted. Linux, Windows, and macOS all chose the simpler kernel/user split.

---

## System Calls

When your program needs something only the kernel can do, like open a file,
fork a process, write to the terminal. 
It makes a **syscall**: a controlled jump from ring 3 into ring 0.

On x86-64 the instruction is `syscall`. It switches the CPU to ring 0
and jumps to a fixed address in the kernel. The kernel reads which syscall
you wanted (a number stored in RAX), does the work, puts the result back
in RAX, and executes `sysret`, which switches back to ring 3.

```
your program (ring 3):
    mov rax, 1        <- syscall number 1 = write()
    mov rdi, 1        <- argument: fd = stdout
    mov rsi, buf      <- argument: buffer address
    mov rdx, len      <- argument: length
    syscall           <- jump to kernel (ring 0)
                            |
                     kernel executes write()
                     puts bytes count result in rax
                     sysret                    <- jump back (ring 3)
your program continues:
    result is in rax
```

Syscalls examples:
```
open(), read(), write()   -> file operations
fork(), execve()          -> process creation
malloc()                  -> eventually calls brk() or mmap()
printf()                  -> calls write()
readline()                -> calls read()
sigaction()               -> signal configuration
waitpid()                 -> wait for child process
```

---

## The Process Control Block

The kernel maintains a **process control block (PCB)** for every process,
from creation until fully cleaned up. It contains everything the kernel
needs to manage a process:

```
pid                  process ID
state                running / sleeping / stopped / zombie
registers            saved CPU state when process is not running
memory mappings      where code, stack, heap are in virtual memory
open file descriptors
signal bitmasks:
    pending          signals that arrived but not yet delivered
    blocked          signals currently blocked (sa_mask, sigprocmask)
    handlers         function pointer or SIG_DFL/SIG_IGN per signal
parent pid
priority / scheduling info
```

---

## The Terminal Driver

The terminal driver is a kernel subsystem that sits between hardware
and your programs, managing everything about how text flows in and out.

```
keyboard input:
    raw key scan codes from hardware
        |
    terminal driver
        |
        +-- translates scan codes to characters
        +-- line buffering  -> holds input until Enter (canonical mode)
        +-- echo            -> sends what you type back to the screen
        +-- backspace       -> erases last char from buffer
        +-- Ctrl+C  -> SIGINT  to foreground process group
        +-- Ctrl+Z  -> SIGTSTP (stop / suspend)
        +-- Ctrl+\  -> SIGQUIT
        +-- Ctrl+D  -> EOF
        +-- Ctrl+S/Q -> freeze / resume output (flow control)
        +-- window resize -> SIGWINCH
        |
    your program receives a clean line via read()

program output:
    your program calls write()
        |
    terminal driver
        |
        +-- translates \n to \r\n (carriage return + newline)
        +-- handles ANSI escape codes (colors, cursor movement)
        |
    pixels appear on screen
```

### Canonical mode vs raw mode

In **canonical mode** (default), the terminal driver buffers everything
until Enter. Your program only sees complete lines.

**readline** switches to **raw mode** via `tcsetattr()` so it can see
every keypress immediately and handle its own line editing, history,
tab completion, cursor movement. Without raw mode none of that is possible.

```
canonical mode:
    you type: h e l l o ← ← x y Enter
    terminal driver processes everything
    your program receives: "helxy\n"    <- only the final result

raw mode (readline):
    'h' -> append to buffer, redisplay
    'e' -> append to buffer, redisplay
    ←   -> move cursor left, redisplay
    'x' -> insert at cursor, redisplay
    Enter -> return "helxy"
```

However, **signal generation** (the `ISIG` flag) stays enabled in raw mode.
Ctrl+C still generates SIGINT. The signal layer of the terminal driver
operates independently of buffering mode.

---

## What is a Signal?

A signal is a way for the OS (or another process) to notify a process
that something happened. It is asynchronous, it can arrive at any point
during execution, between any two instructions.

```
A signal is: a flag in a kernel data structure
           + a detour in the normal return-to-userspace path
```

When you press Ctrl+C:

```
terminal driver sets bit 2 (SIGINT) in the process's pending bitmask
nothing runs immediately

pending bitmask:
[ 0 ][ 1 ][ 0 ][ 0 ][ 0 ]...
  1    2    3    4    5
       ^
       SIGINT pending
```

Next time the kernel is about to return control to your process
(after a syscall, after a timer interrupt, after being scheduled):

```
no signal pending:              signal pending:
    kernel                          kernel
      |                               |
      | return to userspace           | save full CPU state
      |                               | (RIP, RSP, all registers)
      v                               | push handler frame onto stack
    program continues                 | jump to your handler
    instruction X                     v
                                    handler runs:
                                      g_signal_received = SIGINT
                                      write("\n")
                                      rl_redisplay()
                                      |
                                      v
                                    handler returns
                                      |
                                      v
                                    kernel restores saved CPU state
                                    (RIP -> instruction X,
                                     RSP -> original stack,
                                     all registers as before)
                                      |
                                      v
                                    program resumes instruction X
                                    as if nothing happened
```

The handler runs in your process's own memory and stack. It is not a
separate process, not a kernel thread. It is your code, called by the
kernel at an unexpected moment.

### Common signals

```
SIGINT  (2)  -> Ctrl+C     -> interrupt  (default: terminate)
SIGQUIT (3)  -> Ctrl+\     -> quit       (default: terminate + core dump)
SIGTERM (15) -> kill       -> terminate  (default: terminate, can be caught)
SIGKILL (9)  -> kill -9    -> kill       (cannot be caught or ignored)
SIGHUP  (1)  -> terminal closed         (default: terminate)
SIGCHLD (17) -> child stopped or exited
SIGWINCH     -> terminal window resized
SIGTSTP      -> Ctrl+Z -> stop/suspend process
```

### Three choices for signal disposition

```
SIG_DFL   value 0 -> OS default behavior (usually terminate)
SIG_IGN   value 1 -> ignore the signal completely
handler           -> run your own function, then resume
```

### SIGKILL — cannot be caught

SIGKILL is handled entirely inside the kernel. It never reaches userspace.
The kernel removes the process from the scheduler immediately. Your handler
code is never reached, SIG_IGN does not work, there is no way to block it.

```
kill -9 pid
    |
    v
kernel marks process for immediate termination
process removed from scheduler
memory freed, file descriptors closed
your code never runs again — no handler, no cleanup, nothing
```

---

## sigaction

`sigaction()` is the syscall that installs a signal handler. It tells
the kernel "when signal X arrives, do Y".

### The struct

`struct sigaction` is defined by POSIX in <signal.h>. Not something you
write. It's part of the system API:

```
struct sigaction {
    void     (*sa_handler)(int);  // function the OS calls when signal arrives
    sigset_t   sa_mask;           // extra signals to block while handler runs
    int        sa_flags;          // behavior modifiers
};
```

`sa_handler` is a function pointer; the function receives the signal
number as argument so one handler can cover multiple signals if needed.

`sa_mask` is a bitmask of signals to block while the handler runs.
Must always be initialized with `sigemptyset()` before use:

```
before sigemptyset: [ ? ][ ? ][ ? ][ ? ]...  (uninitialized — could be anything)
after  sigemptyset: [ 0 ][ 0 ][ 0 ][ 0 ]...  (all cleared — block nothing extra)
```

Note: even with an empty mask, the triggering signal is automatically
blocked while its own handler runs to prevent recursive calls.

`sa_flags` modifies edge case behavior:

```
SA_RESTART   -> restart interrupted system calls automatically
               instead of returning EINTR
SA_SIGINFO   -> give handler extra info (who sent it, from which process)
0            -> no flags, default behavior
```

### Installing a handler

```
struct sigaction sa;

ft_memset(&sa, 0, sizeof(struct sigaction)); // zero out — avoid uninitialized fields
sigemptyset(&sa.sa_mask);                    // block nothing extra during handler
sa.sa_handler = my_handler;                  // function to call
sa.sa_flags = 0;                             // default behavior

sigaction(SIGINT, &sa, NULL);
//         |       |     |
//         |       |     +-> save old handler here (NULL = don't care)
//         |       +-------> pointer to the new sa struct
//         +---------------> which signal to configure
```

### What a handler looks like

```
void handler(int sig)   // must have exactly this signature
{
    // sig = the signal number that triggered this call
    // keep it short and async-signal-safe
}
```

Async-signal-safe functions (safe to call from a handler):

```
write()    _exit()    signal()    most low-level syscalls
```

NOT safe:

```
printf()   malloc()   free()   readline()
```

Not safe because they use internal locks — if a signal interrupts malloc
while it holds its lock, and the handler calls malloc, it deadlocks.

### sigaction vs signal()

`signal()` is the older API (simpler but unreliable):

```
signal(SIGINT, handler);  // problems:
                          // on some systems handler resets to SIG_DFL after firing
                          // no control over sa_mask or sa_flags
                          // undefined behavior when signal arrives during handler
```

`sigaction()` is always the right choice for production code.

---

## Core Dump — History and Usage

### The history

In the 1950s-60s, computer memory was made of tiny magnetic iron rings
called **magnetic cores**. Each ring stored one bit. When a program
crashed, engineers needed to inspect the memory state to find the bug.
The only way was to physically print the contents of every core — all
the 1s and 0s — onto paper. That printout was called a **core dump**.

Today memory is silicon, but the name remains. A modern core dump is a
file the OS writes containing the full memory state of a process at the
moment it crashed: stack, heap, registers, everything.

### Triggering and debugging a core dump

```
ulimit -c unlimited       # allow core files of any size (disabled by default)

./myprogram
# press Ctrl+\ to send SIGQUIT
# file called 'core' appears in the current directory

gdb ./myprogram core      # open the core dump in the debugger
```

Inside gdb:

```
bt          -> backtrace: full call stack at moment of crash
info regs   -> all register values
frame 2     -> jump to a specific frame in the stack
list        -> show source code at that point
print x     -> inspect a variable's value
```

The core file is a snapshot of the exact moment the process died.
You can inspect every variable and every function call that led to the
crash — even though the process is long gone.

```
Ctrl+C  (SIGINT)  -> terminate cleanly,              no core file
Ctrl+\  (SIGQUIT) -> terminate + core file on disk,  $? = 131
kill -9 (SIGKILL) -> terminate immediately,           no core file
                     (kernel handles it, never reaches your code)
```

---

## Exit Codes and Signals

Exit codes in Unix are 0-255. Convention:

```
0          -> success
1-125      -> program-defined error codes
126        -> command found but not executable
127        -> command not found
128 + N    -> terminated by signal N
```

So:

```
128 + SIGINT  (2) = 130   -> terminated by Ctrl+C
128 + SIGQUIT (3) = 131   -> terminated by Ctrl+\
128 + SIGKILL (9) = 137   -> killed by kill -9
```

This is a POSIX convention — not enforced by the OS but followed by
bash and most shells. The shell reads the child's exit status using
`waitpid` and the `WIFSIGNALED` / `WTERMSIG` macros:

```
waitpid(pid, &status, 0);
if (WIFSIGNALED(status))
    exit_code = 128 + WTERMSIG(status);  // reconstruct 128 + N
