[Console 1]
_start:
    PRINT "entering loop(5000x)"
    OUTC '10
    LOOPTO #0,$R1,#5000,#1
    LOOPBACK #0
    OUTC '10
    PRINT "Done!"
HALT
