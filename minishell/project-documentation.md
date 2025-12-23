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
    
    // 2. Expand variables (TODO!)
    expand_variables(tokens, data.envp, data.last_exit);
    
    // 3. Remove quotes (TODO!)
    remove_quotes(tokens);
    
    // 4. Parse
    commands = parser(tokens);
    
    // 5. Execute (TODO!)
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
- **Syntax errors:** Use ft_printf (e.g., "syntax error near '|'")
- **System errors:** Use perror (e.g., "cat: No such file or directory")

### Signal Handling (TODO)
Global variable: `volatile sig_atomic_t g_signal`
- Parent: custom SIGINT handler
- Child: SIG_DFL

---

## Tests

A basic custom test suite is included for validating the lexer independently from the full minishell execution.

Currently implemented:
- Lexer tests (`test-lexer`)
- Tokenization, operators, spacing, and basic edge cases

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

### Notes about the tests / TODO

- Tests currently cover only the lexer
- Parser tests are not implemented yet
- Executor tests are not implemented yet
- Additional tests will follow the same structure

---

## Current Status

 **Completed:**
- Basi Tokenizer (lexer)
- Token linked list utilities
- Parser (basic pipes + redirections)
- Command linked list utilities
- Redirection linked list utilities
- Debug print functions

 **TODO:**
- Quote handling (' and ")                                        - in lexer
- Variable expansion (`$VAR`, `$`?)                               - new expansion.c (between lexer and parser)
- Quote removal                                                   - in expansion.c (after variable expansion)
- Execution (fork, pipe, execve)                                  - new executor module
- Built-in commands (echo, cd, pwd, export, unset, env, exit)     - new builtins module
- Signal handling (`Ctrl+C`, `Ctrl+D`, `Ctrl+\`)                  - in main loop + executor
- Heredoc implementation (<<)                                     - in parser + executor
- Error handling improvements                                     - throughout all modules
