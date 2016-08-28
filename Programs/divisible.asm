
;[Debug Text]
[Debug Lines]
;[Compile]
[Flip Mode 1]
[CPU_freq 13]
[Console 1]

;prints multiples of 2

_start:

    SET $R0,#1
    ;TEXT_LOOP INCR $R0
    LOOPTO #0, $R0, #21, #1

        JUMP CHECK_DIVISIBLE, #1
    LOOPBACK #0
    ;JUMP TEXT_LOOP
HALT

CHECK_DIVISIBLE REM $R1, $R0, #2
    ;set a second argument to jmpz to disable jmpbk for this statement
    JMPZ IS_DIVISIBLE

    ;the jmpbk statement will now go back into TEXT_LOOP instead of the previous jump statement
    OUT $R0
    PRINT " not a multiple of 2"
    OUTC '10
    JMPBK

    IS_DIVISIBLE OUT $R0
        PRINT " is a value of 2!"
        OUTC '10
        JMPBK
