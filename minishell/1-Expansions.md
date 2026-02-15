# Shell Variable Expansion and Quote Removal

## What is Expansion?

Expansion is the process where the shell replaces special tokens in the 
command line with their actual values BEFORE executing the command.

Example:
  User types:    echo $USER
  Shell expands: echo anna
  Command runs:  echo anna
  Output:        anna

This happens BETWEEN tokenization and execution.


## Types of Expansion

### 1. Environment Variable Expansion ($VAR)

Replace $VARIABLE_NAME with its value from the environment.

Examples:
  $USER       → your username
  $HOME       → /home/username
  $PATH       → /usr/bin:/bin:/usr/local/bin
  $PWD        → current directory
  $OLDPWD     → previous directory
  $SHELL      → /bin/bash


### 2. Exit Status Expansion ($?)

Replace $? with the exit code of the last command.

Examples:
```
$ ls
$ echo $?
0              ← success

$ nonexistent
$ echo $?
127            ← command not found

$ sleep 10
^C
$ echo $?
130            ← interrupted by Ctrl+C
```


### 3. Quoted vs Unquoted

Behavior changes based on quotes:

```
echo $USER           → Expands
echo "$USER"         → Expands (double quotes)
echo '$USER'         → Does NOT expand (single quotes)
```


## The Processing Pipeline

The shell transforms user input through several distinct phases:

### 1. Lexer (Tokenization)
Splits input string into tokens

Example:
```
Input:  echo "$USER"
Tokens: [echo] ["$USER"]
```

Note: Quote characters are part of the token values at this stage.


### 2. Expansion
Replaces variables with their values while tracking quote state

Example:
```
Input:  ["$USER"]
Output: ["anna"]
```

Key behaviors:
- Variables in double quotes: **expand**
- Variables in single quotes: **do NOT expand**
- Quote characters: **kept in output**

The expansion phase must keep quotes because they control whether 
expansion happens. Only after we've used quotes to make expansion 
decisions can we safely remove them.


### 3. Quote Removal
Strips quote characters from tokens

Example:
```
Input:  ["anna"]
Output: [anna]
```

This is a simple filter: remove all ' and " characters.


### 4. Parser
Builds command structures from clean tokens

Example:
```
Input:  [echo] [anna]
Output: cmd->argv = ["echo", "anna", NULL]
```

The parser receives tokens that are already expanded and unquoted,
so it only needs to organize them into command structures.


### 5. Executor
Runs the commands with the final argv arrays


## Why This Order?

**Why expansion before quote removal?**
- Quotes determine whether expansion happens
- "$USER" must expand, but '$USER' must not
- We need quotes present during expansion to make this decision
- After expansion completes, quotes have served their purpose

**Why quote removal before parsing?**
- Parser receives clean, final strings
- No special quote handling needed in parser logic
- Command structures contain values ready for execution
- Simpler and cleaner separation of concerns


## Complete Test Cases

To understand how the shell behaves and ensure correctness, we can build comprehensive tests covering all cases. 

**Part 1: Expansion Tests**
- Tests expansion logic in isolation
- Input: tokens with variables and quotes
- Output: tokens with expanded variables (quotes still present)
- Verifies: variable replacement, quote state tracking, edge cases

**Part 2: Quote Removal Tests**
- Tests quote removal logic in isolation
- Input: tokens with quote characters
- Output: tokens without quote characters
- Verifies: proper filtering of ' and " characters

**Part 3: Integration Tests**
- Tests the complete pipeline end-to-end
- Input: user command strings
- Output: final argv arrays ready for execution
- Verifies: correct integration using print_commands output


## Reading the Tests

Each test follows a consistent format:

For expansion and quote removal tests:
```
#### Test X.Y: Description
Input tokens: [token1] [token2]
After expansion/removal: [result1] [result2]
Rule: Explanation of the behavior
```

For integration tests:
```
#### Test X.Y: Description
Input: command string
Token list: [token1] [token2]
After expansion: [expanded1] [expanded2]
After quotes: [final1] [final2]
Expected: cmd1->argv = ["arg0", "arg1", NULL]
```

