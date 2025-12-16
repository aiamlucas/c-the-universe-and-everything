# get_next_line BONUS

Create two files:
- file1.txt
- file2.txt

--------------------------------------------------

printf "A1\nA2\n" > file1.txt
printf "B1\nB2\n" > file2.txt

--------------------------------------------------

Expected output:

fd1 -> A1
fd2 -> B1
fd1 -> A2
fd2 -> B2

--------------------------------------------------

If the output mixes lines or repeats data,
the BONUS implementation is incorrect.
