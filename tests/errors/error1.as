; file with errors of second iter.
mov FAKE, LABEL1
mov LABEL1, FAKE
mov r1, LABEL1
jmp FAKE(r1,r2)
bne FAKE(FAKE2,LABEL1)
sub r1, r4
bne LABEL1(r1,r2)
bne LABEL1(r1,FAKE)


LABEL1: .data 1, 3, -4
