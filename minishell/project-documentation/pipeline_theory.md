# Pipelines

## What is a Pipeline?

A pipeline connects multiple commands so the output of one becomes the
input of the next. The shell creates a chain where data flows left to
right without ever being written to disk.

```
cat file.txt | grep hello | sort | wc -l

cat reads file.txt
    → its stdout flows into grep's stdin
        grep filters lines containing "hello"
            → its stdout flows into sort's stdin
                sort alphabetically sorts the lines
                    → its stdout flows into wc's stdin
                        wc counts the lines → prints the number
```

Each command runs simultaneously — they are all forked at the same time
and run in parallel. `grep` starts reading as soon as `cat` starts writing.
`sort` starts receiving data as soon as `grep` starts filtering.

---

## The pipe() Syscall

`pipe(fd)` creates a one-directional in-memory channel and returns two
file descriptors:

```
int fd[2];
pipe(fd);

fd[0]  → read end   (consumer reads from here)
fd[1]  → write end  (producer writes here)
```

Whatever is written to `fd[1]` can be read from `fd[0]`. The kernel
buffers the data in memory. No file is ever created on disk.

```
producer                    consumer
write(fd[1], "space\n", 6)  read(fd[0], buf, 6)
         │                          ▲
         └──── kernel buffer ───────┘
```

### EOF and pipe closing

The consumer (reader) sees EOF only when **all** write ends of the pipe
are closed. This is critical:

```
if the parent keeps fd[1] open after forking:
    consumer reads all data → waits for more
    parent never closes fd[1] → consumer blocks forever
    deadlock
```

This is why every process that does not use a pipe end must close it.

---

## N commands need N-1 pipes

Each pipe connects exactly two adjacent commands. For N commands:

```
2 commands:  A | B              → 1 pipe
3 commands:  A | B | C          → 2 pipes
4 commands:  A | B | C | D      → 3 pipes
N commands:                     → N-1 pipes
```

---

## The pipes array

The shell creates all pipes before forking any child. The result is a
2D array: `int **pipes` where each `pipes[i]` holds one `{read, write}`
pair. 

`read  -> pipe[i][0]` 
`write -> pipe[i][1]`

The file descriptors are assigned by the kernel starting from the
lowest available integer.

For `ls | grep | wc` (3 commands, 2 pipes):

```
pipe(pipes[0]);   // create first pipe

┌────┬─────────────────────────────────┐
│ fd │ what                            │
├────┼─────────────────────────────────┤
│ 0  │ stdin                           │
│ 1  │ stdout                          │
│ 2  │ stderr                          │
│ 3  │ pipes[0][0]  ← read  end pipe 0 │  (ls → grep)
│ 4  │ pipes[0][1]  ← write end pipe 0 │
└────┴─────────────────────────────────┘

pipe(pipes[1]);   // create second pipe

┌────┬─────────────────────────────────┐
│ fd │ what                            │
├────┼─────────────────────────────────┤
│ 0  │ stdin                           │
│ 1  │ stdout                          │
│ 2  │ stderr                          │
│ 3  │ pipes[0][0]  ← read  end pipe 0 │  (ls → grep)
│ 4  │ pipes[0][1]  ← write end pipe 0 │
│ 5  │ pipes[1][0]  ← read  end pipe 1 │  (grep → wc)
│ 6  │ pipes[1][1]  ← write end pipe 1 │
└────┴─────────────────────────────────┘
```

Each `|` operator gets one pipe. The first `|` is `pipes[0]`, the
second `|` is `pipes[1]` and so on:

```
ls  |  grep  |  wc
    ^        ^
  pipes[0]  pipes[1]

pipes[0] connects ls   → grep
pipes[1] connects grep → wc
```

---

## Wiring with dup2

Each child calls `setup_pipes()` before `execve`. It rewires stdin and
stdout to the correct pipe ends using `dup2`:

```
setup_pipes(pipes, cmd_index, total):
    if cmd_index > 0:
        dup2(pipes[cmd_index - 1][0], STDIN_FILENO)
        → stdin reads from previous pipe's read end

    if cmd_index < total - 1:
        dup2(pipes[cmd_index][1], STDOUT_FILENO)
        → stdout writes to current pipe's write end
```

### 4 Commands Example: 

```
               cat file.txt | grep hello | sort | wc -l`
                            ^            ^      ^                               
                         pipe[0]     pipe[1]   pipe[2]
```

| command  | index | stdin                       | stdout                      |
|----------|-------|-----------------------------|-----------------------------|
| cat      | 0     | unchanged (keyboard/file)   | dup2(pipes[0][1], STDOUT)   |
| grep     | 1     | dup2(pipes[0][0], STDIN)    | dup2(pipes[1][1], STDOUT)   |
| sort     | 2     | dup2(pipes[1][0], STDIN)    | dup2(pipes[2][1], STDOUT)   |
| wc -l    | 3     | dup2(pipes[2][0], STDIN)    | unchanged (terminal)        |

The pattern:
```
first command  → stdin unchanged,       stdout → pipes[0] write end
middle command → stdin ← pipes[i-1] read,  stdout → pipes[i] write end
last command   → stdin ← pipes[N-2] read,  stdout unchanged
```

---

## Why close_pipes() in every child

After `dup2`, the child has rewired stdin and stdout to the pipe ends it
needs. All the other pipe ends are now unnecessary and dangerous:

If grep keeps `pipes[1][0]` open (sort's read end), sort will never see
EOF from grep's side, it blocks forever. 
`close_pipes()` closes all of them so only the two `dup2`'d ends remain.

---

## Why the parent must also close all pipes

After forking all children, the parent holds copies of every pipe end.
If it doesn't close them:

```
parent keeps pipes[2][1] open (sort→wc write end)
wc reads all of sort's output → waits for more
sort exits → its write end closes
but parent still has pipes[2][1] open → wc sees no EOF → blocks forever
```

```
close_pipes(pipes, count - 1);   // parent closes all pipe ends
waitpid(last_pid, &status, 0);   // now wc can see EOF and exit
```

---

## Exit code of a pipeline

Only the exit code of the **last command** is returned.

The implementation waits for the last child with `waitpid`, then waits
for all remaining children with `wait(NULL)` to avoid zombies.

---

## Full data flow: `cat file.txt | grep hello | sort | wc -l`

```
parent:
    create pipes[0], pipes[1], pipes[2]
    fork child 0 (cat):
        dup2(pipes[0][1], STDOUT)   stdout → pipe[0]
        close all pipes
        execve("cat", ["file.txt"])
        cat reads file.txt, writes to pipe[0], exits

    fork child 1 (grep):
        dup2(pipes[0][0], STDIN)    stdin  ← pipe[0]
        dup2(pipes[1][1], STDOUT)   stdout → pipe[1]
        close all pipes
        execve("grep", ["hello"])
        grep reads from pipe[0], filters, writes to pipe[1], exits

    fork child 2 (sort):
        dup2(pipes[1][0], STDIN)    stdin  ← pipe[1]
        dup2(pipes[2][1], STDOUT)   stdout → pipe[2]
        close all pipes
        execve("sort")
        sort reads from pipe[1], sorts, writes to pipe[2], exits

    fork child 3 (wc):
        dup2(pipes[2][0], STDIN)    stdin  ← pipe[2]
        close all pipes
        execve("wc", ["-l"])
        wc reads from pipe[2], counts lines, prints to terminal, exits

    close all pipes               ← parent must close or wc blocks
    waitpid(wc_pid, &status)      ← wait for last command wait(NULL) × 3                ← wait for remaining children return wc's exit code ```
