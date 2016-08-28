;[Debug Text]
;[Compile]
[CPU_freq 13]
[Console 1]

;R0 = draw x
;R1 = draw y
;R2 = y loops
;R3 = x loops

;R4 = initial draw x pos
;R5 = initial draw y pos

_start:
;draw a 32x32 square
DRWBALL SET $R2,#32

    ;loop vertically
    SET $R1,$R5
    YLOOP SUB $R2,#1
        JMPN DUNDRW

        SET $R3,#32
        INCR $R1
        SET $R0,$R4
        
        ;loop horizontally
        XLOOP SUB $R3,#1
            ;if x limit reached, loop through y again
            JMPN XFIN

            ;plot a pixel
            SETPIXELF #0,$R0,$R1,#65535
            INCR $R0

        JUMP XLOOP
    XFIN JUMP YLOOP

    ;move ball across screen
    DUNDRW ADD $R4,#3
    ADD $R5,#3
    SET $R1,$R5
    SUB $R1,#160
    JMPN DRWBALL

;draw everything once instead of per loop
FLIP #0

;end of program
END HALT