The [token] notation represents the token list as shown by debug 
print functions. Empty tokens are shown as [].


## Key Implementation Concepts

### Quote State Tracking

During expansion, track which quote context you're in:
- STATE_NONE: Outside quotes → expand variables
- STATE_SINGLE: Inside ' ' → do NOT expand (everything literal)
- STATE_DOUBLE: Inside " " → expand variables

### Variable Name Rules

Valid variable name characters:
- Letters: a-z, A-Z
- Digits: 0-9 (but not as first character)
- Underscore: _

Variable name stops at any other character:
```
$USER@host  → variable is "USER", @ is literal
$USER:$HOME → variable is "USER", : is literal
$USER.txt   → variable is "USER", . is literal
```

### Special Variables

- $? = last exit code (0-255)
- Other special variables like $$, $0, $1 are not required for basic minishell

### Undefined Variables

Undefined variables expand to empty string (not an error):
```
$UNDEFINED           → (empty)
hello$UNDEFINEDworld → helloworld
```

## Test Environment Setup

Tests assume these environment variables exist:
```
USER=anna
HOME=/home/anna
SHELL=/bin/bash
FILE=input.txt
OUTPUT=output.txt
PATTERN=pattern
LOG=log.txt
```

And these variables do NOT exist (undefined):
```
UNDEFINED
A, B, C
```

Default last exit code for tests: $? = 0 (unless specified otherwise)


---

## PART 1: EXPANSION TESTS (ALL CASES + EDGE CASES)

Goal: Test expand_token_value function thoroughly
Input: Token list representation
Output: Token list after expansion (quotes KEPT)

---

### Basic Variable Expansion (8 tests)

#### Test 1.1: Simple variable
Input tokens: [$USER]
After expansion: [anna]
Rule: No quotes, expand normally

#### Test 1.2: Undefined variable
Input tokens: [$UNDEFINED]
After expansion: []
Rule: Undefined becomes empty string

#### Test 1.3: Variable with prefix
Input tokens: [prefix$USER]
After expansion: [prefixanna]
Rule: Text before variable preserved

#### Test 1.4: Multiple variables
Input tokens: [$USER$HOME]
After expansion: [anna/home/anna]
Rule: Both expanded, no separator

#### Test 1.5: Variables with separator
Input tokens: [$USER:$HOME]
After expansion: [anna:/home/anna]
Rule: Colon separator preserved

#### Test 1.6: Variable with special char
Input tokens: [$USER@host]
After expansion: [anna@host]
Rule: Variable stops at @, @ is literal

#### Test 1.7: Variable with dot
Input tokens: [$USER.txt]
After expansion: [anna.txt]
Rule: Variable stops at dot

#### Test 1.8: Variable with dash
Input tokens: [$USER-test]
After expansion: [anna-test]
Rule: Variable stops at dash

---

### Exit Status (3 tests)

#### Test 2.1: Basic exit status
Input tokens: [$?]
After expansion: [0]
Rule: Replace with last exit code

#### Test 2.2: Exit status with text
Input tokens: [code:$?]
After expansion: [code:0]
Rule: Exit code inserted in text

#### Test 2.3: Multiple exit status
Input tokens: [$?:$?]
After expansion: [0:0]
Rule: Both replaced with same value

---

### Edge Cases (8 tests)

> Bash supports “special parameters” (not environment variables):

- $? : exit status of last foreground command
- $$ : PID of the current shell
- $! : PID of the last background job
- $0 : name of the shell / script
- $1, $2, … : positional arguments (script/function args)
- $# : number of positional arguments
- $@ : all positional arguments (preserves words; quote-sensitive)
- $* : all positional arguments as one string (quote/IFS-sensitive)
- $- : current shell option flags
- $_ : last argument of the previous command

Minishell mandatory requirement:
- Expand only:
  - $NAME (environment variables), NAME = [A-Za-z_][A-Za-z0-9_]*
  - $? (exit status)

