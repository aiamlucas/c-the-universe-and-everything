# Minishell

---

## 1. Lexer

Reads the raw input and breaks it into a linked list of tokens.
Quoted sections are kept intact

```
input:  echo hey | wc -c

[echo] -> [hey] -> [|] -> [wc] -> [-c] -> NULL
```

---

## 2. Expansion

Replaces `$VAR` and `$?` with their values from the environment.
Quote context determines whether expansion happens:

```
export USER=annamaria

input:  echo $USER

before:  [echo] -> [$USER] -> NULL
after:   [echo] -> [annamaria] -> NULL
```

Quote rules:
```
"$USER"   →  annamaria       double quotes: $VAR still expands
'$USER'   →  $USER           single quotes: $VAR kept literal
$?        →  0               last exit code
$UNSET    →  (empty)         unset variable → empty string
```

---

## 3. Quote Removal

Strips quote characters from TOKEN_WORD values

Heredoc delimiters are **skipped** intentionally. Their quotes must survive
until the parser reads them to decide whether `$VAR` should expand inside
the heredoc content. 
The parser handles quote removal for delimiters separately.

```
input:  echo "hi annamaria"

before:  [echo] -> ["hi annamaria"] -> NULL
after:   [echo] -> [hi annamaria]   -> NULL
```

```
input:  cat << 'EOF'

before:  [cat] -> [<<] -> ['EOF'] -> NULL
after:   [cat] -> [<<] -> ['EOF'] -> NULL   ← ['EOF'] untouched
```

---

## 4. Parser

Walks the token list once and builds a linked list of commands.
Each command holds:
- `argv` NULL-terminated array of strings: the command and its arguments
- `redirections` linked list of redirection pairs: operator + target word

```
input:  echo hey | wc -c > file.txt

tokens: [echo]->[hey]->[|]->[wc]->[-c]->[>]->[file.txt]->NULL

[echo][hey] → cmd1: argv=["echo","hey",NULL]
[|]         → add cmd1 to list, start fresh
[wc][-c]    → cmd2: argv=["wc","-c",NULL]
[>][file]   → redir={ REDIR_OUT → "file.txt" } attached to cmd2
end         → append cmd2 manually (no pipe triggered it)

┌────────────────────────────┐     ┌─────────────────────────────────────┐
│ cmd1                       │next │ cmd2                                │next
│ argv:  ["echo","hey",NULL] │────>│ argv:  ["wc","-c",NULL]             │────> NULL
│ redir: NULL                │     │ redir: { REDIR_OUT → "file.txt" }   │
└────────────────────────────┘     └─────────────────────────────────────┘
```

---

## 5. Execution

### Example A [builtin] 
### `echo hey`

Builtins that don't modify shell state (echo, pwd, env) and have no
redirections run directly in the parent process, no fork needed.

Builtins that modify shell state (cd, export, unset, exit) **must** run
in the parent, a child process would change its own state and exit,
leaving the parent unchanged.

```
echo hey

is_builtin           → true
must_run_in_parent?  → false (echo doesn't modify shell state)
has redirections?    → no
→ run execute_builtin() directly in parent
→ prints "hey"
→ return 0
```

### Example B [external command]
### `/bin/ls`

External commands always need a fork. execve() does not create a new
process, it transforms the current one: same pid, but code, stack and
heap are completely replaced by the new program. 
It never returns on success. If the shell called execve() directly, the shell would be gone.

fork() first creates a copy of the shell. execve() then runs inside
the copy. The original shell waits and continues after.


```
/bin/ls

find_dir("/bin/ls") → has '/' → already a path, use directly

fork()
┌─────────────────────────────────────────────┐
│ PARENT                  CHILD               │
│                                             │
│ waits...                no redirections     │
│                         execve("/bin/ls")   │
│                         child IS now ls     │
│                         ls runs, exits 0    │
│                                             │
│ wait returns            (gone)              │
│ exit code = 0                               │
└─────────────────────────────────────────────┘
```

### Example C [redirection]
### `echo hey > out.txt`

Builtins with redirections run in a fork so the fd changes don't affect
the shell's own stdin/stdout permanently.

