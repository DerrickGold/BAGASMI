;[Debug Text]
;[Compile]
[CPU_freq 13]
[Console 1]
[Auto Error]

;This program will open an 8 bit bitmap
;and then write it to the screen!


_start:

    PRINT "opening bitmap.bmp\n"

    FATOPEN #0, "bitmap.bmp", "rb"
    IFERR FAILED

    PRINT "file opened!\n"

    ;=============================================
    ;Lets read some header info
    ;=============================================

    ;grab offset to image data
    FATSEEK #0, #10, #0
    FATREAD #0, BMP_OFFSET, #4, #1

    ;grab image width
    FATSEEK #0, #18, #0
    FATREAD #0, BMP_WIDTH, #4, #1

    ;read image height
    FATSEEK #0, #22, #0
    FATREAD #0, BMP_HEIGHT, #4, #1

    ;read bit count
    FATSEEK #0, #28, #0
    FATREAD #0, BMP_BIT_COUNT, #1, #2

    ;read number of colors
    FATSEEK #0, #46, #0
    FATREAD #0, COLOR_COUNT, #4, #1

    ;=============================================
    ;Print out the header info
    ;=============================================

    ;output info of image
    PRINT "Width: "
    OUT BMP_WIDTH
    OUTC <\n

    PRINT "Height: "
    OUT BMP_HEIGHT
    OUTC <\n

    PRINT "Bit depth: "
    OUT BMP_BIT_COUNT
    OUTC <\n

    PRINT "Colors: "
    OUT COLOR_COUNT
    OUTC <\n

    ;================================================
    ;now to load the bitmap data
    ;first, lets start with the palette
    ;================================================
    ;seek past the header information
    FATSEEK #0, #54, #0

    PRINT "Reading palette!\n"
    FATREAD #0, PALETTE, #1, #1024

    ;now lets process the palette
    ;$R0 is our counter from 0 here
    ;$R15 is our palette position
    ;$R3 to R5 are red,green,blue values
    ;$R14 is the final color

    LOOPTO #0, $R0, COLOR_COUNT, #1
        ;==================================================
        ;set position in the palette buffer to read for red
        ;==================================================
        ;Grab red value - [(i<<2)+2]
        SHL $R15, $R0, #2
        ADD $R15, $R15, #2
        LDARYC $R3,PALETTE,$R15

        ;Convert to value between 0 and 31 - ([(i<<2)+2] >> 3) & 31
        SHR $R3, $R3, #3
        AND $R3, $R3,#31

        ;Print value
        OUT $R3
        OUTC <, ;add a comma

        ;==================================================
        ;set position in pal buffer to read green - [(i<<2)+1]
        ;==================================================
        DECR $R15
        LDARYC $R4, PALETTE, $R15

        ;([(i<<2)+1] >> 3) & 31
        SHR $R4, $R4, #3
        AND $R4, $R4, #31

        ;print
        OUT $R4
        OUTC <,

        ;==================================================
        ;set position in pal buffer to read blue - [(i<<2)]
        ;==================================================
        DECR $R15
        LDARYC $R5, PALETTE, $R15 ;

        ;([i<<2] >> 3) & 31
        SHR $R5, $R5, #3
        AND $R5, $R5, #31

        ;print
        OUT $R5
        OUTC <,

        ;==================================================
        ;combine the r,g,b values to a 16 bit integer
        ;==================================================
        RGB $R14, $R3, $R4, $R5

        OUT $R14
        OUTC <\n

        ;store the new value back into the palette array
        ;since there is enough space to hold each entry
        STARY $R14, PALETTE, $R0

    LOOPBACK #0


    ;================================================
    ;Next, read the bitmap data
    ;================================================
    ;seek to graphics offset in file
    FATSEEK #0, BMP_OFFSET, #0

    ;this loops through and reads a line from the image
    ;then plots it to screen

    SET $R0, #0 ;reset our counter back to 0
    LOOPTO #0, $R0, BMP_HEIGHT, #1
        ;read one row of data
        FATREAD #0, PIXEL, #1, BMP_WIDTH

        ;now loop through the line read and plot the pixels
        ;$R14 stores the byte value of the image (refers to palette index)
        ;$R15 stores the color obtained from the palette
        ;$R6 is the reverse Y coordinate of $R0 to display the image right side up
        SET $R1,#0
        LOOPTO #1, $R1, BMP_WIDTH, #1

            ;grab color from pal
            LDARYC $R14, PIXEL, $R1
            LDARY $R15, PALETTE, $R14

            ;place pixel on screen3
            ;bitmaps are flipped on the y axis, so flip it
            SUB $R6, BMP_HEIGHT, $R0
            SETPIXEL #0, $R1, $R6, $R15

        LOOPBACK #1
    LOOPBACK #0

    ;done loading, close the file
    FATCLOSE #0

    ;now display the image
    FLIP #0
    JUMP HOLD_LOOP

    ;If the file couldn't be opened/found in FOPEN
    FAILED PRINT "Couldn't open file\n"

    ;stall the program so it doesn't exit until any key is pressed
    SET $R15, #0
    HOLD_LOOP GETKEY $R15
        JMPP FINAL
    JUMP HOLD_LOOP

FINAL HALT

;bitmap header info
BMP_WIDTH DATA #0
BMP_HEIGHT DATA #0
BMP_BIT_COUNT DATA #0
COLOR_COUNT DATA #0
BMP_OFFSET DATA #0

;bitmap palette
PALETTE ARRAY '1024

PIXEL ARRAY '21
