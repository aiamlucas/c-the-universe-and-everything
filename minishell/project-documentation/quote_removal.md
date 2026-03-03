## Quote Removal — `quote_removal.c`

Called after `expand_tokens`. Quotes were kept as markers during expansion to track state. Now we need to remove them. 

### How the quote tracker works

`quote` variable starts as `\0` (meaning: outside any quotes).

Three cases per char:

    quote == '\0' and char is ' or "  → opening quote found
                                         set quote = char, skip it
    char == quote                      → closing quote found (matches opener)
                                         set quote = '\0', skip it
    anything else                      → regular char, keep it

This means:
    - ' inside "..."  → quote is '"', char is '\'' → not equal → kept as regular char
    - " inside '...'  → quote is '\'', char is '"' → not equal → kept as regular char

### Two-pass design (same pattern as expansion)

1. `calculate_length_without_quotes` Walks the string with the quote tracker, counts only the chars that will be kept (not the quotes themselves)

2. `remove_quote_chars` Allocates exact buffer using that count, walks again with same logic, copies only the kept chars

### Entry point

`remove_quotes` walks the token list, calls `remove_quote_chars` for each
TOKEN_WORD, frees the old value and replaces it with the new one.
Returns false on malloc failure.

### Example

    input:  "/home/anna and 'anna'"
    
    "         → opening double quote  → quote='"',  skip
    /home/anna and  → regular chars   → keep
    '         → quote='"', char='\''  → not equal  → keep (regular char)
    anna      → regular chars         → keep
    '         → quote='"', char='\''  → not equal  → keep (regular char)
    "         → closing double quote  → quote='\0', skip
    
    result: /home/anna and 'anna'
    
    wait — the single quotes are still there because we were inside double quotes.
    a second token "'anna'" (from '$USER' expanded) would strip its own quotes:
    
    '         → opening single quote  → quote='\'', skip
    anna      → regular chars         → keep
    '         → closing single quote  → quote='\0', skip
    
    result: anna