```
echo hey > out.txt

fork()
┌────────────────────────────────────────────────┐
│ PARENT                   CHILD                 │
│                                                │
│ stdout → terminal        open("out.txt") →fd=3 │
│ (untouched forever)      dup2(3, stdout)       │
│                          stdout → out.txt      │
│ waits...                 echo prints "hey"     │
│                          → goes to out.txt     │
│ wait returns             exit(0)               │
└────────────────────────────────────────────────┘
```

### Example D [pipeline]
### `ls | grep .c | wc -l`

N commands need N-1 pipes. All children fork simultaneously and run in parallel.

```
ls  |  grep .c  |  wc -l
    ^           ^
 pipes[0]     pipes[1]
```

`pipes` is a 2D array `int **pipes`
where each `pipes[i]` holds one `{read, write}` pair:

```
pipe(pipes[0]);   // create first pipe
pipe(pipes[1]);   // create second pipe

┌─────┬──────────────────────────────────┐
│ fd  │ what                             │
├─────┼──────────────────────────────────┤
│  0  │ stdin                            │
│  1  │ stdout                           │
│  2  │ stderr                           │
│  3  │ pipes[0][0]  ← read  end pipe 0  │  grep reads ls output from here
│  4  │ pipes[0][1]  ← write end pipe 0  │  ls writes its output here
│  5  │ pipes[1][0]  ← read  end pipe 1  │  wc reads grep output from here
│  6  │ pipes[1][1]  ← write end pipe 1  │  grep writes its output here
└─────┴──────────────────────────────────┘
```

Each child rewires stdin/stdout to the correct pipe ends:

| command | index | stdin                     | stdout                    |
|---------|-------|---------------------------|---------------------------|
| ls      | 0     | unchanged (keyboard)      | dup2(pipes[0][1], STDOUT) |
| grep    | 1     | dup2(pipes[0][0], STDIN)  | dup2(pipes[1][1], STDOUT) |
| wc -l   | 2     | dup2(pipes[1][0], STDIN)  | unchanged (terminal)      |

```
parent:
    close all pipe ends        ← must close or wc never sees EOF and blocks
    waitpid(wc_pid)            ← wait for last command, save its exit code
    wait(NULL) until -1        ← collect ls and grep (avoid zombies)
    return wc exit code        ← pipeline result = last command's exit code
```

---

## 6. Heredoc

Heredoc content is read from the user after parsing, before execution.
The delimiter quotes decide whether `$VAR` expands inside.

```
cat << EOF    →  should_expand = true   ($VAR expands inside)
cat << 'EOF'  →  should_expand = false  ($VAR kept literal)
```

`process_all_heredocs()` runs before execution. 
It forks a child for each heredoc redirection. 
The child reads lines from the user with `readline("> ")` and writes them into a pipe. 
If `should_expand` is true, each line is expanded.
When the user types the delimiter alone, the child exits.
The pipe read end is stored in the redirection node and later wired to stdin
of the command via `apply_redirections()`.

```
cat << EOF
hello $USER
EOF

→ should_expand = true
→ "hello $USER" → expand → "hello annamaria"
→ written to pipe
→ cat reads from pipe → prints "hello annamaria"
```

---

## 7. Signals

| signal  | at prompt              | during command          |
|---------|------------------------|-------------------------|
| Ctrl+C  | new prompt, `$?=130`   | kills command, `$?=130` |
| Ctrl+\  | ignored                | kills + core dump       |
| Ctrl+D  | exit shell             | EOF to command          |

**`setup_signals()`** is called once at startup. 
It sets the shell's signal handlers for the interactive prompt:
- `SIGINT` → custom handler: clears the readline line and redisplays the prompt
- `SIGQUIT` → ignored

**`reset_signals()`** is called in every child after `fork()`, before `execve()`.
It restores `SIG_DFL` for both signals so the program being executed gets
default behaviour 

**`update_sigint()`** swaps the SIGINT handler at runtime.

While a child is running, the shell should not redisplay the prompt
on Ctrl+C.

    before waitpid → handle_sigint_child   (just sets the flag, nothing else)
    after  waitpid → handle_sigint_parent  (clears line, redisplays prompt)

- with update_sigint:
```
$[ 🛸 ]>cat
^C
$[ 🛸 ]> ← clean new prompt
```

- without update_sigint:
```
$[ 🛸 ]>cat
^C
$[ 🛸 ]>$[ 🛸 ]> ← prompt printed twice
```

