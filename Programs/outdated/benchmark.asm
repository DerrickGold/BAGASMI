;program used for benchmarking the interpreter

;[Debug Text]
;[Compile]
[CPU_freq 13]

_start:
;draw a 32x32 square
DRWBALL LOAD $R0,DRAWX
    LOAD $R1,DRAWY
    SET $R2,#32

    ;loop vertically
    YLOOP SUB $R2,#0
        JMPZ DUNDRW
        SET $R3,#32

        ;loop horizontally
        XLOOP SUB $R3,#0
            ;if x limit reached, loop through y again
            JMPZ XFIN

            ;plot a pixel
            SET $VX0,$R0
            ADD $VX0,$R3
            SET $VY0,$R1
            ADD $VY0,$R2
            SETPIXEL #0,$VX0,$VY0,#65535
            DECR $R3
        JUMP XLOOP
        XFIN DECR $R2
    JUMP YLOOP

    ;move ball across screen
    DUNDRW ADD $R0,#2
    STORE $R0,DRAWX
    ADD $R1,#2
    STORE $R1,DRAWY
    SUB $R1,#160
JMPN DRWBALL

FLIP #0

;end of program
END HALT

;data declarations
DRAWX DATA #0
DRAWY DATA #0
