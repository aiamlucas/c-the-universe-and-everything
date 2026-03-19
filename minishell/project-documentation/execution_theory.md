# Execution — Theory

## Processes and the OS

Every program running on a computer is a **process**, an isolated
instance of a program with its own memory, registers, file descriptors,
and execution state. The OS kernel manages all processes and decides
which one runs on the CPU at any given moment.

Each process has a **PID** (Process ID): a unique integer assigned by
the kernel. The kernel also tracks a **PPID** (Parent Process ID)
every process except the very first one was created by another process.

```
kernel
  └── init (pid=1)
        └── bash (pid=100)
              └── minishell (pid=200)
                    └── ls (pid=201)   ← created by minishell to run a command
```

---

## fork()

`fork()` is the Unix syscall for creating a new process. It makes an
almost exact copy of the calling process: same memory, same file
descriptors, same signal handlers. The new process is called the
**child**; the original is the **parent**.

The critical detail: `fork()` returns **twice**.

```
pid_t pid = fork();

parent receives: pid = child's PID  (e.g. 201)   → pid > 0
child  receives: pid = 0                         → pid == 0
error:           pid = -1
```

Both processes continue executing from the line after `fork()`. The
return value is the only thing that tells them apart:

```
int main(void)
{
    pid_t pid = fork();

    if (pid == 0)
        printf("I am the child\n");   // runs in child
    else
        printf("I am the parent\n"); // runs in parent

    // both processes reach here
}
```

After fork, child and parent run independently and simultaneously.
They share nothing the child gets a **copy** of the parent's memory,
not the original. Changes in the child do not affect the parent.

---

## execve()

`execve()` replaces the current process with a new program. It does not
create a new process. It transforms the calling process.

```
execve(path, argv, envp)
//     ^      ^      ^
//                         path -> path to executable
//                         argv -> NULL-terminated array of arguments
//                         envp -> NULL-terminated array of environment variables
```

When `execve()` succeeds, the process's code, stack, and heap are
completely replaced by the new program. The PID stays the same.
`execve()` never returns on success, the original code is gone.

```
before execve:           after execve:
  pid = 201               pid = 201  (unchanged)
  code: minishell         code: /bin/ls
  stack: minishell        stack: ls
  heap: minishell         heap: ls
  open fds: inherited     open fds: inherited (unless O_CLOEXEC)
```

If `execve()` returns, something went wrong, the file was not found,
not executable, or the kernel ran out of resources.

---

## The fork + execve Pattern

Running an external command always requires both:

```
1. fork()    → create a child process
2. execve()  → replace the child with the target program
```

Why not just `execve()` directly? Because `execve()` destroys the
calling process. If the shell called it directly, the shell would be
gone and replaced by `ls`. There would be no shell left to continue.

```
shell
  │
  ├── fork() ──────────────────► child (copy of shell)
  │                                  │
  │   (shell continues here)         ├── set up redirections
  │                                  ├── reset signals
  │                                  └── execve("/bin/ls") → becomes ls
  │                                      ls runs, prints, exits
  │
  └── waitpid(child) ◄────────── child exit status returned
        shell gets exit code
        shell continues
```

This pattern appears everywhere in Unix. It is how every shell runs
every external command.

---

## waitpid()

After forking, the parent typically waits for the child to finish.
`waitpid()` blocks the parent until the child exits and returns its
exit status.

```
waitpid(pid, &status, 0);
```

The exit status is encoded in an integer. Two macros decode it:

```
WIFEXITED(status)    → true if child exited normally (via exit() or return)
WEXITSTATUS(status)  → the actual exit code (0–255)

WIFSIGNALED(status)  → true if child was killed by a signal
WTERMSIG(status)     → which signal killed it
```

Shell convention for signal-killed processes:

```
exit_code = 128 + signal_number

Ctrl+C (SIGINT=2)  → 130
Ctrl+\ (SIGQUIT=3) → 131
kill -9 (SIGKILL=9) → 137
```

---

## File Descriptors and Redirections
 
Every process inherits three open file descriptors from its parent:
 
```
0  stdin   → keyboard (by default)
1  stdout  → terminal (by default)
2  stderr  → terminal (by default)
```
 
A file descriptor is just an integer: a number that the kernel uses
to identify an open file, pipe, or device. When you open a file the
kernel returns the lowest available integer:
 
```
process starts with:   fd=0  (stdin), fd=1 (stdout), fd=2 (stderr)
open("out.txt")    →   fd=3  (next available)
open("in.txt")     →   fd=4
```
 
