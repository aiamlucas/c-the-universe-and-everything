# Minishell Function Map 

## 1. READLINE LIBRARY (Input, Prompt, History)

────────────────────────

### readline

Prototype:
```
char *readline(const char *prompt);
```

**Purpose**
Reads a full line from the terminal with editing support, arrow keys and Ctrl shortcuts

**What it does:**
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

**Where it can be used:**
- Main loop
- Heredoc (reading limiter lines)

────────────────────────

### add_history

Prototype:
```
void add_history(const char *line);
```

**Purpose**
Add a line to the command history

**Use in minishell:**
- Save non-empty commands to History
- Allow user to access previous commands with UP/DOWN arrows

**Important notes:**
- You must call it **after** checking the line is non-empty
- NEVER store heredoc input lines here

**Example:**
```
if (line && *line)
    add_history(line);
```

**Where it can be used:**
- After reading a valid user command

────────────────────────

### rl_clear_history

Prototype:
```
void rl_clear_history(void);
```

**Purpose:**
Clear the entire command history 

**Use in minishell:**
- Clean up history before exit
- Reset history if needed

**Where it is used:**  
- Right before exit to free memory (not required but clean).

**Example:**
```
rl_clear_history();
```

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

**Where it is used:**  
- When Ctrl+C is pressed at the prompt.

**Example (signal handler):**
```
rl_on_new_line();
```

────────────────────────

### rl_replace_line

Prototype:
```
void rl_replace_line(const char *text, int clear_undo);
```

**Purpose:**
- Replace the curret line buffer with new text
- Useful for Ctrl+C behavior


**Used in minishell:**
- Clear the input line whe Ctrl+C is pressed
- Can be used to reset the input buffer

**Where it is used:**  
- Inside SIGINT handler for the parent shell.

Example:
```
rl_replace_line("", 0);      // clears the current line
rl_on_new_line();
rl_redisplay();
```

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

**Where it is used:**  
- SIGINT handler (interactive mode)

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

# 2. UNIX I/O (Redirections, File Handling)

### write

Prototype:
```
ssize_t write(int fd, const void *buf, size_t count);
```

**Purpose:**
write bytes to file descriptor.

**Used in minishell:**
- Output in signal handlers (printf is not signal-safe)
- Write to pipes
- Write to redirected files 

**Where it is used:**  
- Redirection handling  
- Pipelines  
- Error printing in execve failures  
- Signal-safe output in SIGINT/SIGQUIT handlers 

**Example:**
```
write(STDOUT_FILENO, "hello\n", 6);
write(1, "minishell: error\n", 17);
```

────────────────────────

### open

Prototype
```
int open(const char *pathname, int flags, mode_t mode);
```

**Purpose:** 
Open a file and return a file descriptor.

**Use in minishell:**
- Open files for redirections (<, >, >>)
- Create new files for output redirection

**Where it is used:**  
- Redirection engine  
- Heredoc creation  
- PATH resolution checks (optional but useful)  

**Flags:**
- O_RDONLY: input redirection (<)
- O_WRONLY | O_CREAT | O_TRUNC: output redirection (>)
- O_WRONLY | O_CREAT | O_APPEND: append redirection (>>)

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
Read bytes from a file descriptor.

**Use in minishell:**  
- Rare in mandatory minishell
- Could be used for:
  - Custom heredoc implementation (reading from tmp file)
  - Copying data manually between fds (not required)
  - Reading from pipes (but usually handled by execve’d programs)

**Where it is used:**  
- Optional in heredoc implementation  
- Optional in pipe/manual file operations  

**Example:**
```
char buffer[1024];
ssize_t n = read(fd, buffer, sizeof(buffer));
if (n > 0)
    write(STDOUT_FILENO, buffer, n);
```

────────────────────────

### close

Prototype:
```
int close(int fd);
```

**Purpose:**  
Close a file descriptor.

**Use in minishell:**  
- Closing files after redirection
- Closing unused pipe ends
- Preventing FD leaks
- Cleanup before exiting child processes

**Where it is used:**  
- After every open()
- After dup2()
- During pipeline setup
- After fork() in children & parent

**Example:**
```
int fd = open("file.txt", O_RDONLY);
dup2(fd, STDIN_FILENO);
close(fd); // Always close after dup2()
```

────────────────────────

### access

Prototype:
```
int access(const char *pathname, int mode);
```

**Purpose:**  
Check if a file exists or if the user has permission to access it.

**Use in minishell:**  
- PATH resolution: checking executability of each candidate file
- Detect “permission denied” before execve
- Checking read/write permissions on redirection files

