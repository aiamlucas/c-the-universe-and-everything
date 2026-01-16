# Minishell - Project Documentation

---

## Data Flow

```
User Input String
    ↓
[TOKENIZER] → Token List (linked list)
    ↓
[PARSER] → Command List (linked list)
    ↓
[EXECUTOR] → Execute commands
    ↓
Output / Exit Code
```

---

## Core Data Types

### Token Types (enum)

```
typedef enum e_token_type
{
    TOKEN_WORD,           // Regular words: cat, hello, file.txt
    TOKEN_PIPE,           // |
    TOKEN_REDIR_IN,       // < 
    TOKEN_REDIR_OUT,      // >
    TOKEN_REDIR_APPEND,   // >>
    TOKEN_HEREDOC         // <<
}   t_token_type;
```

**Purpose:** Categorizes each token after lexical analysis.

---

### Token Structure (linked list node)

```
typedef struct s_token
{
    char            *value;     // The text: "cat", "|", "file.txt"
    t_token_type    type;       // Type: TOKEN_WORD, TOKEN_PIPE, etc.
    struct s_token  *next;      // Pointer to next token
}   t_token;
```

**Purpose:** Stores individual pieces of the input command.

**Example:**
```
Input: cat < input.txt | grep hello

Token List:
[cat:WORD] → [<:REDIR_IN] → [input.txt:WORD] → [|:PIPE] → 
[grep:WORD] → [hello:WORD] → NULL
```

---

### Redirection Structure (linked list node)

```
typedef struct s_redir
{
    t_token_type    type;       // REDIR_IN, REDIR_OUT, REDIR_APPEND, HEREDOC
    char            *file;      // Filename for redirection
    struct s_redir  *next;      // Pointer to next redirection
}   t_redir;
```

**Purpose:** Stores redirections for a specific command. Each command can have multiple redirections.

**Example:**
```
Command: cat < input.txt > output.txt

Redirection List:
[< input.txt] → [> output.txt] → NULL
```

---

### Command Structure (linked list node)

```
typedef struct s_command
{
    char                **argv;         // Array for execve: ["cat", "-n", NULL]
    t_redir             *redirections;  // Linked list of redirections
    struct s_command    *next;          // Pointer to next command (pipeline)
}   t_command;
```

**Purpose:** Represents one command in a pipeline. Contains arguments and redirections.

**Example:**
```
Command: cat -n < input.txt

Structure:
argv: ["cat", "-n", NULL]
redirections: [< input.txt] → NULL
next: NULL
```

---

### Main Data Structure

```
typedef struct s_data
{
    char        **envp;         // Environment variables (array)
    t_token     *tokens;        // Token list (temporary, freed after parsing)
    t_command   *commands;      // Command list (for execution)
    int         last_exit;      // Last exit code ($?)
}   t_data;
```

**Purpose:** Main container holding all program state.

---

## Complete Example

```
Input: cat < input.txt | grep hello > output.txt

After Tokenization:
tokens: [cat] → [<] → [input.txt] → [|] → [grep] → [hello] → [>] → [output.txt] → NULL

After Parsing:
commands:
  ┌─────────────────────┐         ┌──────────────────────┐
  │ Command 1           │  next   │ Command 2            │  next
  │                     │ ──────> │                      │ ──────> NULL
  │ argv:               │         │ argv:                │
  │  ["cat", NULL]      │         │  ["grep", "hello",   │
  │                     │         │   NULL]              │
  │ redirections:       │         │                      │
  │  [< input.txt]─>NULL│         │ redirections:        │
  │                     │         │  [> output.txt]─>NULL│
  └─────────────────────┘         └──────────────────────┘
```

---
## Linked List Functions

Each data type (token, command, redirection) has its own set of linked list functions following the same pattern:

```
// Pattern: type_action

type *xxx_new(...)              // Create new node
void xxx_add_back(type **list)  // Add node to end of list
type *xxx_last(type *list)      // Get last node in list
void xxx_clear(type **list)     // Free entire list
```

### All Functions

```
// Token functions
t_token   *token_new(char *value, t_token_type type);
void      token_add_back(t_token **list, t_token *new);
t_token   *token_last(t_token *list);
void      token_clear(t_token **list);

// Command functions
t_command *command_new(void);
void      command_add_back(t_command **list, t_command *new);
t_command *command_last(t_command *list);
void      command_clear(t_command **list);

// Redirection functions
t_redir   *redir_new(t_token_type type, char *file);
void      redir_add_back(t_redir **list, t_redir *new);
t_redir   *redir_last(t_redir *list);
void      redir_clear(t_redir **list);
```

