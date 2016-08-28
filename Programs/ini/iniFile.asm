;[Debug Text]
[Debug Lines]
;[Compile]
;[Flip Mode 1]
;[CPU_freq 13]
[Console 1]

;start of the program
_start:

    ;open ini file
    PRINT "Opening ini file\n"
    INI_OPEN "test.ini"

    ;read an integer value from it and store in $R0
    PRINT "reading an integer value\n"
    INI_GETINT $R0, "Main", "integer", #-1 ;if the value cannot be found, -1 is the default value returned

    ;output the value
    OUT $R0
    OUTC <\n

    ;read a string from the ini file and store it in our string buffer
    PRINT "reading a string\n"
    INI_GETSTR stringBuf, #32, "Misc", "no_spaces_in_names", "failed" ;if the string couldn't be found, return "failed" as read text

    PRINT stringBuf
    OUTC <\n

    ;write a new integer to the file
    PRINT "Writing new int\n"
    INI_SETINT "Main", "integer", #12345

    ;write a new string to the file
    PRINT "Writing new string entry\n"
    INI_SETSTR "Main", "text", "brynnana"

    PRINT "Writing new entry!\n"
    ;make a new entry in the ini file
    INI_SETINT "giggity", "floop", #5

    PRINT "Writing a complete string!\n"
    INI_SETSTR "PENISTICULAR", "value", "this is a sentence!"

    PRINT "Writing all!\n"
    INI_WRITE "test.ini"

    INI_CLOSE
HALT

;string buffer for 32 characters
stringBuf ARRAY '32
