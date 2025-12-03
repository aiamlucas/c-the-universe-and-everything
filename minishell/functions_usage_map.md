# 2.5 — MINIMAL FUNCTION SET REQUIRED FOR EACH PART

## PART 1 — INPUT HANDLING (readline, signals, history)

• readline()  
• add_history()  
• rl_on_new_line()  
• rl_replace_line()  
• rl_redisplay()  
• signal() OR sigaction()  
• tcgetattr(), tcsetattr() (TTY behavior)  
• isatty()  
• write()   ← only for safe signal output  
• exit()

Used for:
- Reading user input
- Handling Ctrl+C and Ctrl+D
- Managing history
- Updating prompt after signals
- Detecting interactive TTY


## PART 2 — LEXER (tokenizer)

• write() (only for error output)  
• strerror(), perror() (optional)  
• getenv() (only to detect $?)  
• access() (optional, during validation)

Used for:
- Producing error messages
- Reading environment variables
- Detecting special tokens


## PART 3 — PARSER (building command structures)

• No system calls required here  
(Only struct creation, string manipulation, malloc, free)


## PART 4 — EXPANSION ($VAR, $? handling)

• getenv()  
• getenv("?") (simulated via stored exit status)  
• write() (optional error output)

Used for:
- Expanding environment variables
- Expanding last exit status $?
- Handling quotes properly


## PART 5 — QUOTE REMOVAL

• No system calls  
(Only string operations)


## PART 6 — REDIRECTIONS (<, >, >>, <<)

• open()  
• close()  
• dup()  
• dup2()  
• access()  
• unlink() (for removing temporary heredoc files)  
• write() (for error messages)  
• read()  (only if doing heredoc manually)  

Used for:
- Opening input/output files
- Appending to files
- Creating temporary heredoc storage
- Redirecting STDIN/STDOUT
- Applying redirections before execve()


## PART 6.4 — HEREDOC (<<)

• readline()  
• write()  
• open() or pipe() for temporary storage  
• dup2()  
• close()  
• unlink() (remove temp file)  

Used for:
- Reading user input until LIMITER
- Expanding or not expanding variables
- Feeding heredoc content into command STDIN


## PART 7 — EXECUTION (builtins + external)

• fork()  
• execve()  
• waitpid()  
• wait3(), wait4() (optional)  
• access()  
• getcwd()  
• chdir()  
• exit()  
• stat(), lstat(), fstat() (optional for path handling)

Used for:
- Creating child processes
- Executing external programs
- Running builtins (sometimes without forking)
- Getting and changing working directory
- PATH lookup and validation


## PART 8 — PIPES (| operator)

You will use:

• pipe()  
• dup2()  
• close()  
• fork()  
• execve()  
• waitpid()  

Used for:
- Creating pipeline between commands
- Duplicating pipe ends into STDIN/STDOUT
- Running each command in its own process


## PART 9 — EXIT STATUS MANAGEMENT

You will use:

• waitpid()  
• WIFEXITED(), WEXITSTATUS()  
• WIFSIGNALED(), WTERMSIG()  
• kill()  
• signal() / sigaction()  

Used for:
- Reading child exit codes
- Detecting signals (Ctrl+C, Ctrl+\)
- Updating the $? variable


## PART 10 — MAIN LOOP

You will use:

• readline()  
• add_history()  
• signal() / sigaction()  
• getenv()  
• exit()  
• isatty()  
• ttyname(), ttyslot() (optional)  
• tgetent(), tgetstr(), tgoto(), tputs() (optional for bonus prompt)

Used for:
- The shell loop structure
- Prompt handling
- Input reading
- Logging commands to history
- Exiting properly


# TABLE

PART             | FUNCTIONS USED
-----------------|---------------------
1 Input handling | readline(), add_history(), signal(), sigaction(), rl_on_new_line(), rl_replace_line(), write(), exit(), isatty(), tcgetattr(), tcsetattr()
2 Lexing         | write(), strerror(), perror(), getenv(), access()
3 Parsing        | (no system calls)
4 Expansion      | getenv(), write()
5 Quote removal  | (no system calls)
6 Redirections   | open(), close(), dup(), dup2(), access(), unlink(), read(), write()
6.4 Heredoc      | readline(), open() or pipe(), write(), dup2(), close(), unlink()
7 Execution      | fork(), execve(), waitpid(), access(), getcwd(), chdir(), exit(), stat()
8 Pipes          | pipe(), dup2(), close(), fork(), execve(), waitpid()
9 Exit status    | waitpid(), WIFEXITED(), WEXITSTATUS(), WIFSIGNALED(), WTERMSIG(), kill(), signal(), sigaction()
10 Main loop     | readline(), add_history(), signal(), sigaction(), getenv(), exit(), isatty()
