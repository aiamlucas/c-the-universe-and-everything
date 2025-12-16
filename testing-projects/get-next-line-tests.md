# get_next_line

You only need ONE file: test.txt  
Run ONE command before each test to rewrite the file.

--------------------------------------------------

TEST 1: Simple lines

Command:
printf "Space\nis\nplace\n" > test.txt

Expected:
- "Space\n"
- "is\n"
- "place\n"
- NULL

--------------------------------------------------

TEST 2: Empty file

Command:
> test.txt

Expected:
- NULL on first call

--------------------------------------------------

TEST 3: Only newline

Command:
printf "\n" > test.txt

Expected:
- "\n"
- NULL

--------------------------------------------------

TEST 4: No newline at EOF

Command:
printf "This file has no newline at the end" > test.txt

Expected:
- "This file has no newline at the end"
- NULL

--------------------------------------------------

TEST 5: Multiple consecutive newlines

Command:
printf "\n\nHello\n\n\n" > test.txt

Expected:
- "\n"
- "\n"
- "Hello\n"
- "\n"
- "\n"

--------------------------------------------------

TEST 6: Long line (BUFFER_SIZE stress)

Command:
printf "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n" > test.txt

Expected:
- One long line
- NULL
- Works with BUFFER_SIZE = 1

--------------------------------------------------

NOTES:
- Use printf instead of echo to control newlines
- Always overwrite the file with >
- Recompile when changing BUFFER_SIZE
- Check for memory leaks with valgrind
