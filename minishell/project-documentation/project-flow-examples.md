## input: pwd

```
readline_loop()
    │
    └── process_input("pwd", data)
            │
            ├── lexer()         → [pwd] → NULL
            ├── expand_tokens() → [pwd] → NULL  (nothing to expand)
            ├── remove_quotes() → [pwd] → NULL  (no quotes)
            ├── parser()        → cmd1: argv=["pwd", NULL], redir=NULL
            ├── token_clear()
            ├── process_all_heredocs() → 0  (no heredocs)
            └── execute_command()
                    │
                    └── execute_single_command()
                            │
                            ├── is_builtin("pwd")      → true
                            ├── must_run_in_parent()   → false
                            ├── cmd->redirections      → NULL
                            └── execute_builtin()
                                    │
                                    └── builtin_pwd()
                                            getcwd() → "/home/user"
                                            ft_printf("/home/user\n")
                                            free(cwd)
                                            return 0
                                    ↑
                            exit_code = 0
                    ↑
            exit_code = 0
            command_clear()
            check_signal() → false
            return 0
    ↑
data->last_exit = 0
```
---

## input: /bin/ls
```
readline_loop()
    │
    └── process_input("/bin/ls", data)
            │
            ├── lexer()         → [/bin/ls] → NULL
            ├── expand_tokens() → [/bin/ls] → NULL
            ├── remove_quotes() → [/bin/ls] → NULL
            ├── parser()        → cmd1: argv=["/bin/ls", NULL], redir=NULL
            ├── token_clear()
            ├── process_all_heredocs() → 0
            └── execute_command()
                    │
                    └── execute_single_command()
                            │
                            ├── is_builtin("/bin/ls") → false
                            └── find_dir("/bin/ls")
                                    ft_strchr("/bin/ls", '/') → found
                                    return ft_strdup("/bin/ls")
                                    │
                                    └── execute_single_process("/bin/ls", data)
                                            │
                                            ├── fork()
                                            │
                                            ├── child (pid=0):
                                            │     reset_signals()
                                            │     free(path)
                                            │     apply_redirections(NULL) → loop skips
                                            │     execute_child_command()
                                            │         is_builtin? → false
                                            │         find_dir("/bin/ls") → "/bin/ls"
                                            │         stat() → not a directory
                                            │         execve("/bin/ls", ["/bin/ls",NULL], envp)
                                            │         → child becomes ls
                                            │         → ls prints output
                                            │         → ls exits 0
                                            │         → child is gone
                                            │
                                            └── parent:
                                                  free(path)
                                                  update_sigint(handle_sigint_child)
                                                  wait_child(pid) → blocks until ls exits
                                                      WIFEXITED → true
                                                      WEXITSTATUS → 0
                                                  update_sigint(handle_sigint_parent)
                                                  return 0
                            ↑
                    exit_code = 0
            ↑
            command_clear()
            check_signal() → false
            return 0
    ↑
data->last_exit = 0
```

---

## Single Command with Redirection: echo hello > out.txt

```
cmd->argv = ["echo", "hello", NULL]
cmd->redirections = { REDIR_OUT → "out.txt" }

is_builtin("echo")    → true
must_run_in_parent?   → false
cmd->redirections?    → not NULL → execute_builtin_forked()

execute_builtin_forked():
    fork()
    ├── child (pid=0):
    │     reset_signals()
    │     apply_redirections({ REDIR_OUT → "out.txt" }):
    │         open("out.txt", O_CREAT|O_WRONLY|O_TRUNC) → fd=3
    │         dup2(3, STDOUT_FILENO)  → stdout now points to out.txt
    │         close(3)
    │     execute_builtin("echo") → prints "hello\n" → goes to out.txt
    │     exit(0)
    │
    └── parent:
          update_sigint(handle_sigint_child)
          waitpid(pid, &status)
          update_sigint(handle_sigint_parent)
          return WEXITSTATUS(status) → 0
```

---

## Pipeline: echo hello | wc -c

```
cmd1: argv=["echo","hello",NULL]   redir=NULL
cmd2: argv=["wc","-c",NULL]        redir=NULL

execute_pipeline():
    count = 2
    create_pipes(1) → pipes[0] = { fd[0]=read, fd[1]=write }

    fork child 0 (echo):
        setup_pipes(pipes, index=0, total=2):
            index > 0? no  → don't touch stdin
            index < total-1? yes → dup2(pipes[0][1], STDOUT_FILENO)
                                    stdout now writes to pipe
        close_pipes()
        apply_redirections(NULL)
        execute_child_command("echo"):
            execve → echo prints "hello\n" into pipe, exits

    fork child 1 (wc):
        setup_pipes(pipes, index=1, total=2):
            index > 0? yes → dup2(pipes[0][0], STDIN_FILENO)
                              stdin now reads from pipe
            index < total-1? no → don't touch stdout
        close_pipes()
        apply_redirections(NULL)
        execute_child_command("wc"):
            execve → wc reads "hello\n" from stdin, prints "6", exits

    parent:
        close_pipes()         → must close or wc never sees EOF
        waitpid(last_child)   → wait for wc
        wait(NULL) loop       → wait for all others
        return wc's exit code
```
