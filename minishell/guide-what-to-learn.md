# Minishell

- Learn how to display a prompt.

- Learn how to use readline() to:
  - show a prompt
  - read user input
  - keep history

- Learn how PATH works and how shells find executables.

- Learn how to execute commands using execve().

- Learn what a global variable for signals is (just one int).

- Learn how to write a signal handler for:
  - SIGINT (ctrl-C)
  - SIGQUIT (ctrl-\)

- Learn why the signal handler must NOT touch your program data.

- Learn how to detect and reject:
  - unclosed quotes
  - unsupported characters like \ or ;

- Learn how single quotes work: 'text' → literal (no expansion)

- Learn how double quotes work:
  - "text" → expand variables
  - "$VAR" works, "$?" works

- Study redirections:
  - <  (input)
  - >  (output, truncate)
  - >> (output, append)
  - << (heredoc until delimiter)

- Learn how to expand environment variables:
  - $VAR
  - $?

- Learn interactive mode behavior:
  - ctrl-C → new prompt
  - ctrl-D → exit shell
  - ctrl-\ → nothing

- Learn how bash handles these keys while waiting for input.

- Study the required builtins:
  - echo (-n)
  - cd (path only)
  - pwd
  - export
  - unset
  - env
  - exit

- Learn how to manage memory (no leaks except readline's own).

- If unsure how something works → test it in bash.
