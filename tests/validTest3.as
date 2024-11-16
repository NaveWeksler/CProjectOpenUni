.entry LENGTH
.extern W
MAIN:   mov r3 ,LENGTH
LOOP:   jmp L1(#-1,r6)
        prn #-5
        bne W(r4,r5)
        sub r1, r4
        bne L3
L1:     inc K
mcr t1
    prn #-5
    prn r2
    sub r2, r3
endmcr

.entry LOOP

        bne LOOP(K,W)
END:    stop
STR:    .string "abcdef"
LENGTH: .data 6,-9,15
K:      .data 22
TEST:
    bne W(r4,r5)
    t1
    jmp L1(#-1,r6)
    t1
    t1
.extern L3