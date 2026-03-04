# Tokenizer

## What it does

Takes the raw input string and splits it into a linked list of tokens.
Each token has a value (the text) and a type (word, pipe, redirection).

```
"echo hello > file.txt"
        |
        v
[echo] --> [hello] --> [>] --> [file.txt] --> NULL
 WORD       WORD    REDIR_OUT     WORD 
```

---

## Where

```
src/lexer/
    tokenizer.c    -> lexer entry point and token extraction
    token-utils.c  -> linked list operations (new, add_back, last, clear)
```

---

## Data Structure

```
typedef struct s_token
{
    char           *value;   // the text:  "echo", "|", "file.txt"
    t_token_type    type;    // the type:  TOKEN_WORD, TOKEN_PIPE, etc.
    struct s_token *next;    // next token in the list
}   t_token;
```

Token types:

```
TOKEN_WORD          -> command, argument, filename
TOKEN_PIPE          -> |
TOKEN_REDIR_IN      -> <
TOKEN_REDIR_OUT     -> >
TOKEN_REDIR_APPEND  -> >>
TOKEN_HEREDOC       -> <<
```

---

## How it works

### The main loop: lexer()

```
input: "echo "$HOME" | cat > file.txt"
        ^
        ptr walks forward

for each position:
    1. skip whitespace
    2. get_token_operator()  -> extract token string
    3. get_type()            -> classify it
    4. advance ptr by token length
    5. add to list
    6. repeat
```

### Extracting a token: get_token_operator()

```
get_token_operator(ptr):

    ptr == '|'  ───────────────────────────────────> TOKEN_PIPE     "|"

    ptr == '<'  && next char is '<'    ────────────> TOKEN_HEREDOC  "<<"
    ptr == '<'  && !(next char is '<') ────────────> TOKEN_REDIR_IN "<"

    ptr == '>'  && next char is '>'    ────────────> TOKEN_REDIR_APPEND ">>"
    ptr == '>'  && !(next char is '>') ────────────> TOKEN_REDIR_OUT    ">"

    other ──────────────────────────────────────> get_word()
```

>Operator strings like "|", "<<", ">" are string constants defined in minishell_macros.h. They are not malloc'd so they are never freed. Only word tokens (from get_word) are heap allocated.

### Reading a word: get_word()

Advances `current` until it hits whitespace, `|`, `<`, or `>`.
Quoted sections are treated as one unit even if they contain spaces or operators:

```
input: echo "hello | world"
            ^             ^
            start         current stops here (after closing quote)
            
word = ft_substr(start, 0, current - start)
     = "hello | world"   (quotes included, removed later)
```

When a quote is found inside a word, `skip_quoted_section` jumps past it:

```
skip_quoted_section:
    input++                     -> skip opening quote
    while *input != quote_char  -> skip everything inside
    if *input == quote_char
        input++                 -> skip closing quote
    return input
```

If closing quote is missing, the while loop reaches end of string.
This is caught earlier by has_unclosed_quotes() in the readline loop.

### Classifying the token: get_type()

Compares token string against operator constants with ft_strncmp.

```
"|"  -> TOKEN_PIPE
 "<"  -> TOKEN_REDIR_IN
"<<"  -> TOKEN_HEREDOC
">"  -> TOKEN_REDIR_OUT
">>" -> TOKEN_REDIR_APPEND
other -> TOKEN_WORD
```
---

## Quote Handling

The lexer does NOT remove or interpret quotes.
It only uses them to know where a word ends.

```
echo "hello | world"
              ^
              this | stays inside the word token -- not a pipe

result: [echo:WORD] -> ["hello | world":WORD] -> NULL
```

---

## Memory

```
operator tokens  -> string literal macros, not heap allocated, not freed
word tokens      -> ft_substr() allocates in get_word()
                    new_token() duplicates with ft_strdup()
                    get_word() result is freed after new_token() copies it
token_clear()    -> frees value + node for every token in the list
```

---

## Example Trace

Input: `echo "$HOME" | cat > file.txt`
```
    "echo"     -> get_word()           -> get_type() -> TOKEN_WORD      -> [echo:WORD]
    '"$HOME"'  -> get_word()           -> get_type() -> TOKEN_WORD      -> ["$HOME":WORD]
               (skip_quoted_section skips past closing ")
    "|"        -> get_token_operator() -> get_type() -> TOKEN_PIPE      -> [|:PIPE]
    "cat"      -> get_word()           -> get_type() -> TOKEN_WORD      -> [cat:WORD]
    ">"        -> get_token_operator() -> get_type() -> TOKEN_REDIR_OUT -> [>:REDIR_OUT]
    "file.txt" -> get_word()           -> get_type() -> TOKEN_WORD      -> [file.txt:WORD]

result:
[echo] -> ["$HOME"] -> [|] -> [cat] -> [>] -> [file.txt] -> NULL
 WORD      WORD       PIPE   WORD   REDIR_OUT    WORD
```