### dup2(oldfd, newfd)
 
`dup2` is the syscall that makes redirection possible. It makes `newfd`
point to the same file as `oldfd`, closing whatever `newfd` was pointing
to before:
 
```
before dup2:
  fd=1 → terminal
  fd=3 → out.txt
 
dup2(3, 1)
 
after dup2:
  fd=1 → out.txt   ← stdout now writes to the file
  fd=3 → out.txt   ← both point to the same file
 
close(3)           ← fd=3 no longer needed, fd=1 is enough
```
 
After `dup2(3, 1)`, anything that writes to stdout (fd=1) goes to
`out.txt`. The program doesn't know it just writes to fd=1 as always.
 
### Full example: echo hello > out.txt
 
```
open("out.txt", O_WRONLY|O_CREAT|O_TRUNC) → fd=3
dup2(3, STDOUT_FILENO)  → fd=1 now points to out.txt
close(3)                → fd=3 no longer needed
execve("echo")          → echo writes "hello\n" to fd=1 → goes to out.txt
```
 
### Why redirections must happen in the child after fork
 
```
fork()
  child:
    open file → fd=3
    dup2(3, STDOUT_FILENO)   → rewire child's stdout
    close(3)
    execve()                 → new program inherits the rewired stdout
  parent:
    untouched — its stdout still points to terminal ✓
```
 
If `dup2` ran in the parent before fork, the parent's stdout would be
permanently redirected to the file. The shell would stop printing to
the terminal.
 
### Heredoc and fd
 
For heredoc, there is no file to open — the content comes from a pipe
that was created earlier by `process_all_heredocs()`. The pipe's read
end was stored in `redir->fd` at that point.
 
```
process_all_heredocs():
    pipe(fd)             → fd[0]=read, fd[1]=write
    fork child:
        reads lines from user → writes to fd[1]
        exits when delimiter matched
    parent:
        redir->fd = fd[0]    ← store read end in the redir node
 
apply_redirections() later:
    if (redir->fd != -1)     ← -1 means heredoc was interrupted (Ctrl+C)
        dup2(redir->fd, STDIN_FILENO)  → stdin reads from the pipe
        execve("cat") → cat reads heredoc content from stdin ✓
```
 
The `fd != -1` check is a guard — if the user pressed Ctrl+C during
heredoc input, the pipe was closed and `fd` was never set. Calling
`dup2(-1, stdin)` would be an error.
 
---

## Pipes

A pipe is a one-directional in-memory channel between two processes.
`pipe(fd)` creates it and returns two file descriptors:

```
fd[0]  → read end
fd[1]  → write end

whatever is written to fd[1] can be read from fd[0]
```

For a pipeline `ls | grep .c`:

```
pipe(fd) → fd[0]=read, fd[1]=write

fork child 1 (ls):
    dup2(fd[1], STDOUT_FILENO)  → ls writes to pipe instead of terminal
    close(fd[0]), close(fd[1])
    execve("ls")

fork child 2 (grep):
    dup2(fd[0], STDIN_FILENO)   → grep reads from pipe instead of keyboard
    close(fd[0]), close(fd[1])
    execve("grep", [".c"])

parent:
    close(fd[0]), close(fd[1])  → parent must close both ends
    waitpid(child1)
    waitpid(child2)
```

The parent must close its copies of both pipe ends. If it keeps fd[1]
open, grep will never see EOF and will block forever waiting for more
input that never comes.

---

## Builtins: Why They Run in the Parent

Builtins like `cd`, `export`, `unset`, and `exit` modify the shell's
own state. `cd` changes the working directory, `export` modifies the
environment, `exit` terminates the shell itself.

If these ran in a child process, the changes would affect only the child.
The child would `cd` to `/tmp`, then exit. The parent's working directory
would be unchanged. The `cd` would appear to do nothing.

```
cd /tmp in a child:
    child: chdir("/tmp")  → child's cwd = /tmp
    child exits
    parent: cwd still = /home/user  ← nothing changed

cd /tmp in the parent:
    parent: chdir("/tmp") → parent's cwd = /tmp  ✓
```

This is why `must_run_in_parent()` exists — it identifies the builtins
that must run directly in the shell process to have any effect.

The one exception: if a parent-modifying builtin has redirections
(e.g. `export > file`), it runs in a fork so the file descriptor
changes don't affect the shell's own stdin/stdout permanently.
