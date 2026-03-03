# Expansion

## Overview

The expansion system processes shell variable expansion for all TOKEN_WORD tokens
in the token list. It runs in two passes:

1. **Pass 1 — Calculate length** (`expanded_length`): walks the string and counts
   exactly how many bytes the expanded result will need
2. **Pass 2 — Build result** (`expand_variable`): allocates the buffer and fills it

After both passes, `remove_quotes` strips the quote characters that were kept
as markers during expansion.

---

## Data Structures

### `t_expand_state` (enum)
Tracks the current quote context while walking the string:

```
EXPAND_NORMAL  → outside any quotes
EXPAND_SINGLE  → inside '...' (no expansion)
EXPAND_DOUBLE  → inside "..." (expansion allowed)
```

### `t_dollar_act` (enum)
Return value of `count_dollar` / `build_dollar`, tells the caller what to do next:

```
D_STOP    → $ was at end of string, stop the loop
D_SKIP    → $ was handled (literal or $?), continue to next char
D_EXPAND  → $ followed by valid identifier, look up variable
```

### `t_expand` (struct)
Bundles the expansion state for pass 2:

```
typedef struct s_expand
{
    char           *result;    // output buffer
    size_t         *position;  // current write index (pointer for pass-by-ref)
    t_expand_state *state;     // current quote state (pointer for pass-by-ref)
}   t_expand;
```

---

## State Machine

`update_state` tracks quote context char by char:

```
state=NORMAL + '  ->  SINGLE   (opening single quote)
state=NORMAL + "  ->  DOUBLE   (opening double quote)
state=SINGLE + '  ->  NORMAL   (closing single quote)
state=DOUBLE + "  ->  NORMAL   (closing double quote)
```

Key rule: `"` inside `' ... '` does NOT change state.
          `'` inside `" ... "` does NOT change state.

---

## The Three Questions

Every character in the string is processed by asking three questions in order:

```
1. is quote or not $? ──yes──► update state if quote → count/append → next char
    │
    no (it's a $)
    │
    ▼
2. what kind of $?
    ├── end of string   ──► append $ literally → STOP
    ├── inside '...'    ──► append $ literally → SKIP → next char
    ├── $?              ──► append exit code   → SKIP → next char
    ├── non-identifier  ──► append $ literally → SKIP → next char
    └── identifier      ──► look up variable
                                │
                                ▼
                       3. found in env?
                            ├── yes → count/append value → next char
                            └── no  → count 0 / append nothing → next char
```

---

## Pass 1 — `expand_length.c`

Calculates the exact byte count needed for the expanded result.

### Main loop — `expanded_length`

```
while (*ptr)
{
    if (*ptr == '\'' || *ptr == '\"')
        update_state(&state, *ptr);
    if (count_if_literal(&ptr, &len))
        continue ;
    action = count_dollar(&ptr, &len, state, last_exit);
    if (action == D_STOP)
        break ;
    if (action == D_SKIP)
        continue ;
    len += count_var_len(&ptr, internal_env); // question 3
}
```

### Helper functions

| Function                 | What it does                                                          |
|--------------------------|-----------------------------------------------------------------------|
| `count_if_literal`       | if char is not $, increments len and advances ptr                     |
| `count_dollar`           | advances ptr past $, counts bytes needed based on what follows        |
| `count_exit_len`         | counts digits needed for exit code string                             |
| `count_var_len`          | finds variable in env, advances ptr returns value length (0 if unset) |
| `advance_and_count_name` | advances ptr past variable name, returns name length                  |

---

## Pass 2 — `expand_variable_helpers.c` and `expand_copy_value.c`

Allocates the buffer and fills it char by char.

### Main loop — `expand_variable`

```
while (*ptr)
{
    if (*ptr == '\'' || *ptr == '\"')
        update_state(exp.state, *ptr);
    if (build_if_literal(&ptr, &exp))
        continue ;
    action = build_dollar(&ptr, &exp, last_exit);
    if (action == D_STOP)
        break ;
    if (action == D_SKIP)
        continue ;
    copy_var_value(&ptr, &exp, internal_env);
}
```

### Helper functions

| Function                 | What it does                                         |
|--------------------------|------------------------------------------------------|
| `init_expand`            | allocates buffer, sets position=0, state=NORMAL      |
| `build_if_literal`       | appends char if not $, advances ptr, returns true    |
| `append_char`            | writes one char to buffer, advances ptr and position |
| `append_dollar`          | writes $ literally to buffer, advances position      |
| `build_dollar`           | decides what kind of $ and writes accordingly        |
| `build_exit_code`        | converts exit code to string and writes digits       |
| `copy_var_value`         | finds variable in env and appends its value          |
| `append_var_value`       | writes value string char by char into buffer         |
| `advance_and_count_name` | advances ptr past variable name, returns name length |

---

## Parallel Design

Pass 1 and Pass 2 are mirror images of each other:

```
Pass 1 (count)            Pass 2 (build)
─────────────────────     ─────────────────────
count_if_literal          build_if_literal
count_dollar              build_dollar
count_exit_len            build_exit_code
count_var_len             copy_var_value
advance_and_count_name    advance_and_count_name  (shared)
```

The same logic runs twice: once to measure, once to write.

---

## Pass 3 — `quote_removal.c`

After expansion, quotes are still present in the token values as markers.
`remove_quotes` strips them:

```
before: "/home/anna and '$USER'"
after:  /home/anna and $USER
```

Note: by the time `remove_quotes` runs, variables inside double quotes are already expanded. The quotes are only needed during expansion to track state. After that they are removed.

---

## Entry Point — `expand_tokens`

```
bool expand_tokens(t_token *tokens, t_env *internal_env, int last_exit)
```

Walks the token list. For each TOKEN_WORD:
1. calls `expanded_length` → get byte count
2. calls `expand_variable` → allocate and fill result
3. frees old token value, replaces with expanded value
4. returns false on malloc failure (caller cleans up tokens)

---

## Example Trace

Input: `echo "$HOME and '$USER'"`
Environment: HOME=/home/anna, USER=anna

### Pass 1 — count

```
"        → quote → update state (NORMAL→DOUBLE), count 1
$HOME    → dollar → D_EXPAND → count_var_len → /home/anna = 10
 and     → count each char → 4
'        → quote → update state (stays DOUBLE), count 1
$USER    → dollar → D_EXPAND → count_var_len → anna = 4
'        → quote → count 1
"        → quote → update state (DOUBLE→NORMAL), count 1

total len = 1 + 10 + 4 + 1 + 4 + 1 + 1 = 22
```

### Pass 2 — build

```
"        → append " to result
$HOME    → copy_var_value → append /home/anna
 and     → append each char
'        → append '
$USER    → copy_var_value → append anna
'        → append '
"        → append "

result = "/home/anna and 'anna'"
```

### Pass 3 — remove quotes

```
"/home/anna and 'anna'"  →  /home/anna and anna
```

Final token value: `/home/anna and anna`
