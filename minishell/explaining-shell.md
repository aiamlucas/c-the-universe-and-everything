# Explaining Bash

## 1 - Input handling (readline)

Minishell uses the `readline()` function to read user input from the terminal.  
This function provides a command-line interface with history support and handles basic terminal input.

---

### Basic Input Behaviors

**Empty line** → prints a new prompt  
```
$
$
```

**Whitespaces only** → bash ignores the whitespaces and prints a new prompt  
```
$
$
```

**Ctrl+C (in empty prompt)** → new line + new prompt  
```
$ ^C
$
```

**Ctrl+C (with typed text)** → clear line + new prompt  
```
$ sometext^C
$
```

**Ctrl+D (in empty prompt)** → exit shell  
```
$
exit
```

**Ctrl+D (with typed text)** → does nothing  
```
$ sometext
```

**Ctrl+\ (in empty prompt)** → does nothing  
```
$
```

**Ctrl+\ (during a running command)** → quit signal  
```
$ cat
hello
hello
^\Quit (core dumped)
$
```

---

### History

For having history, minishell must store every valid command line into the history list using `add_history()`.

Correct history behavior:

- Only *complete* input lines entered at the prompt are saved.
- Empty lines or whitespace-only lines are NOT saved.
- Lines typed inside a heredoc are NOT saved

---

### Signal Handling Implementation

Minishell must handle signals correctly to achieve the behaviors described above.

**Subject constraint: Only ONE global variable allowed.**

```
external volatile sig_atomic_t g_signal = 0;
```

This variable must:

- Store ONLY the signal number received  
- NOT be a struct or contain multiple fields  
- NOT provide access to other data structures  

---

### Signal Setup

Minishell needs different signal behaviors in different contexts:

**In parent (interactive mode - waiting for input):**

- SIGINT (Ctrl+C) → custom handler that prints new line + new prompt  
- SIGQUIT (Ctrl+\) → ignored (SIG_IGN)

**In child (during command execution):**

- SIGINT (Ctrl+C) → default behavior (terminates child)  
- SIGQUIT (Ctrl+\) → default behavior (terminates child with core dump)

---

### Signal Handler Workflow

1. Signal arrives → handler sets g_signal = signal_number  
2. Main loop detects g_signal != 0  
3. Take appropriate action:  
   - SIGINT → print newline, new prompt, update `$?` to 130  
   - SIGQUIT (in child) → update `$?` to 131  
4. Reset g_signal = 0  

---

### Exit Status from Signals

When a child process is killed by a signal, the exit code follows this formula:

```
exit_code = 128 + signal_number
```

It must add 128 because Unix shells reserve:

- 0–125 → normal exits  
- 126 → permission denied  
- 127 → command not found  

**Common cases:**

- SIGINT (2) → 128 + 2 = 130  
- SIGQUIT (3) → 128 + 3 = 131  

**Implementation:**

After `waitpid()`, check:

- `WIFSIGNALED(status)`  
- If true → `128 + WTERMSIG(status)`  

---

# 2 - Lexing / Tokenizing

Tokenizing is the process of breaking the user input into meaningful pieces called *tokens*.

