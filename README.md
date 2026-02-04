NUMASFUK
NUMASFUK is a chaotic, tape‑driven esoteric programming language built around single‑character opcodes, raw jumps, and a 16‑cell circular string tape.
It embraces danger, weirdness, and direct memory hits — a language that rewards precision and chaos in equal measure.

NUMASFUK programs use .naf as their file extension.

Features
16‑cell circular tape of strings

Each string max 64 characters

Single numeric register

Raw character‑index jumps

Hex and binary byte output

Repeat operator

Input, random digits, comparisons

Zero‑check and nonzero‑check jumps

Fully unsafe by design

Core Opcodes
Opcode	Description
1	Append "1" to current string
2	Move to next string (create if needed)
3	Set current string to "0.0"
4	Read a line of input into current string
5(hex)	Print bytes from hex literal
6(n)	Repeat next opcode n times
7	Append random digit
8(bin)	Print one byte from binary literal
9	Dump all strings and exit
Extended Opcodes (v2)
Opcode	Description
A	Move tape pointer forward
B	Move tape pointer backward
E	Set register = 0 if empty, else 1
0(n)	Set register to n
+(n)	Add n to register
-(n)	Subtract n from register
C(hex)	Compare first byte of string to hex literal
J(n)	Jump to raw index n
Z(n)	Jump if register == 0
N(n)	Jump if register != 0
Example Program (echo until empty)
numasfuk
# read first line
4

# check if empty
E Z(40)

# print newline
5(0A)

# dump strings (prints the input)
9

# clear string
3

# read again
4

# loop
J(10)

# exit
5(0A)
Running a Program
Use the Ruby interpreter:

Code
ruby numasfuk.rb program.naf
File Extension
NUMASFUK files use:

Code
.naf
To force GitHub to treat .naf files as NUMASFUK, add this to your repo:

Code
*.naf linguist-language=NUMASFUK
*.naf linguist-detectable=true
License
NUMASFUK is released under the Unlicense, placing it in the public domain.
You may use, modify, distribute, or relicense it without restriction.
