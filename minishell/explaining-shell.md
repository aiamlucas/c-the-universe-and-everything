# Bash Behavior & Shell Stages

## 1. Explaining Bash

### 1.1 Input Handling (readline)

- **Empty line → prints a new prompt**

```
$
$
```

- **Whitespaces only → bash ignores them and prints a new prompt**

```
$
$
```

- **Ctrl+C (in empty prompt → new line + new prompt)**

```
$ ^C
$
```

- **Ctrl+C (with typed text → clear line + new prompt)**

```
$ sometext^C
$
```

- **Ctrl+D (in empty prompt → exit shell)**

```
$
exit
```

- **Ctrl+D (with typed text → does nothing)**

```
$ sometext
```

- **Ctrl+\ (in empty prompt → does nothing)**

```
$
```

- **Ctrl+\ (during a running command → sends SIGQUIT)**

```
$ cat
hello
hello
^\Quit (core dumped)
$
```

---

## 2. Lexing / Tokenizing

Tokenizing breaks the user’s input into *tokens*:  
words, operators, quotes, and `$`.

### 2.1 Rules for Splitting Tokens

#### Rule 1: Whitespace separates tokens

```
$ echo hello world
```

Tokens:
- echo  
- hello  
- world

#### Rule 2: Pipes & redirections are ALWAYS separate tokens

```
$ echo hi>out.txt
```

Tokens:
- echo  
- hi  
- \>  
- out.txt

#### Rule 3: Single quotes ('...') preserve everything

```
$ echo 'hello | world $USER'
```

Token:
- 'hello | world $USER'

#### Rule 4: Double quotes preserve everything except `$`

```
$ echo "Hello $USER"
```

Tokens:
- echo  
- "Hello $USER"

#### Rule 5: `$` starts expansion  
(handled later)

#### Rule 6: `<<` and `>>` are single tokens

```
$<< EOF
```

Tokens:
- <<  
- EOF

#### Rule 7: Words stop at unquoted operators

```
$ cat<file
```

Tokens:
- cat  
- <  
- file

---

## 3. Parsing

Parsing organizes tokens into a structure containing:
- commands  
- arguments  
- redirections  
- pipelines  

### Example

Input:

```
$ ls -l | grep txt > out.txt
```

Tokens:  
ls, -l, |, grep, txt, >, out.txt

Parsed representation:

```
PIPELINE
Command 1:
  name: ls
  args: ["ls", "-l"]
  redirections: none

Command 2:
  name: grep
  args: ["grep", "txt"]
  redirections:
      stdout → out.txt (overwrite)
```

Parser algorithm:
1. First word starts a command.  
2. Following words are arguments.  
3. Redirection → attach filename.  
4. Pipe → end command, start next.  
5. Continue until tokens end.

---

## 4. Expansion

Expansion happens **after parsing**, before quote removal.

### Types of Expansion
- `$VAR` environment variable  
- `$?` last exit code  
- expansion inside `" "`  
- **no expansion inside `' '`**  
- undefined variable → empty string  

---

### Rules

#### 1. `$VAR` → variable value

```
$ echo $HOME
ssin
```

#### 2. `$?` → exit status

```
$ echo $?
2
```

#### 3. Expansion inside double quotes

```
$ export USER=ssin
$ echo "User: $USER"
User: ssin
```

#### 4. No expansion in single quotes

```
$ echo '$USER'
$USER
```

#### 5. Undefined variable → empty string

```
$ echo "$DOES_NOT_EXIST"
```

(empty output)

#### 6. Expansion can change argument count (word splitting)

```
$ PATH="/bin /usr/bin"
$ echo $PATH
/bin /usr/bin
```

Two arguments.

Quoted:

```
$ echo "$PATH"
/bin /usr/bin
```

printf demonstration:

```
$ printf "[%s]\n" $PATH
[/bin]
[/usr/bin]

$ printf "[%s]\n" "$PATH"
[/bin /usr/bin]
```

#### 7. Minishell does NOT implement:
- `{}` brace expansion  
- `$(( ))` arithmetic  
- `$(...)` command substitution  
- `~` tilde expansion  

Only `$VAR` and `$?`.

---

## 5. Quote Removal

After expansion:
- quotes are stripped  
- expanded values remain  
- `' '` prevents expansion but still gets removed  

### Examples

#### Single quotes → no expansion, quotes removed

Input:

```
$ echo '$USER'
```

After quote removal:  
`$USER`

#### Double quotes → allow expansion, then removed

USER=alice

```
$ echo "User: $USER"
```

Final:  
`User: alice`

#### Mixed quoting example

```
$ echo "'hello' "$USER
```

Final arguments:
- `'hello'`  
- `alice`

---

# 6. Redirections

Redirections allow a command to read from or write to files instead of the terminal.

Minishell must support:

- Input redirection: `<`
- Output redirection (overwrite): `>`
- Output redirection (append): `>>`
- Heredoc: `<<`

Redirections always belong **to the command immediately before them** (unless it's the first token).

---

## 6.1 Input Redirection `<`

Syntax:

`command < file`

This replaces the command’s STDIN (fd 0) with the contents of the file.

Example:

```
$ cat < input.txt
```

Implementation steps:
1. `open(file, O_RDONLY)`
2. `dup2(fd, STDIN_FILENO)`
3. `close(fd)`

Minishell must detect:
- missing filename → syntax error
- unreadable file → print error, set exit code to 1

---

## 6.2 Output Redirection `>`

Syntax:

`command > file`

Writes command output to the file, overwriting it.

Example:

```
$ echo hello > out.txt
```

Implementation:
1. `open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644)`
2. `dup2(fd, STDOUT_FILENO)`
3. `close(fd)`

---

## 6.3 Append Redirection `>>`

Syntax:

`command >> file`

Appends output at the end of the file.

Example:

```
$ echo world >> out.txt
```

Implementation:
1. `open(file, O_WRONLY | O_CREAT | O_APPEND, 0644)`
2. `dup2(fd, STDOUT_FILENO)`
3. `close(fd)`

---

## 6.4 Heredoc `<<`

Syntax:

`command << LIMITER`

The shell reads input **until** the limiter word appears.

Example:

```
$ cat << EOF
hello
there
EOF
```

Behavior:
- Minishell reads lines using `readline()`
- Stops when the line == LIMITER
- The collected text is written into a temporary pipe
- The command’s STDIN is replaced with that pipe

Important:
- NO variable expansion inside heredoc if limiter is quoted
- Expansion **is allowed** if limiter is unquoted

Examples:

```
$ cat << EOF
$USER
EOF
```

→ expands `$USER`

But:

```
$ cat << "EOF"
$USER
EOF
```

→ printed literally as `$USER`

---

## 6.5 Multiple Redirections

Example:

```
$ cat < in.txt > out.txt
```

Rules (Bash-compatible):
- The **last** redirection of each type wins
- Order matters for execution, but not for parsing structure

---

# 7. Execution (Builtins and External Commands)

After parsing, redirections, and expansion, minishell must execute the command.

There are two types of commands:

1. **Builtins** (handled inside the shell)
2. **External programs** (run via `execve()`)

---

## 7.1 Builtins

Mandatory builtins for minishell:

- `echo`  
- `cd`  
- `pwd`  
- `export`  
- `unset`  
- `env`  
- `exit`

Builtins run **without creating a new process**, *except when inside a pipeline*.

Example:

```
$ cd ..
```

`cd` must change the working directory of the **minishell process**, so it cannot be executed using `execve()`.

---

## 7.2 External Commands

Everything that is not a builtin is executed using:

```
fork()
execve()
waitpid()
```

Execution steps:

1. Fork a child process
2. Apply redirections (dup2)
3. Build `argv`
4. Search command in PATH using:
   - The command itself if it contains `/`
   - Paths from `$PATH` environment variable
5. `execve(path, argv, envp)`
6. Parent waits using `waitpid()`

---

## 7.3 PATH Resolution

Minishell must try:

For command `ls`:

- `/bin/ls`
- `/usr/bin/ls`
- etc.

If the command contains `/`, e.g. `./program`, PATH is ignored.

Errors:

- command not found → exit status `127`
- permission denied → exit status `126`

Example:

```
$ asdf
minishell: asdf: command not found
```

---

## 7.4 Execution in Pipelines

In a pipeline:

`A | B | C`

Each command runs in **its own process**, including builtins.

Each child receives:
- pipe input
- pipe output
- any redirections

Parent only:
- closes unused pipe ends
- waits for all children

---

## 7.5 Builtins in Pipelines

Example:

```
$ echo hi | wc -c
```

`echo` is a builtin → but must run in a forked child because it is part of a pipeline.

Rule:
- **If builtin is alone → run in parent (no fork)**  
- **If builtin is in a pipeline → run in child (fork)**

---

## 7.6 Exit Status

After execution, minishell must set `$?` equal to:
- waitpid() result for external commands  
- builtin return value  
- correct signal code if killed by signal (`130`, `131`, …)

Examples:

Ctrl+C during a command → exit code 130  
Ctrl+\ (SIGQUIT) → exit code 131

Examples:

```
$ sleep 10
^C
$ echo $?
130
```

---

## 7.7 Execution Error Cases

- File not found
- Permission denied
- Directory instead of executable
- Missing PATH
- Execve failure

Minishell prints errors **like bash**.

Examples:

```
$ ./folder
minishell: ./folder: Is a directory
```

```
$ ./no-permission
minishell: ./no-permission: Permission denied
```

## TODO
- 8 — Pipelines  
- 9 — Exit status loop  

