# Minishell Allowed Functions

## 1. INPUT HANDLING & HISTORY (Chapter 1)

────────────────────────
### readline
Prototype:
```
char *readline(const char *prompt);
```

What it does:
- Prints the prompt (e.g. "minishell$ ")
- Lets the user type a full editable line
- Supports arrow keys, editing, copy/paste, HOME/END…
- Returns a malloc’d string containing the typed line
- Returns NULL if Ctrl+D is pressed → shell should exit

Why minishell needs it:
- This replaces manual reading using read()
- Handles interactive behavior for you
- Makes minishell feel like bash

**Example:**
```
char *line = readline("minishell$ ");
if (!line)
    exit(0);    // Ctrl+D pressed
```

Used in:
- Main loop
- Heredoc (reading limiter lines)


────────────────────────
### add_history
Prototype:
```
void add_history(const char *line);
```

What it does:
- Saves a command line to readline’s history
- Lets the user press ↑ to revisit past commands

Important notes:
- You must call it **after** checking the line is non-empty
- NEVER store heredoc input lines here

**Example:**
```
if (line && *line)
    add_history(line);
```

Used in:
- After reading a valid user command


────────────────────────
### rl_replace_line
Prototype:
```
void rl_replace_line(const char *text, int clear_undo);
```

Purpose:
- Replace the curret line buffer with new text
- Useful for Ctrl+C behavior

Example:
```
rl_replace_line("", 0);      // clears the current line
rl_on_new_line();
rl_redisplay();
```

Used in:
- Clear the input line whe Ctrl+C is pressed
- Can be used to reset the input buffer


────────────────────────
### rl_on_new_line
Prototype:
```
int rl_on_new_line(void);
```

Purpose:
- Tells readline the cursor moves to a new line
- Required before redisplay()

Used in:
- After printing output from signal handler
- Helps readline redraw the prompt correctly after Ctrl+C


────────────────────────
### rl_redisplay
Prototype:
```
void rl_redisplay(void);
```

Purpose:
- Redraws the prompt + current buffer
- Needed after clearing/replacing input

Used in:
- After signal handler modifies the display
- Refresh prompt after Ctrl+
- Often used together with rl_on_new_line

**Example (typical signal handler pattern):**
```
void handle_sigint(int sig)
{
	g_signal = sig;
	write(1, "\n", 1);
	rl_on_new_line();
	rl_replace_line("", 0);
	rl_redisplay();
}
```


────────────────────────
### rl_clear_history
Prototype:
```
void rl_clear_history(void);
```

Purpose:
- Frees history list internally

Used in:
- Clean up history before exit
- Reset history if needed


## 2. BASIC I/O & MEMORY  (Used Everywhere)

────────────────────────
### write
Prototype:
```
ssize_t write(int fd, const void *buf, size_t count);
```

Purpose:
- write bytes to a file descriptor
- Async-signal-safe → safe inside handlers

**Used in minishell:**
- Output in signal handlers (prinft is not signal-safe) 
- Write pipes
- Write to redirect files 

**Example (Ctrl+C handler):**
```
write(1, "\nminishell$ ", 12);
```

────────────────────────

## 3. FILESYSTEM ACCESS (Redirections - Chapter 6)

────────────────────────
### open
Prototype:
```
int open(const char *pathname, int flags, mode_t mode);
```

**Purpose:**
- Open a file and return a file descriptor

**Use in minishell:**:
- Open files for redirection (`<`, `>`, `>>`)
- Create new files for output redirection

Important flags:
- O_RDONLY: input redirection (`file <`)
- O_WRONLY | O_CREAT | O_TRUNC → output redirection (`> file`)
- O_WRONLY | O_CREAT | O_APPEND → append redirection (`>> file`)

Common errors minishell must handle:
- Permission denied
- No such file
- Directory instead of file

**Example:**
```
// Input redirection: < file
int fd = open("input.txt", O_RDONLY);

// Output redirection: > file
int fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

// Append redirection: >> file
int fd = open("output.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
```

────────────────────────
### read
Prototype:
```
ssize_t read(int fd, void *buf, size_t count);
```

**Purpose:**
- Read bytes from a file descriptor

**Use in minishell:**
- Read from files (for < redirection)
- Read from pipes 
- Not used for normal command input (readline handles that)


────────────────────────
### access
Prototype:
```
int access(const char *pathname, int mode);
```

**Purpose:**
- Check file permissions and existence

**Use in minishell:**
- Check if a file exists before execution
- Verify execute permissions (X_OK)
- Check read/write permissions for redirections

**Example:**
```
if (access("/bin/ls", x_OK) == 0)
	// File exists and is exacutable
if (acess(file, F_OK) == 0)
	// File exists
if (acess(file, R_OK) == 0)
	// File is readable
```

**Modes:**
- F_OK → file exists
- X_OK → executable permission

**Used in:**
- PATH resolution before execve


────────────────────────
### close
Prototype:
```
int close(int fd);
```

Purpose:
- Close file descriptors
- Very important in pipes to avoid FD leaks

Used in:
- After dup2()
- After creating pipes
- Redirection cleanup


────────────────────────
### unlink
Prototype:
```
int unlink(const char *pathname);
```

Purpose:
- Deletes a file

Used in:
- Heredoc temporary file cleanup


────────────────────────
### stat / lstat / fstat
Prototypes:
```
int stat(const char *path, struct stat *buf);
int lstat(const char *path, struct stat *buf);
int fstat(int fd, struct stat *buf)*
