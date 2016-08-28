;[Debug Text]
;[Compile]
;[Flip Mode 1]
;[CPU_freq 13]
[Console 1]

;start of the program
_start:

    PRINT "Setting register to 3333"
    OUTC '10
    SET $R0,#3333

    PRINT "Storing register to array index"
    OUTC '10
    ;STARY $R0,TESTARY,#19
    ;STARY $R0,TESTARY[76],#0
    SET TESTARY[76],$R0

    PRINT "Loading register from array index "
    OUTC '10
    LDARY $R12,TESTARY,#19
    OUT $R12
    OUTC '10

    PRINT "test var value: "
    OUT TESTVAR

HALT

TESTARY ARRAY #20
TESTVAR DATA #13
