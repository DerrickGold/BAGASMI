;Benchmark program, revision 3

[CPU_freq 13]
[Console 1]
;[Debug Text]
;[Compile]

;R0 y loop
;R1 x loop
;R3 current x and y position
;R4 y loop + current x position
;R5 x loop + current x position

_start:
    ;draw from x and y position up to 160 on both axiis
    SET $R3,#0
    LOOPTO #0,$R3,#160,#3

        ;now draw the square
        SET $R4,$R3;

        ;this is the y values of the square
        SET $R0,#0
        LOOPTO #1,$R0,#32,#1
            SET $R5,$R3

            ;loop x values
            SET $R1,#0
            LOOPTO #2,$R1,#32,#1
                SETPIXELF #0,$R5,$R4,#65535
                INCR $R5
            LOOPBACK #2

            INCR $R4
        LOOPBACK #1

    LOOPBACK #0

FLIP #0
;end of program
HALT
