_start:
    PRINT "no jumping here!"
    OUTC '10
    JUMP TEST1, #1
    PRINT "end of all jumps"
HALT

TEST1 JUMP TEST2, #1
    PRINT "in test 1 stuff!"
    OUTC '10
    JMPBK

TEST2 JUMP TEST3, #1
    PRINT "in test 2!"
    OUTC '10
    JMPBK

TEST3 JUMP TEST4, #1
    PRINT "in test3!"
    OUTC '10
    JMPBK

TEST4 JUMP TEST5, #1
    PRINT "this is test 4"
    OUTC '10
    JMPBK

TEST5 PRINT "last test 5"
    OUTC '10
    JMPBK