**Where it is used:**  
- PATH resolution engine  
- Before executing external commands  
- Before redirection (<, >, >>)

**Example:**
```
if (access("/bin/ls", X_OK) == 0)
    printf("ls is executable\n");

if (access("file.txt", F_OK) == 0)
    printf("file exists\n");
```

────────────────────────

### unlink

Prototype:
```
int unlink(const char *pathname);
```

**Purpose:**  
Delete (remove) a file.

**Use in minishell:**  
- Remove temporary heredoc files (if using real files)  
- Cleanup after errors  
- Implementing safe tmp-file lifecycle

**Where it is used:**  
- Heredoc temporary file deletion  
- Cleanup routines  

**Example:**
```
unlink("/tmp/minishell_heredoc_42");
```

────────────────────────

# 3. DIRECTORY HANDLING (Navigation & env/pwd)

### opendir

Prototype:
```
DIR *opendir(const char *name);
```

**Purpose:**  
Open a directory stream for reading file entries.

**Use in minishell:**  
- Optional for debugging / listing dirs
- Useful in your own implementation of `env` or `ls` (not required)

**Where it is used:**  
- Rarely used in mandatory minishell  

**Example:**
```
DIR *d = opendir(".");
if (!d)
    perror("opendir");
```

────────────────────────

### readdir

Prototype:
```
struct dirent *readdir(DIR *dirp);
```

**Purpose:**  
Read the next entry in a directory stream.

**Use in minishell:**  
- Also optional (not required by the project)
- Could help if making your own commands or debugging

**Where it is used:**  
- Normally not used in mandatory minishell

**Example:**
```
struct dirent *ent;
while ((ent = readdir(d)))
    printf("%s\n", ent->d_name);
```

────────────────────────

### closedir

Prototype:
```
int closedir(DIR *dirp);
```

**Purpose:**  
Close a directory stream opened by opendir().

**Use in minishell:**  
- Only if using opendir/readdir

**Where it is used:**  
- Normally not used in mandatory minishell

**Example:**
```
closedir(d);
```

────────────────────────

### chdir

Prototype:
```
int chdir(const char *path);
```

**Purpose:**  
Change the current working directory.

**Use in minishell:**  
- Implementing the `cd` builtin
- Must update $PWD and $OLDPWD

**Where it is used:**  
- Builtin: **cd**

**Example:**
```
if (chdir("/tmp") == -1)
    perror("chdir");
```

────────────────────────

### getcwd

Prototype:
```
char *getcwd(char *buf, size_t size);
```

**Purpose:**  
Get the current working directory path.

**Use in minishell:**  
- Implement `pwd` builtin (allowed to use getcwd)
- Update PWD and OLDPWD during cd

**Where it is used:**  
- Builtin: **pwd**
- Builtin: **cd** (for updating environment variables)

**Example:**
```
char buf[PATH_MAX];
if (getcwd(buf, sizeof(buf)))
    printf("%s\n", buf);
```

────────────────────────

# 4. FILE METADATA (stat, lstat, fstat)

These functions retrieve detailed information about files.  
Minishell mainly uses them to detect **“Is this a directory?”** or **“Does the file exist?”** before executing.

────────────────────────
### stat

Prototype:
```
int stat(const char *pathname, struct stat *statbuf);
```

**Purpose:**  
Get information about a file (type, permissions, size, etc.) by following symlinks.

**Use in minishell:**  
- Check whether a command path is a **directory**  
- Before execve:  
  - If the “command” is a directory → return exit status 126  
- Detect file existence

**Where it is used:**  
- PATH resolution  
- Detect "Is a directory" error  
- Validate executable files

**Example:**
```
struct stat st;
if (stat("/bin/ls", &st) == 0) {
    if (S_ISDIR(st.st_mode))
        printf("ls is a directory?!\n");
}
```

────────────────────────
### lstat

Prototype:
```
int lstat(const char *pathname, struct stat *statbuf);
```

**Purpose:**  
Same as stat(), but **does NOT follow symlinks**.

