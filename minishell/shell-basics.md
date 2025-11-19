# The Shell should 

## Display a prompt when waiting for a new command

The shell runs in a loop, waiting for the user to type a command.
It must show a prompt string (e.g., minishell$ ) whenever it is ready to receive user input.
This prompt should appear at startup and after each executed command.
It should not appear while running commands that take control of the terminal (e.g., cat waiting for input), only when returning to the shell loop.

## Have a working history
