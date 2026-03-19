# Manual Tests

---

## Variable Expansion

| Input                              | Expected output  | Notes                          |
|------------------------------------|------------------|--------------------------------|
| `export MYVAR=hello`               | —                | setup                          |
| `echo $MYVAR`                      | `hello`          | basic expansion                |
| `echo "$MYVAR"`                    | `hello`          | double quotes: expands         |
| `echo '$MYVAR'`                    | `$MYVAR`         | single quotes: no expansion    |
| `echo "say $MYVAR world"`          | `say hello world`| expansion inside string        |
| `echo $MYVAR$MYVAR`                | `hellohello`     | two variables adjacent         |
| `echo "$?"`                        | `0`              | exit code expansion            |
| `ls notexist` → `echo "$?"`        | `2`              | exit code after error          |
| `echo $UNDEFINED`                  | *(empty)*        | unset variable                 |
| `echo $`                           | `$`              | lone dollar — literal          |
| `echo "$"`                         | `$`              | lone dollar in double quotes   |
| `echo "hello $"`                   | `hello $`        | lone dollar at end of string   |

---

## Heredoc

| Input                                             | Expected output               | Notes                        |
|---------------------------------------------------|-------------------------------|------------------------------|
| `cat << EOF` / `$MYVAR` / `EOF`                   | `hello`                       | unquoted: expands            |
| `cat << 'EOF'` / `$MYVAR` / `EOF`                 | `$MYVAR`                      | single quoted: no expansion  |
| `cat << "EOF"` / `$MYVAR` / `EOF`                 | `$MYVAR`                      | double quoted: no expansion  |
| `cat << EOF` / `line one` / `$MYVAR` / `EOF`      | `line one` / `hello`          | multiline heredoc            |
| `cat << EOF \| wc -l` / `a` / `b` / `c` / `EOF`   | `3`                           | heredoc piped                |
| `cat << EOF` / `$` / `EOF`                        | `$`                           | lone dollar inside heredoc   |

---

## Redirections

| Input                                              | Expected output    | Notes                      |
|----------------------------------------------------|--------------------|----------------------------|
| `echo hello > out.txt` → `cat out.txt`             | `hello`            | output redirection         |
| `echo hello >> out.txt` → `cat out.txt`            | `hello` twice      | append redirection         |
| `cat < out.txt`                                    | `hello`            | input redirection          |
| `echo hello > out.txt > out2.txt` → `cat out2.txt` | `hello`            | multiple redirections      |
| `> out.txt echo hello` → `cat out.txt`             | `hello`            | redirection before command |
| `echo hello >`                                     | error message      | missing filename           |

---

## Pipelines

| Input                                              | Expected output | Notes            |
|----------------------------------------------------|-----------------|------------------|
| `echo hello \| cat`                                | `hello`         | basic pipe       |
| `echo hello \| wc -c`                              | `6`             | pipe to wc       |
| `echo hello \| cat \| cat \| cat`                  | `hello`         | multiple pipes   |
| `cat << EOF \| wc -l` / `a` / `b` / `c` / `EOF`    | `3`             | heredoc into pipe|

---

## Builtins

| Input                                   | Expected output             | Notes                    |
|-----------------------------------------|-----------------------------|--------------------------|
| `echo`                                  | *(empty line)*              | echo with no args        |
| `echo -n hello`                         | `hello` (no newline)        | -n flag                  |
| `echo -nnn hello`                       | `hello` (no newline)        | multiple n flags         |
| `pwd`                                   | current directory           | —                        |
| `cd /tmp` → `pwd`                       | `/tmp`                      | cd basic                 |
| `cd`                                    | `$HOME`                     | cd with no args          |
| `cd -`                                  | previous dir                | cd to OLDPWD             |
| `export TEST=42` → `env \| grep TEST`   | `TEST=42`                   | export                   |
| `unset TEST` → `env \| grep TEST`       | *(empty)*                   | unset                    |
| `exit 0`                                | exit code `0`               | —                        |
| `exit 42`                               | exit code `42`              | —                        |
| `exit abc`                              | `numeric argument required` | invalid exit code        |
| `exit 1 2`                              | `too many arguments`        | —                        |

---

## Signals

| Input                   | Expected output              | Notes            |
|-------------------------|------------------------------|------------------|
| Ctrl+C at prompt        | new prompt                   | SIGINT handled   |
| Ctrl+C during command   | interrupted, `$?=130`        | —                |
| Ctrl+C during heredoc   | heredoc cancelled            | —                |
| Ctrl+\ at prompt        | nothing                      | SIGQUIT ignored  |
