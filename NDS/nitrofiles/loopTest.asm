
[CPU_freq 13]
[Console 1]

_start:
    ;loop from 0 to 4
    SET $R0,#0
    LOOPTO #0,$R0,#5,#1
        OUT $R0
        OUTC '44
    LOOPBACK #0
    OUTC '10
    PRINT "DONE!"
HALT