---

### Why Separate Functions for Each Type?

**Because each struct is different and requires different cleanup:**

**Token:**
- Has: `char *value` (one string)
- Free: string + node

**Command:**
- Has: `char **argv` (array of strings) + `t_redir *redirections` (another list)
- Free: each string in array + array itself + redirections list + node

**Redirection:**
- Has: `char *file` (one string)
- Free: string + node


**Type safety:** Using specific types prevents bugs:
```
token_add_back(&tokens, new_token);      // Compiler checks types
command_add_back(&tokens, new_cmd);      // Compiler error (good!)
```

If we used generic functions with `void*`, the compiler couldn't catch these mistakes.

---

## Main Loop Flow

```
while (true)
{
    1. readline()                → Get input from user
    2. lexer()                   → Create token list
    3. expand_variables()        → Replace $VAR with values (TODO)
    4. remove_quotes()           → Remove ' and " characters (TODO)
    5. parser()                  → Create command list
    6. executor()                → Execute commands (TODO)
    7. Update $?                 → Store exit code
    8. Free tokens & cmds        → Clean memory
    9. Repeat
}
```

---

## Memory Management

**Ownership:**
- Tokenizer creates tokens → Parser uses → Freed after parsing
- Parser creates commands → Executor uses → Freed after execution
- Strings are duplicated with ft_strdup → Each structure owns its strings

**Cleanup pattern:**

```
tokens = lexer(input);                  // Allocate
expand_variables(tokens, envp, exit);   // Modify in place
remove_quotes(tokens);                  // Modify in place
commands = parser(tokens);              // Allocate
token_clear(&tokens);                   // Free tokens (no longer needed)
execute(commands);                      // Use commands
command_clear(&commands);               // Free commands
```

---

## Code Example: readline_loop()

```
void readline_loop(void)
{
char      *prompt;
t_token   *tokens;
t_command *commands;
while (1)
{
    prompt = readline("$[ 🛸 ]> ");
    if (!prompt)
        break;
    
    // 1. Tokenize
    tokens = lexer(prompt);
    
    // 2. Expand variables
    expand_variables(tokens, data.envp, data.last_exit);
    
    // 3. Remove quotes
    remove_quotes(tokens);
    
    // 4. Parse
    commands = parser(tokens);
    
    // 5. Execute
    // execute(commands);
    
    // Cleanup
    token_clear(&tokens);
    command_clear(&commands);
    free(prompt);
}
}
```

---

## Important Notes

### argv Format
Commands use `char **argv` (NULL-terminated array) for execve compatibility:
```
argv[0] = "cat"
argv[1] = "-n"
argv[2] = NULL
```

### Environment Variables
`envp` comes from main(), system provides it. Used for:
- PATH: finding executables
- HOME: cd builtin
- Passed to execve()

### Error Handling
- TODO...

---

## Tests

A basic custom test suite is included for validating the lexer independently from the full minishell execution.

Currently implemented:
- Lexer tests (`test-lexer`)
- Tokenization, operators, spacing, and basic edge cases
- TODO (other tests... builtins, signals, etc.)

---

### Running Tests

```
make test-lexer
```
Builds the lexer test binary.

```
make run-test-lexer
```
Runs the lexer tests.

```
make run-test-lexer-valgrind
```
Runs the lexer tests under valgrind (using readline suppression).

---

## Signal Handling

### 1. What are signals?

Signals are **asynchronous notifications** sent by the kernel to a process to inform it that an event occurred.

They can be triggered by:
- The user (keyboard shortcuts)
- The operating system
- Other processes (via `kill`)

These are the siganls that need to be implemented in minishell:

**Small example:**

When the user presses **Ctrl + C**:
- The terminal driver sends a `SIGINT` signal
- The kernel delivers it to the foreground process (or process group)
- The process either:
  - Handles it
  - Ignores it
  - Terminates (default behavior)

In a normal program, `SIGINT` usually **kills the process**.  
In a shell, this behavior must be customized.

---

### Signals required by the minishell subject

These are the signals that must be handled according to the subject:

| Signal   | Key combo       |
|----------|-----------------|
| SIGINT   | Ctrl + C        |
| SIGQUIT  | Ctrl + \        |
| EOF      | Ctrl + D        |

Other signals (SIGTERM, SIGKILL, etc.) are not mandatory in the minishell projet.

In a **shell**, signals must be handled differently depending on:
- Whether the shell is **waiting for input**
- Whether a **child process** is running
- Whether the command is a **builtin**

---

### 2. Why signal handling is special in a shell

A shell is a **long-running parent process** that:
- Spawns child processes (`fork`)
- Executes commands (`execve`)
- Must **survive Ctrl+C**, not die
- Must propagate correct **exit codes**

Key rule:

> **The parent shell should not die on SIGINT, but child processes should.**

### Why this rule exists

If the shell did not handle signals specially:
- Pressing Ctrl+C would kill the shell itself
- The user would lose the session

Instead:
- Ctrl+C should **cancel the current command**
- The shell should **print a new prompt**
- Execution should continue normally

That is why signals must be:
- **Custom-handled in the parent**
- **Reset to default in children**

---

#### Parent vs Child behavior

Signals behave differently depending on whether they are received by:
- the **interactive shell (parent)**
- or a **running command (child)**

---

##### Interactive mode (parent shell)

| Signal   | Key combo | Behavior                                   |
|----------|-----------|--------------------------------------------|
| SIGINT   | Ctrl + C  | Cancel current input, display a new prompt |
| SIGQUIT  | Ctrl + \  | Ignored (does nothing)                     |
| EOF      | Ctrl + D  | Exit the shell                             |

---

##### Single command execution (child process)

| Signal   | Key combo | Behavior                   |
|----------|-----------|----------------------------|
| SIGINT   | Ctrl + C  | Terminate command          |
| SIGQUIT  | Ctrl + \  | Quit command (core dumped) |
| SIGTERM  | kill      | Terminate command          |

The exit codes follow POSIX rules:

- `128 + signal_number`

Example:
- SIGINT (2) → exit code `130`
- SIGQUIT (3) → exit code `131`

---

##### Pipeline execution (`cmd1 | cmd2 | cmd3`)

| Signal   | Key combo | Behavior                               |
|----------|-----------|----------------------------------------|
| SIGINT   | Ctrl + C  | Interrupt all commands in the pipeline |
| SIGQUIT  | Ctrl + \  | Quit all commands in the pipeline      |
| SIGTERM  | kill      | Terminate all commands                 |

In pipeline execution, the shell:
- Waits for all children
- Returns the **exit status of the last command**
- Exit codes follow POSIX rules: `128 + signal`

---

### 3. Global signal state

The project uses a global variable to store received signals:

```
volatile sig_atomic_t g_signal_received;
```

#### Why a global variable is required

- Signal handlers cannot receive user data
- Signal handlers cannot safely access complex structures

This variable allows the shell to:
- Detect that a signal occurred
- Defer handling to normal program flow
- Convert signals into correct shell exit codes

#### Why `sig_atomic_t`

- Guarantees **atomic read/write**: the signal value is read or written in a single,
  indivisible operation, so it can never be partially updated while a signal
  interrupts the program.
- Safe to modify inside a signal handler
- Prevents data races during asynchronous execution

#### Why `volatile`

- Prevents compiler optimizations that could break async access
- Ensures the variable is always read from memory

---

### 4. Parent signal handling (interactive shell)

#### 4.1 Signal handler

```
void handle_sigint(int sig)
{
	(void)sig;
	g_signal_received = SIGINT;
	write(STDOUT_FILENO, "\n", 1);
}
```

Important points:
- **Only async-signal-safe functions are used**
- No `malloc`
- No `printf`
- No readline calls

This matches how real shells behave:
- Ctrl+C clears the line
- Shell remains alive

---

### 4.2 About `sigaction`

`sigaction` is the modern, safe way to install signal handlers.

It allows:
- Precise control over signal behavior
- Reliable behavior across systems
- Blocking of other signals during handling

