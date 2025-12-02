# Minishell Function Map 

============================================================
1. READLINE LIBRARY (Input, Prompt, History)
============================================================

────────────────────────
### readline
Prototype:
```
char *readline(const char *prompt);
```

**Purpose**
Reads a full line from the terminal with editing support, arrow keys and Ctrl shortcuts

What it does:
- Prints the prompt (e.g. "minishell$ ")
- Lets the user type a full editable line
- Supports arrow keys, editing, copy/paste, HOME/END…
- Returns a malloc’d string containing the typed line
- Returns NULL if Ctrl+D is pressed → shell should exit

**Use in minishell:**
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
### rl_clear_history
Prototype:
```
void rl_clear_history(void);
```

**Purpose:**
- Clear the entire command history 

**Use in minishell:**
- Clean up history before exit
- Reset history if needed

────────────────────────
### rl_on_new_line
Prototype:
```
int rl_on_new_line(void);
```

**Purpose:**
- Tells readline the cursor moves to a new line
- Required before redisplay()

**Used in minishell:**
- After printing output from signal handler
- Helps readline redraw the prompt correctly after Ctrl+C

────────────────────────
### rl_replace_line
Prototype:
```
void rl_replace_line(const char *text, int clear_undo);
```

**Purpose:**
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
### rl_redisplay
Prototype:
```
void rl_redisplay(void);
```

**Purpose:**
- Redraws the prompt + current buffer
- Needed after clearing/replacing input

**Used in minishell:**
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

============================================================
3. UNIX I/O (Redirections, File Handling)
============================================================

### write

**Purpose:**
write bytes to file descriptor.

Prototype:
+code
ssize_t write(int fd, const void *buf, size_t count);
+code

**Used in minishell:**
- Output in signal handlers (printf is not signal-safe)
- Write to pipes
- Write to redirected files 

**Example:**
+code
write(STDOUT_FILENO, "hello\n", 6);
write(1, "minishell: error\n", 17);
+code

────────────────────────
### open

**Purpose:** 
Open a file and return a file descriptor.

Prototype
+code
int open(const char *pathname, int flags, mode_t mode);
+code

**Use in minishell:**
- Open files for redirections (<, >, >>)
- Create new files for output redirection

**Flags:**
- O_RDONLY: input redirection (<)
- O_WRONLY | O_CREAT | O_TRUNC: output redirection (>)
- O_WRONLY | O_CREAT | O_APPEND: append redirection (>>)

**Example:**
+code
// Input redirection: < file
int fd = open("input.txt", O_RDONLY);

// Output redirection: > file
int fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

// Append redirection: >> file
int fd = open("output.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
+code


────────────────────────
### read 

**Purpose:**
Read bytes from a file descriptor.

+code
ssize_t read(int fd, void *buf, size_t count);
+code

**Use in minishell:**
- Rarely in minishell unless implementing your own heredoc pipe copy


────────────────────────

### close

**Purpose:** Close a file descriptor.

Prototype: 
+code
int close(int fd);
+code

**Use in minishell:**
- Close files after redirection
- Close pipe ends
- Clean up file descriptors

**Example:**
+code
int fd = open("file.txt", O_RDONLY);
dup2(fd, STDIN_FILENO);
close(fd);  // Always close after dup2
+code


────────────────────────
### access

**Purpose:** Check file permissions and existence.

Prototype
+code
int access(const char *pathname, int mode);
+code

**Use in minishell:**
- Check if a file exists before execution (perfect for PATH resolution)
- Verify execute permissions (X_OK)
- Check read/write permissions for redirections

**Example:**
+code
if (access("/bin/ls", X_OK) == 0)
    // File exists and is executable
if (access(file, F_OK) == 0)
    // File exists
if (access(file, R_OK) == 0)
    // File is readable
+code

────────────────────────
### unlink

**Purpose:** Delete a file.

Prototype
+code
int unlink(const char *pathname);
+code

**Use in minishell:**
- Remove temporary files created for heredoc
- Clean up temporary storage

────────────────────────