**Use in minishell:**  
- Rarely needed  
- Useful if implementing detailed error behavior (like bash's symlink checks)

**Where it is used:**  
- Optional for advanced correctness  
- Not required in basic minishell

**Example:**
```
struct stat st;
lstat("linkfile", &st);
if (S_ISLNK(st.st_mode))
    printf("it's a symlink!\n");
```

────────────────────────

### fstat

Prototype:
```
int fstat(int fd, struct stat *statbuf);
```

**Purpose:**  
Get file metadata from an **open file descriptor**.

**Use in minishell:**  
- Optional  
- Useful when working with pipes or heredocs  
- Rare in mandatory part

**Where it is used:**  
- Advanced pipe/FD analysis (not required)

**Example:**
```
int fd = open("a.txt", O_RDONLY);
struct stat st;
fstat(fd, &st);
if (S_ISREG(st.st_mode))
    printf("regular file\n");
```


5. PROCESS CONTROL (fork, execve, wait…)

This is the **core** of executing external programs and pipelines.

────────────────────────

### fork

Prototype:
```
pid_t fork(void);
```

**Purpose:**  
Create a new child process.

**Use in minishell:**  
- Running external commands  
- Executing builtins **inside pipelines**  
- Creating one child per command in a pipeline

**Where it is used:**  
- Execution engine  
- Pipeline creation

**Example:**
```
pid_t pid = fork();
if (pid == 0)
    execve(...);  // child
else
    waitpid(pid, NULL, 0); // parent
```

────────────────────────

### execve

Prototype:
```
int execve(const char *pathname, char *const argv[], char *const envp[]);
```

**Purpose:**  
Replace the current process image with a new program.

**Use in minishell:**  
- Run external programs  
- Called only from the **child process** after fork  
- never returns unless failed

**Where it is used:**  
- External command execution  
- Every pipeline command

**Example:**
```
execve("/bin/ls", argv, envp);
perror("execve");  // only if failed
exit(126);
```

────────────────────────

### wait / waitpid

Prototypes:
```
pid_t wait(int *wstatus);
pid_t waitpid(pid_t pid, int *wstatus, int options);
```

**Purpose:**  
Wait for child processes to finish.

**Use in minishell:**  
- Get exit status for `$?`  
- Wait for pipeline children  
- Wait for standalone external commands  
- Detect if child was terminated by a **signal** (SIGINT, SIGQUIT)

**Where it is used:**  
- Execution  
- Pipelines  
- Exit status handling

**Example:**
```
int status;
waitpid(pid, &status, 0);
if (WIFEXITED(status))
    printf("exit code = %d\n", WEXITSTATUS(status));
```

────────────────────────

### wait3 / wait4

Prototype:
```
pid_t wait3(int *status, int options, struct rusage *rusage);
pid_t wait4(pid_t pid, int *status, int options, struct rusage *rusage);
```

**Purpose:**  
Like waitpid, but also returns resource usage.

**Use in minishell:**  
- Fully optional  
- You can ignore resource usage  
- Allowed by subject but not needed

**Where it is used:**  
- Rare; not required for minishell behavior

**Example:**
```
wait3(NULL, 0, NULL);
```


────────────────────────

### kill

Prototype:
```
int kill(pid_t pid, int sig);
```

**Purpose:**  
Send a signal to a process.

**Use in minishell:**  
- Rare  
- Could be used to send SIGINT to children (not necessary, system does it)

**Where it is used:**  
- Optional signal control

**Example:**
```
kill(pid, SIGINT);
```


────────────────────────

### exit

Prototype:
```
void exit(int status);
```

**Purpose:**  
Terminate the current process.

**Use in minishell:**  
- Exiting the child process after execve fails  
- Exiting the parent when builtin `exit` is executed  

**Where it is used:**  
- Builtin: exit  
- Child cleanup

**Example:**
```
if (error)
    exit(1);
```

────────────────────────

# 6. SIGNAL HANDLING  (signal, sigaction, sigemptyset, sigaddset)

Signal handling is CRUCIAL in minishell because bash-like behavior must be respected.

────────────────────────

### signal

Prototype:
```
void (*signal(int signum, void (*handler)(int)))(int);
```

**Purpose:**  
Set a simple signal handler for a given signal.

**Use in minishell:**  
- Often used for *child process* signals (reset to default)
- Sometimes used for quick setups, but **sigaction is preferred**

**Where it is used:**  
- Setting SIGINT and SIGQUIT behavior in child processes  
- Optional for parent, but not fully correct

**Example:**
```
signal(SIGINT, SIG_DFL);   // child: default CTRL+C behavior
signal(SIGQUIT, SIG_IGN);  // parent: ignore CTRL+\
```

────────────────────────

### sigaction

Prototype:
```
int sigaction(int signum, const struct sigaction *act,
              struct sigaction *oldact);
```

**Purpose:**  
Install a **robust** signal handler (recommended over signal()).

**Use in minishell:**  
- Correct handling of Ctrl+C and Ctrl+\ in interactive mode  
- Must update global variable g_signal  
- Ensures no undefined behavior

**Where it is used:**  
- Input handling  
- Main interactive loop

**Example:**
```
void handler(int sig)
{
    g_signal = sig;
}

struct sigaction sa;
sa.sa_handler = handler;
sigemptyset(&sa.sa_mask);
sa.sa_flags = 0;

sigaction(SIGINT, &sa, NULL);   // custom handler
sigaction(SIGQUIT, SIG_IGN, NULL); // ignore
```

────────────────────────

### sigemptyset

Prototype:
```
int sigemptyset(sigset_t *set);
```

**Purpose:**  
Initialize an empty signal set.

**Use in minishell:**  
- Required when setting up sigaction  
- Ensures no signals are blocked during handler execution

**Where it is used:**  
- Only inside sigaction setup

**Example:**
```
sigset_t mask;
sigemptyset(&mask);
sa.sa_mask = mask;
```

────────────────────────

### sigaddset

Prototype:
```
int sigaddset(sigset_t *set, int signum);
```

**Purpose:**  
Add a signal to the set (block during handler execution).

**Use in minishell:**  
- Rarely needed  
- You can add SIGINT to the mask to block nested interrupts

**Where it is used:**  
- Optional inside sigaction

**Example:**
```
sigaddset(&sa.sa_mask, SIGINT);
```

────────────────────────

# 7. TERMINAL & TTY (isatty, ttyname, ttyslot, ioctl,
                     tcsetattr, tcgetattr,
                     tgetent, tgetflag, tgetnum, tgetstr, tgoto, tputs)

