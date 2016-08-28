[Console 1]


_start:
    PRINT "String fun!"
    OUTC '10

    ;lets put some text to our string from below
    STRNCPY TEST_STRING,"Hello World!",#128

    ;grab the length of the string
    STRLEN $R0,TEST_STRING

    ;out put some string details
    PRINT "Here is the output of the string:"
    OUTC '10
    PRINT TEST_STRING
    OUTC '10
    PRINT "And it has a length of: "
    OUT $R0

    ;copy string 1 to string 2, using the length of the string
    STRNCPY TEST_STRING2,TEST_STRING,$R0

    OUTC '10
    PRINT "Here are the contents of string 2!"
    OUTC '10
    PRINT TEST_STRING2

    ;check if both strings are equal
    STRNCMP TEST_STRING,TEST_STRING2,#128
    JMPZ EQUAL
        OUTC '10
        PRINT "Hmm, they should be but aren't!"
        OUTC '10
        JUMP DONE_COMPARE1

    EQUAL OUTC '10
        PRINT "Yes both strings are equal!"
        OUTC '10

    ;now lets add stuff to the string
    DONE_COMPARE1 NOP
    STRNCAT TEST_STRING2," Fuck You!",#20


    PRINT "New string 2"
    OUTC '10 
    PRINT TEST_STRING2
    OUTC '10
    
    ;now lets compare it
    STRNCMP TEST_STRING,TEST_STRING2,#128
    JMPZ DONE_PRGM
    PRINT "Yup, the strings are different"


DONE_PRGM HALT

;make a test string that can hold 128 characters
TEST_STRING ARRAY '128
;have another string
TEST_STRING2 ARRAY '128