Tokens represent words, operators (`|`, `<`, `>`, `<<`, `>>`, `&&`, `||`), quotes (`'`, `"`) and special symbols (`$`, `\`, `;`, `&`, `(`, `)`, `*`, `?`, `[`, `]`, `~`).

**Mandatory for minishell:**  
operators (`|`, `<`, `>`, `<<`, `>>`), quotes (`'`, `"`) and `$` (expansion)

---

## 2.1 Rules for splitting the input into tokens

### Rule 1: Whitespace separates tokens

```
$ echo hello world
```

Tokens:

echo  
hello  
world  

---

### Rule 2: Pipes & redirections are always separate tokens

```
$ echo hi>out.txt
```

Tokens:

echo  
hi  
>  
out.txt  

---

### Rule 3: Single quotes preserve EVERYTHING (`'...'`)

Nothing is interpreted inside single quotes.

```
$ echo 'hello | world $USER'
```

Token:

'hello | world $USER'

---

### Rule 4: Double quotes preserve everything EXCEPT `$`

```
$ echo "Hello $USER"
```

Tokens:

echo  
"Hello $USER"

---

### Rule 5: `$` starts variable expansion

(Expansion happens later.)

---

### Rule 6: `<<` and `>>` are single tokens

Must not be split into `<` `<` or `>` `>`.

```
$ << EOF
```

Tokens:

<<  
EOF  

---

### Rule 7: Words stop at unquoted special characters

```
$ cat<file
```

Tokens:

cat  
<  
file  

---

# 3 - Parsing

Parsing takes the tokens and builds a structure representing the commands.

The parser must identify:

- Command name  
- Arguments  
- Redirections  
- Pipes  
- Which redirection belongs to which command  

Example:
```
ls -l | grep txt > out.txt
```

Tokens:

ls  
-l  
|  
grep  
txt  
>  
out.txt  

```
PIPELINE
Command 1:
  name: ls
  args: ["ls", "-l"]
  redirections: none
```

```
Command 2:
  name: grep
  args: ["grep", "out.txt"]
  redirections:
    stdout → out.txt (overwrite)
```

Parser steps:

1. Take tokens in order  
2. Create command on word  
3. Add following words as args  
4. On redirection:
   - next token must be filename  
   - attach to command  
5. On pipe:
   - finish current command  
   - start next one  
6. Continue until done  

---

# 4 - Expansion

Expansion happens after parsing and before quote removal.

Expands:

- `$VAR`  
- `$?`  
- variables inside double quotes  
- NOT inside single quotes  

Rules:

1. `$VAR` → environment variable value  
2. `$?` → previous exit code  
3. Double quotes allow expansion  
4. Single quotes disable it  
5. Undefined variable → empty string  
6. Expansion may create multiple args  
7. No advanced Bash expansions required  

Example:

```
$ PATH="/bin /usr/bin"
$ echo $PATH
/bin /usr/bin
$ printf "[%s]\n" $PATH
[/bin]
[/usr/bin]
$ printf "[%s]\n" "$PATH"
[/bin /usr/bin]
```

---

# 5 - Quote Removal

After expansion:

- remove `'`  
- remove `"`  

Examples:

```
$ echo '$USER'
→ $USER
```

```
$ USER=alice
$ echo "User: $USER"
→ User: alice
```

```
$ echo "'hello' "$USER
→ 'hello'
→ alice
```

---

# 6 - Redirections

Minishell must support:

- `<` input  
- `>` output overwrite  
- `>>` append  
- `<<` heredoc  

Redirections belong to **the command immediately before them**  
(unless appearing before any command → then they belong to the first command).

Examples:

`echo hi > out.txt` → `>` belongs to echo  

`cat < in.txt | grep hi > result.txt`  
- `<` belongs to cat  
- `>` belongs to grep  

`> log.txt echo hi`  
- first token → belongs to echo  

---

## 6.1 Input Redirection `<`

```
open(file, O_RDONLY)
dup2(fd, STDIN_FILENO)
close(fd)
```

---

## 6.2 Output Redirection `>`

```
open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644)
dup2(fd, STDOUT_FILENO)
close(fd)
```

---

## 6.3 Append Redirection `>>`

```
open(file, O_WRONLY | O_CREAT | O_APPEND, 0644)
dup2(fd, STDOUT_FILENO)
close(fd)
```

---

## 6.4 Heredoc `<<`

```
$ cat << EOF
hello
there
EOF
```

Behavior:

- readline until limiter  
- store lines  
- redirect STDIN to stored content  

### Heredoc and History

Heredoc lines **do not** enter history.

---

### Heredoc Expansion Rules

**Unquoted delimiter → expands variables**

**Quoted delimiter → no expansion**

Examples:

```
$ USER=alice
$ cat << EOF
Hello $USER
Status: $?
EOF
```

Output:

Hello alice  
Status: 0  

```
$ cat << "EOF"
Hello $USER
Status: $?
EOF
```

Output:

Hello $USER  
Status: $?  

**Any quoting disables expansion:**  
`<< "EOF"`  
`<< 'EOF'`  
`<< E"O"F`  

---

### Heredoc with Pipes

```
$ cat << END | grep hello
hello world
goodbye world
END
```

Output:  

hello world  

---

## 6.5 Multiple Redirections

Process left to right.  
Last redirect of each type wins.

---

# 7 - Execution (builtins / external)

After parsing, redirections, and expansion, minishell executes commands.

Two types:

1. Builtins  
2. External programs  

---

## 7.1 Builtins

Mandatory builtins:

- echo  
- cd  
- pwd  
- export  
- unset  
- env  
- exit  

**Builtins run in parent unless in a pipeline.**

Example:

```
$ cd ..
$ export VAR=hello
$ exit
```

Builtin in pipeline runs in child:

```
$ echo hi | wc -c
```

---

## 7.2 External Commands

Use:

- fork()  
- execve()  
- waitpid()  

Steps:

1. fork  
2. apply redirections  
3. build argv  
4. resolve `$PATH`  
5. execve  
6. parent waits  

---

## 7.3 PATH Resolution

If command contains `/` → execute directly.

Else search:

- /bin  
- /usr/bin  
- etc.  

Errors:

- not found → 127  
- permission denied / directory → 126  

---

## 7.4 Execution in Pipelines

`A | B | C`:

- each command runs in child  
- pipes connect their fds  
- parent closes unused ends  
- waits for all children  

---

## 7.5 Builtins in Pipelines

Builtin alone → parent  
Builtin in pipeline → child  

---

## 7.6 Exit Status

`$?` updated with:

- builtin return  
- child exit code  
- 130 on SIGINT  
- 131 on SIGQUIT  

---

## 7.7 Execution Error Cases

Examples:

```
$ ./folder
minishell: ./folder: Is a directory

$ ./no-permission
minishell: ./no-permission: Permission denied
```

---

# 8 - Pipelines

Syntax: `cmd1 | cmd2 | cmd3`

Rules:

- each command in its own child  
- stdout piped to next stdin  
- redirections isolated per command  
- `$?` = exit status of last command  

---

# 9 - Exit status loop

After each command minishell updates `$?`.

Rules:

- external → waitpid() result  
- builtin → return value  
- pipeline → last command’s exit  
- SIGINT → 130  
- SIGQUIT → 131  

Examples:

```
$ ls
$ echo $?
0

$ nonexistentcommand
127

$ sleep 5
^C
$ echo $?
130
```

---

# 10 - Main Loop (Program Flow)

Minishell loop:

1. Print prompt  
2. Read input  
3. Lexing  
4. Parsing  
5. Expansion  
6. Prepare redirections & pipelines  
7. Execute  
8. Update `$?`  
9. Repeat  

Exit when:

- `exit`  
- Ctrl+D on empty prompt  
