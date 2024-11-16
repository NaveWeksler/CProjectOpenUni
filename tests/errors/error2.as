; file with errors of first iter.

LABEL: .data "hello"
LABEL2: .data 1, 2a, 3
LABEL3: .string abc

; bad instructions.
cmp r1,
jmp LABEL(r1,)
mov r2
bne
END: stop r1