These functions are used by **readline** internally, but minishell may also rely on them.

────────────────────────

### isatty

Prototype:
```
int isatty(int fd);
```

**Purpose:**  
Check if a file descriptor is connected to a terminal.

**Use in minishell:**  
- Detect interactive mode (prompt only if stdin is a terminal)
- Avoid prompt when reading from a script or pipe

**Where it is used:**  
- Input & prompt logic

**Example:**
```
if (isatty(STDIN_FILENO))
    printf("minishell$ ");
```

────────────────────────

### ttyname

Prototype:
```
char *ttyname(int fd);
```

**Purpose:**  
Return the terminal device name.

**Use in minishell:**  
- Rare  
- Used for debugging or terminal checks

**Where it is used:**  
- Not required in mandatory part

**Example:**
```
printf("tty: %s\n", ttyname(STDIN_FILENO));
```

────────────────────────

### ttyslot

Prototype:
```
int ttyslot(void);
```

**Purpose:**  
Get the index of the controlling terminal.

**Use in minishell:**  
- Not required  
- Used internally by some shells

**Where it is used:**  
- Optional (not mandatory)

**Example:**
```
int slot = ttyslot();
```

────────────────────────

### ioctl

Prototype:
```
int ioctl(int fd, unsigned long request, ...);
```

**Purpose:**  
Perform low-level terminal control operations.

**Use in minishell:**  
- Almost never needed directly  
- readline handles terminal modes for you

**Where it is used:**  
- Very advanced features  
- Not needed for minishell

**Example:**
```
ioctl(0, TIOCGWINSZ, &ws);
```

────────────────────────

### tcsetattr / tcgetattr

Prototypes:
```
int tcgetattr(int fd, struct termios *termios_p);
int tcsetattr(int fd, int optional_actions, const struct termios *termios_p);
```

**Purpose:**  
Get/set terminal flags (canonical mode, echo, etc.)

**Use in minishell:**  
- readline already manages terminal modes  
- Some people use these to disable control character echoing  
  (like disabling "^C" printing)

**Where it is used:**  
- Optional improvements (bonus)

**Example:**
```
struct termios t;
tcgetattr(STDIN_FILENO, &t);
t.c_lflag &= ~ECHOCTL;  // disable ^C printing
tcsetattr(STDIN_FILENO, TCSANOW, &t);
```

────────────────────────

### tgetent / tgetflag / tgetnum / tgetstr / tgoto / tputs

These come from **termcap** and are mostly used when implementing your own readline.

**Prototype examples:**
```
int tgetent(char *bp, const char *term);
char *tgetstr(const char *id, char **area);
int tputs(const char *str, int affcnt, int (*putc)(int));
```

**Purpose:**  
Terminal capability querying (colors, cursor movement, etc.)

**Use in minishell:**  
- Not required  
- readline already handles all terminal capabilities  
- Used only if you build your OWN line editor (not required)

**Where it is used:**  
- Bonus project (optional)

**Example:**
```
tgetent(NULL, "xterm-256color");
char *clr = tgetstr("*
```

