;[Debug Text]
;[Compile]
[CPU_freq 13]

_start:
;draw a 32x32 square
DRWBALL LOAD $R0,DRAWX
    SET $R2,#32
    ;loop vertically
    YLOOP SUB $R2,#0
        JMPZ DUNDRW
        SET $R3,#32

        LOAD $R1,DRAWY
        ADD $R1,$R2

        ;loop horizontally
        XLOOP SUB $R3,#0
            ;if x limit reached, loop through y again
            JMPZ XFIN

            ;plot a pixel
            LOAD $R0,DRAWX
            ADD $R0,$R3

            SETPIXELF #0,$R0,$R1,#65535
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

;draw everything once instead of per loop
FLIP #0

;end of program
END HALT

DRAWX DATA #0
DRAWY DATA #0
