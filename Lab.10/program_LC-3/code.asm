        .ORIG x3000
        ADD     R0, R0, #0
        BRz     Stop
        BRp     Pos
        NOT     R0, R0 ; Negate R0
        ADD     R0, R0, #1
        NOT     R1, R1 ; Nagate R1
        ADD     R1, R1, #1
Pos     ADD R2, R2, #1
Loop    AND R3, R0, R2
        BRnp    BitOne
        ADD     R1, R1, R1
        ADD     R2, R2, R2
        BRnzp   Loop
BitOne  ADD R7, R7, R1
        ADD     R1, R1, R1
        ADD     R2, R2, R2
        ADD     R4, R0, #-1 ; Remove the lowest
        AND     R0, R4, R0  ; 1 in R0
        BRnp    Loop
Stop    HALT
        .END