────────────────────────

# 8. ENVIRONMENT & DIRECTORY FUNCTIONS

These functions manage the environment, working directory, or retrieve environment variables.

────────────────────────

### getenv

Prototype:
```
char *getenv(const char *name);
```

**Purpose:**  
Retrieve the value of an environment variable.

**Use in minishell:**  
- Used to build environment copies  
- Needed for PATH resolution  
- Used before execve to attach envp  
- Implementing `echo $VAR`, `export`, etc.

**Where it is used:**  
- Expansion subsystem  
- PATH search  
- Builtin `env`, `export`

**Examples:**
```
char *home = getenv("HOME");
printf("HOME=%s\n", home);
```

────────────────────────

### getcwd

Prototype:
```
char *getcwd(char *buf, size_t size);
```

**Purpose:**  
Returns the current working directory.

**Use in minishell:**  
- Implementing `pwd` builtin  
- Used inside `cd` to update PWD/OLDPWD  
- Useful for debugging internal shell state

**Where it is used:**  
- Builtins (`cd`, `pwd`)  
- Optionally inside startup

**Example:**
```
char path[1024];
getcwd(path, sizeof(path));
printf("Current dir: %s\n", path);
```

────────────────────────

### chdir

Prototype:
```
int chdir(const char *path);
```

**Purpose:**  
Change the current working directory.

**Use in minishell:**  
- Implementing `cd` builtin  
- Must update PWD/OLDPWD

**Where it is used:**  
- Only inside the `cd` builtin

**Example:**
```
if (chdir("/tmp") == -1)
    perror("cd");
```

────────────────────────

# 10. PROCESS MANAGEMENT FUNCTIONS

Core process control functions used to execute external commands and pipelines.

────────────────────────

### fork

Prototype:
```
pid_t fork(void);
```

**Purpose:**  
Create a new process (child).

**Use in minishell:**  
- Used for *every external command*
- Used for *every command in a pipeline* (even builtins)
- Parent waits, child runs execve()

**Where it is used:**  
- Execution engine  
- Pipeline handling

**Example:**
```
pid_t pid = fork();
if (pid == 0)
    execve("/bin/ls", argv, envp);
else
    wait(NULL);
```

────────────────────────

### execve

Prototype:
```
int execve(const char *pathname, char *const argv[], char *const envp[]);
```

**Purpose:**  
Replace current process with a new executable.

**Use in minishell:**  
- Running external commands
- Requires PATH resolution beforehand
- Never returns unless error occurs

**Where it is used:**  
- Child process execution  
- After redirections applied

**Example:**
```
execve("/bin/echo", (char*[]){"echo", "hello", NULL}, envp);
perror("execve");   // only printed if execve fails
```

────────────────────────

### wait / waitpid

Prototypes:
```
pid_t wait(int *wstatus);
pid_t waitpid(pid_t pid, int *wstatus, int options);
```

**Purpose:**  
Wait for child processes to finish and get their exit status.

**Use in minishell:**  
- Wait for a single external command  
- Wait for all children in a pipeline  
- Update $? with WEXITSTATUS or signal code

**Where it is used:**  
- Execution phase  
- Exit status loop

**Example:**
```
int status;
waitpid(pid, &status, 0);
if (WIFEXITED(status))
    g_last_exit = WEXITSTATUS(status);
```

────────────────────────

### wait3 / wait4

Prototypes:
```
pid_t wait3(int *wstatus, int options, struct rusage *rusage);
pid_t wait4(pid_t pid, int *wstatus, int options, struct rusage *rusage);
```

**Purpose:**  
Like waitpid(), but also returns resource usage info.

**Use in minishell:**  
- Not required  
- Optional alternative to waitpid()

**Where it is used:**  
- Not necessary for mandatory part

**Example:**
```
wait3(NULL, 0, NULL);
```

────────────────────────

### kill

Prototype:
```
int kill(pid_t pid, int sig);
```

**Purpose:**  
Send a signal to a process.

**Use in minishell:**  
- Very rarely needed  
- Could be used to forward signals during pipelines (optional)

**Where it is used:**  
- Bonus only (job control)

**Example:**
```
kill(pid, SIGINT);
```

────────────────────────

### exit

Prototype:
```
void exit(int status);
```

**Purpose:**  
Terminate the current process.

**Use in minishell:**  
- Child exits after execve fails  
- Builtin `exit` must terminate the shell (in parent)

**Where it is used:**  
- Builtin exit  
- Child execution cleanup

**Example:**
```
exit(1);
```