Minishell rule for everything else:
- If '$' is NOT followed by '?' or NOT by a valid env-var name start:
	- first character: a letter (a-z or A-Z) or _
	- following characters: letters, digits (0-9), or _
  then '$' is treated as a LITERAL character and the next character is NOT consumed.

--- Tests (minishell expected results + bash behavior) ---

#### Test SP.1: Dollar alone
Input tokens: [$]
After expansion (minishell): [$]
Bash difference: Same as minishell (no name after '$', so '$' stays literal)

#### Test SP.2: Dollar followed by whitespace
Input tokens: [$ test]
After expansion (minishell): [$ test]
Bash difference: Same as minishell ('$' is not followed by a valid name start, so '$' stays literal)

#### Test SP.3: Dollar followed by punctuation (invalid start)
Input tokens: [$+abc]
After expansion (minishell): [$+abc]
Bash difference: Same as minishell ('+' is not a valid variable-name start, so '$' is literal)

#### Test SP.4: Dollar followed by digit (positional params)
Input tokens: [$1test]
After expansion (minishell): [$1test]
Bash difference: Different (bash expands $1 to the first positional argument in scripts/functions; if unset it becomes empty, so it may look like "test")

#### Test SP.5: Dollar followed by @ (positional params list)
Input tokens: [$@test]
After expansion (minishell): [$@test]
Bash difference: Different (bash expands $@ to positional arguments; if there are none it becomes empty, so it may look like "test" in some contexts)

#### Test SP.6: Double dollar (PID)
Input tokens: [$$]
After expansion (minishell): [$$]
Bash difference: Different (bash expands $$ to the PID of the current shell)

#### Test SP.7: Double dollar + env var after
Input tokens: [$$USER]
After expansion (minishell): [$anna]   (assuming USER=anna)
Bash difference: Different (bash expands $$ to PID, then appends literal "USER" => e.g. "12345USER")

