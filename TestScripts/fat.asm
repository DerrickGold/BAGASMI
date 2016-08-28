;[Debug Text]
;[Compile]
[CPU_freq 13]
[Auto Error]


_start:

    PRINT "opening test.txt\n"

    FATOPEN #0,"test.txt","wb"
    IFERR FAILED

    PRINT "file opened!\n"


    FATPRINT #0, "Testing string"
    PRINT "wrote to file\n"

    FATCLOSE #0
    FATOPEN #0, "test.txt", "rb"

    ;read "Hello World!" from the file
    FATREAD #0,TEST_STRING,#1,#13

    ;print the data to screen
    PRINT TEST_STRING

    JUMP END

    FAILED PRINT "Couldn't open file\n"
END HALT

;an array of 16 bytes(')
TEST_STRING ARRAY '16
