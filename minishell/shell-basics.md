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

#### Rule 1 — Whitespace separates tokens

```
$ echo hello world
```

Tokens:
- echo  
- hello  
- world

#### Rule 2 — Pipes & redirections are ALWAYS separate tokens

```
$ echo hi>out.txt
```

Tokens:
- echo  
- hi  
- >  
- out.txt

#### Rule 3 — Single quotes ('...') preserve everything

```
$ echo 'hello | world $USER'
```

Token:
- 'hello | world $USER'

#### Rule 4 — Double quotes preserve everything except `$`

```
$ echo "Hello $USER"
```

Tokens:
- echo  
- "Hello $USER"

#### Rule 5 — `$` starts expansion  
(handled later)

#### Rule 6 — `<<` and `>>` are single tokens

```
$<< EOF
```

Tokens:
- <<  
- EOF

#### Rule 7 — Words stop at unquoted operators

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

## TODO
- 6 — Redirections  
- 7 — Execution  
- 8 — Pipelines  
- 9 — Exit status loop  

