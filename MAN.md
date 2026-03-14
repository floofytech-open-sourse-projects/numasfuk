NUMASFUK(1)                 User Commands                 NUMASFUK(1)

NAME
    numasfuk – chaotic, tape‑driven esoteric programming language

SYNOPSIS
    numasfuk file.naf
    numasfuk --dump file.naf
    numasfuk --version
    numasfuk --help

DESCRIPTION
    NUMASFUK is a deliberately unsafe, tape‑driven esoteric programming
    language built around single‑character opcodes, raw character‑index
    jumps, and a 16‑cell circular string tape. It emphasizes minimalism,
    unpredictability, and direct memory manipulation.

    Programs are stored in plain text files using the .naf extension.

LANGUAGE FEATURES
    • 16‑cell circular tape of strings  
    • Each string may contain up to 64 characters  
    • One numeric register  
    • Raw character‑index jumps  
    • Hex and binary byte output  
    • Repeat operator  
    • Input operations  
    • Random digit insertion  
    • Comparison operators  
    • Zero‑check and nonzero‑check jumps  
    • Fully unsafe by design  

CORE OPCODES
    1            Append "1" to current string
    2            Move to next string (create if needed)
    3            Set current string to "0.0"
    4            Read a line of input into current string
    5(hex)       Print bytes from hex literal
    6(n)         Repeat next opcode n times
    7            Append random digit (0–9)
    8(bin)       Print one byte from binary literal
    9            Dump all strings and exit

EXTENDED OPCODES (v2)
    A            Move tape pointer forward
    B            Move tape pointer backward
    E            Set register = 0 if current string empty, else 1
    0(n)         Set register to n
    +(n)         Add n to register
    -(n)         Subtract n from register
    C(hex)       Compare first byte of string to hex literal
    J(n)         Jump to raw character index n
    Z(n)         Jump if register == 0
    N(n)         Jump if register != 0

EXAMPLE
    The following program echoes input until an empty line is read:

        4          read first line
        E Z(40)    check if empty
        5(0A)      print newline
        9          dump strings
        3          clear string
        4          read again
        J(10)      loop
        5(0A)      exit


FILE EXTENSION
    NUMASFUK source files use the .naf extension.


LICENSE
    NUMASFUK is released under the Unlicense, placing it in the public
    domain. You may use, modify, distribute, or relicense it without
    restriction.

AUTHOR
    FloofyTech 

SEE ALSO
    esolang(7), brainfuck(1), befunge(1)