Key fields:
- `sa_handler` → function to call
- `sa_mask` → signals to block while handling
- `sa_flags` → behavior modifiers (`SA_RESTART`, etc.

---

#### Installing parent handlers

```
void setup_signals(void)
{
	struct sigaction sa_int;
	struct sigaction sa_quit;

	g_signal_received = 0;

	ft_memset(&sa_int, 0, sizeof(struct sigaction)); // clear entire sigaction structure to avoid garbage value
	sigemptyset(&sa_int.sa_mask); // initialize an empty signal mask, so no signals are blocked while handling SIGINT
	sa_int.sa_handler = handle_sigint; // assign the custom SIGINT handler 
	sa_int.sa_flags = SA_RESTART; // Restart interrupted system calls (readline..)
	sigaction(SIGINT, &sa_int, NULL);  // Register the SIGINT handler

	ft_memset(&sa_quit, 0, sizeof(struct sigaction));
	sigemptyset(&sa_quit.sa_mask);
	sa_quit.sa_handler = SIG_IGN;  // Ignore SIGQUIT (Ctrl+\ does nothing in interactive shell)
	sigaction(SIGQUIT, &sa_quit, NULL); // Register the SIGQUIT behavior
}
```

Behavior:
- `SIGINT` → custom handler
- `SIGQUIT` → ignored
- `SA_RESTART` → interrupted system calls restart automatically

This configuration is active **only in the parent shell**.

---

### 5. Resetting signals in child processes

#### Why this is required

After `fork()`, the child **inherits signal handlers** from the parent.

If we do nothing:
- Ctrl+C would NOT kill commands like `sleep`
- This would break expected shell behavior

#### Reset function

```
void reset_signals(void)
{
	struct sigaction sa;

	g_signal_received = 0;  // Clear any previously recorded signal
	ft_memset(&sa, 0, sizeof(struct sigaction)); // Reset the structure
	sigemptyset(&sa.sa_mask); // No signals blocked during handling

	// Restore default signal behavior
	// SIGINT → terminat 
	// SIGQUIT →  core dump
	sa.sa_handler = SIG_DFL;

	sigaction(SIGINT, &sa, NULL); // Apply default behavior to SIGINT
	sigaction(SIGQUIT, &sa, NULL); // Apply dafault behavior to SIGQUIT
}
```

This restores:
- `SIGINT` → default (terminate)
- `SIGQUIT` → default (core dump)

---

### Where reset_signals() is called

- In **single command children**
- In **forked builtin execution**
- In **pipeline child processes**

This guarantees:
- External commands behave like in bash
- Signals kill child processes correctly

---

### 6. Signal-aware execution flow

#### 6.1 Single command execution

1. Parent forks
2. Child:
   - Calls `reset_signals()`
   - Applies redirections
   - Executes command
3. Parent:
   - Waits for child
   - Converts signal to exit code

```
if (WIFSIGNALED(status))
	return (128 + WTERMSIG(status));
```

---

#### Pipeline execution

- Each pipeline command runs in its own child
- All children reset signals
- Parent waits only for the **last command**
- Exit code follows bash rules

After waiting:
```
if (check_signal())
	return (get_signal_exit_code());
```

---

### 7. Exit codes and signals

Shell convention:
- Normal exit → return program exit code
- Signal exit → `128 + signal_number`

Examples:

| Action            | Exit code |
|-------------------|-----------|
| Ctrl+C (SIGINT)   | 130       |
| Ctrl+\ (SIGQUIT)  | 131       |

Implementation:

```
int get_signal_exit_code(void)
{
	int code;

	if (g_signal_received == 0)
		return (0);
	code = 128 + g_signal_received;
	g_signal_received = 0;
	return (code);
}
```

---

### 8. Builtins and signals

#### Builtins that must run in the parent

Some builtins **modify shell state**, so running them in a child would lose changes:

- `cd` → changes working directory
- `export` / `unset` → modify environment
- `exit` → terminates shell

```
bool must_run_in_parent(t_command *cmd)
{
	if (ft_strcmp(cmd->argv[0], "cd") == 0)
		return (true);
	if (ft_strcmp(cmd->argv[0], "export") == 0)
		return (true);
	if (ft_strcmp(cmd->argv[0], "unset") == 0)
		return (true);
	if (ft_strcmp(cmd->argv[0], "exit") == 0)
		return (true);
	return (false);
}
```

Other builtins (`echo`, `pwd`, `env`) can safely run in children.

---

### 9. Readline integration

The signal handler **does not touch readline**.

Instead:
- Signals are detected **after readline returns
- Cleanup is done in normal control flow

This avoids undefined behavior and matches GNU Readline requirements.

---
