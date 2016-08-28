;DS2 settings
[Flip Mode 2]
[CPU_freq 13]

;disable the console on NDS
[Console 0]

;start of the program
_start:
    ;clear the top screen (for DS)
    CLRSCRN #1
    FLIP #1

    ;set up some colors to flash too
    ;red
    RGB $R0,#31,#0,#0
    ADD $R0,#32768
    SET COLORS[0],$R0
    ;green
    RGB $R0,#0,#31,#0
    ADD $R0,#32768
    SET COLORS[1],$R0
    ;blue
    RGB $R0,#0,#0,#31
    ADD $R0,#32768
    SET COLORS[2],$R0

    ;store the screen number in register 15
    SET $R15,#0

    ;pick a random color to use
    FLASH_LOOP SET $R0,#0
        LOOPTO #0,$R0,#3,#1 
            LDARY $R2,COLORS,$R0

            ;only flash when the screen is set to 0
            SET $R3,$R15
            SUB $R3,#1
            JMPZ SKIP_SCREEN

            SETSCRNCOL $R15,$R2
            FLIP $R15

            SKIP_SCREEN JUMP CHECK_INPUT
            DONE_INPUT_CHECK VSYNC
        LOOPBACK #0
    JUMP FLASH_LOOP

HALT
;array of colors to use
COLORS ARRAY #3

CHECK_INPUT GETKEY $R10
    ;check if R is pressed
    CHKKEY $R10,#256
    JMPP R_PRESSED

    ;check if start is pressed
    CHKKEY $R10,#8
    JMPP START_PRESSED
    
    ;otherwise exit this check
    JUMP EXIT_INPUT

;R_PRESSED===============================================================
    ;switch the screen if R is newly pressed
    R_PRESSED SUB $R1,#0
    ;R is still on its last press
    JMPP DONE_INPUT_CHECK
    ;set R to its new press
    INCR $R1

    ;clear current screen before switching
    CLRSCRN $R15
    FLIP $R15

    ;now to switch the screen
    INCR $R15
    ;check if its greater than 2
    SET $R5,$R15
    SUB $R5,#2
    JMPN DONE_INPUT_CHECK
    ;if it is, then set to 0
    SET $R15,#0
    JUMP DONE_INPUT_CHECK

;START_PRESSED===========================================================
    ;exit program when start is pressed
    START_PRESSED HALT

;EXIT_INPUT==============================================================
    ;$R1 is used to ensure that the R key is detected as a new press
    EXIT_INPUT SET $R1,#0
        JUMP DONE_INPUT_CHECK

