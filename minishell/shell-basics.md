# The Shell should 

## Display a prompt when waiting for a new command

- The shell runs in a loop, waiting for the user to type a command.
- It must show a prompt string (e.g., minishell$ ) whenever it is ready to receive user input.
- This prompt should appear at startup and after each executed command.
- It should not appear while running commands that take control of the terminal (e.g., cat waiting for input), only when returning to the shell loop.

## Have a working history

## Search and launch the right executable (based on the PATH variable or using a relative or an absolute path)

- The shell must determine the correct executable to run based on the PATH environment variable.
- If the user provides a relative or absolute path (e.g., ./script or /bin/ls), the shell must execute the file directly.
- If the command is not a builtin, minishell must search each directory in PATH until it finds a matching executable.
  - For example, running ``` cat ``` should cause the shell to search the PATH directories (e.g., /bin, /usr/bin) until it finds the executable (typically /bin/cat).
- If the executable cannot be found or cannot be executed, the shell must display an appropriate error message.
- The program should use system calls such as ``` access() ``` and ``` execve() ``` to validate and execute commands.

## Use at most one global variable to indicate a received signal

- The shell may use only **one** global variable, and it must be used exclusively to store the number of the received signal.
  - This variable can be of type ``` volatile sig_atomic_t ```, which is safe to modify inside a signal handler.
- This global variable must **not** store any additional information and must not provide access to other data or program state.
- The signal handler must not interact with minishell’s internal data structures; it should only update this single global variable.
- Global structures, arrays, pointers, or multiple global variables are forbidden, as they would allow the handler to access internal program state.