#### Test SP.8: Dollar before quote character
Input tokens: [$"test]
After expansion (minishell): [$"test]
Bash difference: Same as minishell ('"' is not a valid variable-name start, so '$' is literal)
Note: quote handling/removal is a separate step.

---

### Variable Name Boundaries (6 tests)

#### Test 4.1: Variable with numbers
Input tokens: [$USER123]
After expansion: [] or [value]
Rule: USER123 is full variable name (depends if exists)

#### Test 4.2: Variable with underscore
Input tokens: [$USER_NAME]
After expansion: [anna_smith]
Rule: Underscore valid in variable name

#### Test 4.3: Variable stops at slash
Input tokens: [$USER/path]
After expansion: [anna/path]
Rule: Slash not valid in variable name

#### Test 4.4: Variable stops at equals
Input tokens: [$USER=value]
After expansion: [anna=value]
Rule: Equals not valid in variable name

#### Test 4.5: Variable stops at bracket
Input tokens: [$USER(test)]
After expansion: [anna(test)]
Rule: Bracket not valid in variable name

---

### Double Quotes - Expand (10 tests)

#### Test 5.1: Variable in double quotes
Input tokens: ["$USER"]
After expansion: ["anna"]
Rule: Expand variable, KEEP quotes

#### Test 5.2: Multiple vars in double quotes
Input tokens: ["$USER $HOME"]
After expansion: ["anna /home/anna"]
Rule: Both expanded, quotes kept, spaces preserved

#### Test 5.3: Text with var in double quotes
Input tokens: ["hello $USER world"]
After expansion: ["hello anna world"]
Rule: Expand variable, keep text and quotes

#### Test 5.4: Empty var in double quotes
Input tokens: ["hello$UNDEFINED world"]
After expansion: ["hello world"]
Rule: Empty variable disappears, space remains

#### Test 5.5: Exit status in double quotes
Input tokens: ["exit:$?"]
After expansion: ["exit:0"]
Rule: Expand $?, keep quotes

#### Test 5.6: Empty double quotes
Input tokens: [""]
After expansion: [""]
Rule: No expansion needed, quotes kept

#### Test 5.7: Only text in double quotes
Input tokens: ["hello world"]
After expansion: ["hello world"]
Rule: No variables, nothing changes

#### Test 5.8: Single quote inside double
Input tokens: ["He said 'hi'"]
After expansion: ["He said 'hi'"]
Rule: Inner single quotes are literal

#### Test 5.9: Multiple vars adjacent in double
Input tokens: ["$USER$HOME"]
After expansion: ["anna/home/anna"]
Rule: Both expanded, concatenated

#### Test 5.10: Variable at boundaries in double
Input tokens: ["$USER and $HOME"]
After expansion: ["anna and /home/anna"]
Rule: Both expanded, text preserved

---

### Single Quotes - No Expansion (8 tests)

#### Test 6.1: Variable in single quotes
Input tokens: ['$USER']
After expansion: ['$USER']
Rule: NO expansion in single quotes

#### Test 6.2: Multiple vars in single quotes
Input tokens: ['$USER $HOME']
After expansion: ['$USER $HOME']
Rule: NO expansion, everything literal

#### Test 6.3: Exit status in single quotes
Input tokens: ['$?']
After expansion: ['$?']
Rule: NO expansion, stays as $?

#### Test 6.4: Empty single quotes
Input tokens: ['']
After expansion: ['']
Rule: No expansion needed, quotes kept

#### Test 6.5: Only text in single quotes
Input tokens: ['hello world']
After expansion: ['hello world']
Rule: No variables, nothing changes

#### Test 6.6: Double quote inside single
Input tokens: ['He said "hi"']
After expansion: ['He said "hi"']
Rule: Inner double quotes are literal

#### Test 6.7: Dollar signs in single quotes
Input tokens: ['$$USER']
After expansion: ['$$USER']
Rule: Everything literal, no expansion

#### Test 6.8: Special chars in single quotes
Input tokens: ['$@#%']
After expansion: ['$@#%']
Rule: All special chars literal

---

### Mixed Quotes (8 tests)

#### Test 7.1: Double then single
Input tokens: ["$USER"'$HOME']
After expansion: ["anna"'$HOME']
Rule: First part expanded, second not

#### Test 7.2: Single then double
Input tokens: ['$USER'"$HOME"]
After expansion: ['$USER'"/home/anna"]
Rule: First part not expanded, second expanded

#### Test 7.3: Three segments
Input tokens: ["$USER"'literal'"$HOME"]
After expansion: ["anna"'literal'"/home/anna"]
Rule: Double quote parts expanded, single not

#### Test 7.4: Unquoted then double
Input tokens: [$USER"$HOME"]
After expansion: [anna"/home/anna"]
Rule: Both expanded (unquoted and double quoted)

#### Test 7.5: Double then unquoted
Input tokens: ["$USER"$HOME]
After expansion: ["anna"/home/anna]
Rule: Both expanded

#### Test 7.6: Many alternations
Input tokens: ["a"'b'"c"'d']
After expansion: ["a"'b'"c"'d']
Rule: No variables, all quotes kept

#### Test 7.7: Empty quotes mixed
Input tokens: [""'']
After expansion: [""'']
Rule: Empty quotes kept

#### Test 7.8: Variable in each segment
Input tokens: ["$USER"'$HOME'$SHELL]
After expansion: ["anna"'$HOME'/bin/bash]
Rule: Expand in double and unquoted, not in single

---

## PART 2: QUOTE REMOVAL TESTS (ALL CASES + EDGE CASES)

Goal: Test remove_quotes function thoroughly
Input: Token list with quotes
Output: Token list without quote characters

---

### Double Quote Removal (6 tests)

#### Test 1.1: Simple double quotes
Input tokens: ["hello"]
After removal: [hello]
Rule: Remove both " characters

#### Test 1.2: Double quotes with spaces
Input tokens: ["hello world"]
After removal: [hello world]
Rule: Remove quotes, keep spaces

#### Test 1.3: Empty double quotes
Input tokens: [""]
After removal: []
Rule: Remove quotes, result is empty

#### Test 1.4: Multiple double quote pairs
Input tokens: ["a""b"]
After removal: [ab]
Rule: Remove all " characters

#### Test 1.5: Double quotes at positions
Input tokens: ["start"middle"end"]
After removal: [startmiddleend]
Rule: Remove all quotes, concatenate

#### Test 1.6: Double quotes with special chars
Input tokens: ["@#$%"]
After removal: [@#$%]
Rule: Remove quotes, keep special chars

---

### Single Quote Removal (6 tests)

#### Test 2.1: Simple single quotes
Input tokens: ['hello']
After removal: [hello]
Rule: Remove both ' characters

#### Test 2.2: Single quotes with spaces
Input tokens: ['hello world']
After removal: [hello world]
Rule: Remove quotes, keep spaces

#### Test 2.3: Empty single quotes
Input tokens: ['']
After removal: []
Rule: Remove quotes, result is empty

#### Test 2.4: Multiple single quote pairs
Input tokens: ['a''b']
After removal: [ab]
Rule: Remove all ' characters

#### Test 2.5: Single quotes at positions
Input tokens: ['start'middle'end']
After removal: [startmiddleend]
Rule: Remove all quotes, concatenate

#### Test 2.6: Single quotes with special chars
Input tokens: ['@#$%']
After removal: [@#$%]
Rule: Remove quotes, keep special chars

---

### Mixed Quote Removal (6 tests)

#### Test 3.1: Double then single
Input tokens: ["hello"'world']
After removal: [helloworld]
Rule: Remove all quote characters

#### Test 3.2: Single then double
Input tokens: ['hello'"world"]
After removal: [helloworld]
Rule: Remove all quote characters

#### Test 3.3: Multiple mixed pairs
Input tokens: ["a"'b'"c"'d']
After removal: [abcd]
Rule: Remove all quotes, concatenate

#### Test 3.4: Empty mixed
Input tokens: [""'']
After removal: []
Rule: All quotes removed, empty result

#### Test 3.5: Mixed with text between
Input tokens: ["a"text'b']
After removal: [atextb]
Rule: Remove quotes, keep text

#### Test 3.6: Alternating many times
Input tokens: ["1"'2'"3"'4'"5"]
After removal: [12345]
Rule: Remove all quotes

---

### Nested-Looking Quotes (4 tests)

#### Test 4.1: Single inside double
Input tokens: ["He said 'hello'"]
After removal: [He said 'hello']
Rule: Remove only outer ", inner ' stays

#### Test 4.2: Double inside single
Input tokens: ['She said "hi"']
After removal: [She said "hi"]
Rule: Remove only outer ', inner " stays

#### Test 4.3: Multiple nested
Input tokens: ["outer 'inner' outer"]
After removal: [outer 'inner' outer]
Rule: Remove only outer quotes

---

### No Quotes (4 tests)

#### Test 5.1: Plain text
Input tokens: [hello]
After removal: [hello]
Rule: No quotes, no change

#### Test 5.2: Text with spaces
Input tokens: [hello world]
After removal: [hello world]
Rule: No quotes, no change

#### Test 5.3: Special characters
Input tokens: [@#$%^&*()]
After removal: [@#$%^&*()]
Rule: No quotes, no change

#### Test 5.4: Numbers and symbols
Input tokens: [123-456_789]
After removal: [123-456_789]
Rule: No quotes, no change

---

### Edge Cases (6 tests)

#### Test 6.1: Only quotes
Input tokens: [""""]
After removal: []
Rule: All quotes removed, empty

#### Test 6.2: Only mixed quotes
Input tokens: [""''""'']
After removal: []
Rule: All quotes removed, empty

#### Test 6.3: Single quote char
Input tokens: ["]
After removal: []
Rule: Single quote removed

#### Test 6.4: Text then quote
Input tokens: [test"]
After removal: [test]
Rule: Trailing quote removed

#### Test 6.5: Quote then text
Input tokens: ["test]
After removal: [test]
Rule: Leading quote removed

#### Test 6.6: Quotes far apart
Input tokens: ["start middle end"]
After removal: [start middle end]
Rule: Both quotes removed

---

## PART 3: INTEGRATION TESTS (EXPANSION + QUOTES + PARSER)

Goal: Test full pipeline from input to argv array
Method: Use print_commands to verify command structure
Format: cmd->argv = [arg0, arg1, ..., NULL]


### Basic Commands (5 tests)

#### Test 1.1: Plain command
Input: echo hello
Token list: [echo] [hello]
After expansion: [echo] [hello]
After quotes: [echo] [hello]
Expected: cmd1->argv = ["echo", "hello", NULL]

#### Test 1.2: Command with multiple args
Input: echo hello world test
Token list: [echo] [hello] [world] [test]
After expansion: [echo] [hello] [world] [test]
After quotes: [echo] [hello] [world] [test]
Expected: cmd1->argv = ["echo", "hello", "world", "test", NULL]

#### Test 1.3: Command no args
Input: ls
Token list: [ls]
After expansion: [ls]
After quotes: [ls]
Expected: cmd1->argv = ["ls", NULL]

#### Test 1.4: Command with flags
Input: ls -la /tmp
Token list: [ls] [-la] [/tmp]
After expansion: [ls] [-la] [/tmp]
After quotes: [ls] [-la] [/tmp]
Expected: cmd1->argv = ["ls", "-la", "/tmp", NULL]

#### Test 1.5: Long command
Input: grep pattern file1 file2 file3
Token list: [grep] [pattern] [file1] [file2] [file3]
After expansion: [grep] [pattern] [file1] [file2] [file3]
After quotes: [grep] [pattern] [file1] [file2] [file3]
Expected: cmd1->argv = ["grep", "pattern", "file1", "file2", "file3", NULL]


### Variable Expansion Integration (7 tests)

#### Test 2.1: Simple variable
Input: echo $USER
Token list: [echo] [$USER]
After expansion: [echo] [anna]
After quotes: [echo] [anna]
Expected: cmd1->argv = ["echo", "anna", NULL]

#### Test 2.2: Multiple variables
Input: echo $USER $HOME
Token list: [echo] [$USER] [$HOME]
After expansion: [echo] [anna] [/home/anna]
After quotes: [echo] [anna] [/home/anna]
Expected: cmd1->argv = ["echo", "anna", "/home/anna", NULL]

#### Test 2.3: Undefined variable
Input: echo $UNDEFINED test
Token list: [echo] [$UNDEFINED] [test]
After expansion: [echo] [] [test]
After quotes: [echo] [] [test]
Expected: cmd1->argv = ["echo", "", "test", NULL]

#### Test 2.4: Exit status
Input: echo $?
Token list: [echo] [$?]
After expansion: [echo] [0]
After quotes: [echo] [0]
Expected: cmd1->argv = ["echo", "0", NULL]

#### Test 2.5: Adjacent variables
Input: echo $USER$HOME
Token list: [echo] [$USER$HOME]
After expansion: [echo] [anna/home/anna]
After quotes: [echo] [anna/home/anna]
Expected: cmd1->argv = ["echo", "anna/home/anna", NULL]

#### Test 2.6: Variable in middle
Input: echo before $USER after
Token list: [echo] [before] [$USER] [after]
After expansion: [echo] [before] [anna] [after]
After quotes: [echo] [before] [anna] [after]
Expected: cmd1->argv = ["echo", "before", "anna", "after", NULL]

#### Test 2.7: All undefined
Input: echo $A $B $C
Token list: [echo] [$A] [$B] [$C]
After expansion: [echo] [] [] []
After quotes: [echo] [] [] []
Expected: cmd1->argv = ["echo", "", "", "", NULL]


### Quoted Expansion Integration (8 tests)

#### Test 3.1: Double quoted variable
Input: echo "$USER"
Token list: [echo] ["$USER"]
After expansion: [echo] ["anna"]
After quotes: [echo] [anna]
Expected: cmd1->argv = ["echo", "anna", NULL]

#### Test 3.2: Single quoted variable
Input: echo '$USER'
Token list: [echo] ['$USER']
After expansion: [echo] ['$USER']
After quotes: [echo] [$USER]
Expected: cmd1->argv = ["echo", "$USER", NULL]

#### Test 3.3: Mixed quotes
Input: echo "$USER" '$HOME'
Token list: [echo] ["$USER"] ['$HOME']
After expansion: [echo] ["anna"] ['$HOME']
After quotes: [echo] [anna] [$HOME]
Expected: cmd1->argv = ["echo", "anna", "$HOME", NULL]

#### Test 3.4: Variable in double with spaces
Input: echo "$USER is here"
Token list: [echo] ["$USER is here"]
After expansion: [echo] ["anna is here"]
After quotes: [echo] [anna is here]
Expected: cmd1->argv = ["echo", "anna is here", NULL]

#### Test 3.5: Multiple vars in double quotes
Input: echo "$USER and $HOME"
Token list: [echo] ["$USER and $HOME"]
After expansion: [echo] ["anna and /home/anna"]
After quotes: [echo] [anna and /home/anna]
Expected: cmd1->argv = ["echo", "anna and /home/anna", NULL]

#### Test 3.6: Empty quotes
Input: echo "" test
Token list: [echo] [""] [test]
After expansion: [echo] [""] [test]
After quotes: [echo] [] [test]
Expected: cmd1->argv = ["echo", "", "test", NULL]

#### Test 3.7: Quote segments
Input: echo "$USER"'literal'test
Token list: [echo] ["$USER"'literal'test]
After expansion: [echo] ["anna"'literal'test]
After quotes: [echo] [annaliteraltest]
Expected: cmd1->argv = ["echo", "annaliteraltest", NULL]

#### Test 3.8: Complex quotes
Input: echo pre"$USER"'$HOME'post
Token list: [echo] [pre"$USER"'$HOME'post]
After expansion: [echo] [pre"anna"'$HOME'post]
After quotes: [echo] [preanna$HOMEpost]
Expected: cmd1->argv = ["echo", "preanna$HOMEpost", NULL]


### Redirections Integration (6 tests)

#### Test 4.1: Input redirection with variable
Input: cat < $FILE
Token list: [cat] [<] [$FILE]
After expansion: [cat] [<] [input.txt]
After quotes: [cat] [<] [input.txt]
Expected:
  cmd1->argv = ["cat", NULL]
  cmd1->redirections = [<INPUT: "input.txt">]

#### Test 4.2: Output redirection with variable
Input: echo $USER > $OUTPUT
Token list: [echo] [$USER] [>] [$OUTPUT]
After expansion: [echo] [anna] [>] [output.txt]
After quotes: [echo] [anna] [>] [output.txt]
Expected:
  cmd1->argv = ["echo", "anna", NULL]
  cmd1->redirections = [<OUTPUT: "output.txt">]

#### Test 4.3: Quoted redirection target
Input: cat < "$FILE"
Token list: [cat] [<] ["$FILE"]
After expansion: [cat] [<] ["input.txt"]
After quotes: [cat] [<] [input.txt]
Expected:
  cmd1->argv = ["cat", NULL]
  cmd1->redirections = [<INPUT: "input.txt">]

#### Test 4.4: Append with variable
Input: echo test >> $LOG
Token list: [echo] [test] [>>] [$LOG]
After expansion: [echo] [test] [>>] [log.txt]
After quotes: [echo] [test] [>>] [log.txt]
Expected:
  cmd1->argv = ["echo", "test", NULL]
  cmd1->redirections = [<APPEND: "log.txt">]

#### Test 4.5: Multiple redirections
Input: cat < $IN > $OUT
Token list: [cat] [<] [$IN] [>] [$OUT]
After expansion: [cat] [<] [in.txt] [>] [out.txt]
After quotes: [cat] [<] [in.txt] [>] [out.txt]
Expected:
  cmd1->argv = ["cat", NULL]
  cmd1->redirections = [<INPUT: "in.txt">, <OUTPUT: "out.txt">]

#### Test 4.6: Redirection with quoted arg
Input: echo "$USER" > $FILE
Token list: [echo] ["$USER"] [>] [$FILE]
After expansion: [echo] ["anna"] [>] [file.txt]
After quotes: [echo] [anna] [>] [file.txt]
Expected:
  cmd1->argv = ["echo", "anna", NULL]
  cmd1->redirections = [<OUTPUT: "file.txt">]


### Pipeline Integration (4 tests)

#### Test 5.1: Simple pipe with variable
Input: echo $USER | cat
Token list: [echo] [$USER] [|] [cat]
After expansion: [echo] [anna] [|] [cat]
After quotes: [echo] [anna] [|] [cat]
Expected:
  cmd1->argv = ["echo", "anna", NULL]
  cmd2->argv = ["cat", NULL]

#### Test 5.2: Pipe with multiple variables
Input: echo $USER | grep $PATTERN
Token list: [echo] [$USER] [|] [grep] [$PATTERN]
After expansion: [echo] [anna] [|] [grep] [pattern]
After quotes: [echo] [anna] [|] [grep] [pattern]
Expected:
  cmd1->argv = ["echo", "anna", NULL]
  cmd2->argv = ["grep", "pattern", NULL]

#### Test 5.3: Pipe with quoted variables
Input: echo "$USER" | grep '$PATTERN'
Token list: [echo] ["$USER"] [|] [grep] ['$PATTERN']
After expansion: [echo] ["anna"] [|] [grep] ['$PATTERN']
After quotes: [echo] [anna] [|] [grep] [$PATTERN]
Expected:
  cmd1->argv = ["echo", "anna", NULL]
  cmd2->argv = ["grep", "$PATTERN", NULL]

#### Test 5.4: Three-command pipeline
Input: cat $FILE | grep $PATTERN | wc
Token list: [cat] [$FILE] [|] [grep] [$PATTERN] [|] [wc]
After expansion: [cat] [file.txt] [|] [grep] [pattern] [|] [wc]
After quotes: [cat] [file.txt] [|] [grep] [pattern] [|] [wc]
Expected:
  cmd1->argv = ["cat", "file.txt", NULL]
  cmd2->argv = ["grep", "pattern", NULL]
  cmd3->argv = ["wc", NULL]


### Edge Cases Integration (4 tests)

#### Test 6.1: Only variable (undefined)
Input: $UNDEFINED
Token list: [$UNDEFINED]
After expansion: []
After quotes: []
Expected:
  cmd1->argv = ["", NULL]

#### Test 6.2: Multiple empty args
Input: echo "" '' $UNDEFINED
Token list: [echo] [""] [''] [$UNDEFINED]
After expansion: [echo] [""] [''] []
After quotes: [echo] [] [] []
Expected:
  cmd1->argv = ["echo", "", "", "", NULL]

#### Test 6.3: Complex mixing
Input: echo $USER:$HOME'$SHELL'end
Token list: [echo] [$USER:$HOME'$SHELL'end]
After expansion: [echo] [anna:/home/anna'$SHELL'end]
After quotes: [echo] [anna:/home/anna$SHELLend]
Expected:
  cmd1->argv = ["echo", "anna:/home/anna$SHELLend", NULL]

#### Test 6.4: Variable in every position
Input: $CMD $ARG1 "$ARG2" > $OUT
Token list: [$CMD] [$ARG1] ["$ARG2"] [>] [$OUT]
After expansion: [command] [arg1] ["arg2value"] [>] [output]
After quotes: [command] [arg1] [arg2value] [>] [output]
Expected:
  cmd1->argv = ["command", "arg1", "arg2value", NULL]
  cmd1->redirections = [<OUTPUT: "output">]
