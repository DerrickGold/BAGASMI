
[CPU_freq 13]
[Console 1]

;data declarations and arrays are stored first in the ram
;then constants are stored in order
;values in ram are word aligned, byte values take up 4 bytes

_start:
    PRINT "testing some ram stuff"
    OUTC '10

    ;print value of position 0 of ram (TESTVAR)
    OUT $_SYSRAM
    OUTC '10

    ;print value of position 4 of ram (TESTVAR2)
    ;data in ram is 4 byte aligned, which is why TESTVAR2 is at position 4 rather than 1
    OUT $_SYSRAM_4
    OUTC '10

    ;load the value from byte 4 of ram (position 1 of an int array)
    LDARY $R0,$_SYSRAM,#1
    OUT $R0
    OUTC '10


    ;print out ram from base _SYSRAM
    SET $R0,#7
    RAM_LOOP INCR $R0
        SUB $R1,$R0,#29
        JMPP END_LOOP1

        LDARYC $R2,$_SYSRAM,$R0
        OUTC $R2
        OUTC '44
        JUMP RAM_LOOP

    ;print a new line between the two tests
    END_LOOP1 OUTC '10

    ;print out ram from a different base
    SET $R0,#0
    RAM_LOOP2 INCR $R0
        SUB $R1,$R0,#22
        JMPP END_PRGM

        LDARYC $R2,$_SYSRAM[7],$R0
        OUTC $R2
        OUTC '44
        JUMP RAM_LOOP2


    END_PRGM OUTC '10

    OUT $_SYSRAM_POS_
    OUTC '10
    SET $_SYSRAM_POS,#0
    OUT $_SYSRAM_POS_
    OUTC '10



    PRINT "DONE!"
HALT

TESTVAR DATA '50
TESTVAR2 DATA #1337
