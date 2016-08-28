;[Debug Text]
;[Compile]
[CPU_freq 13]
[Console 1]
[Auto Error]

;This program will open an 8 bit bitmap
;and then write it to the screen!


_start:

    print       "opening bitmap.bmp\n"

    fatopen     #0, "bitmap.bmp", "rb"
    iferr       FAILED

    print       "file opened!\n"

    ;=============================================
    ;Lets read some header info
    ;=============================================

    ;grab offset to image data
    fatseek     #0, #10, #0
    fatread     #0, BMP_OFFSET, #4, #1

    ;grab image width
    fatseek     #0, #18, #0
    fatread     #0, BMP_WIDTH, #4, #1

    ;read image height
    fatseek     #0, #22, #0
    fatread     #0, BMP_HEIGHT, #4, #1

    ;read bit count
    fatseek     #0, #28, #0
    fatread     #0, BMP_BIT_COUNT, #1, #2

    ;read number of colors
    fatseek     #0, #46, #0
    fatread     #0, COLOR_COUNT, #4, #1

    ;=============================================
    ;Print out the header info
    ;=============================================

    ;output info of image
    print       "Width: "
    out         BMP_WIDTH
    outc        <\n

    print       "Height: "
    out         BMP_HEIGHT
    outc        <\n

    print       "Bit depth: "
    out         BMP_BIT_COUNT
    outc        <\n

    print       "Colors: "
    out         COLOR_COUNT
    outc        <\n

    ;================================================
    ;now to load the bitmap data
    ;first, lets start with the palette
    ;================================================
    ;seek past the header information
    fatseek     #0, #54, #0

    print       "Reading palette!\n"
    fatread     #0, PALETTE, #4, #256

    ;now lets process the palette
    ;$t0 is our counter from 0 here
    ;$t1 is our palette position/
    ;$t2 to t4 are red,green,blue values
    ;$t5 is the final color


    ;load palette address for memory access
    ldaddr      $s0, PALETTE
    loopto      #0, $t0, COLOR_COUNT, #1
        ;==================================================
        ;set position in the palette buffer to read for red
        ;==================================================
        ;Grab red value - [(i<<2)+2]
        shl     $t1, $t0, #2
        add     $t1, $t1, #2
        add     $t1, $t1, $s0

        loadc   $t2, ($t1)
        ;Convert to value between 0 and 31 - ([(i<<2)+2] >> 3) & 31
        shr     $t2, $t2, #3
        and     $t2, $t2,#31

        ;Print value
        out     $t2
        outc    <,

        ;==================================================
        ;set position in pal buffer to read green - [(i<<2)+1]
        ;==================================================
        decr    $t1
        loadc   $t3, ($t1)

        ;([(i<<2)+1] >> 3) & 31
        shr     $t3, $t3, #3
        and     $t3, $t3, #31

        ;print
        out     $t3
        outc    <,

        ;==================================================
        ;set position in pal buffer to read blue - [(i<<2)]
        ;==================================================
        decr    $t1
        loadc   $t4, ($t1)

        ;([i<<2] >> 3) & 31
        shr     $t4, $t4, #3
        and     $t4, $t4, #31

        ;print
        out     $t4
        outc    <,

        ;==================================================
        ;combine the r,g,b values to a 16 bit integer
        ;==================================================
        rgb     $t5, $t2, $t3, $t4

        out     $t5
        outc    <\n

        ;store the new value back into the palette array
        ;since there is enough space to hold each entry
        storeh  $t5, ($t1)

    loopback        #0

    ;================================================
    ;Next, read the bitmap data
    ;================================================
    ;seek to graphics offset in file
    fatseek     #0, BMP_OFFSET, #0

    ;s0 - palette data address
    ;t0 - outer loop handling image height
    ;t1 - inner loop handling image width
    ;t2 - reversed y coordinates ($t0) to display bitmap rightside up

    set         $t0, #0                             ;reset our counter back to 0
    loopto      #0, $t0, BMP_HEIGHT, #1

        ;read one row of data
        fatread     #0, PIXEL, #1, BMP_WIDTH

        ;now loop through the line read and plot the pixels
        ;$R14 stores the byte value of the image (refers to palette index)
        ;$R15 stores the color obtained from the palette

        set         $t1,#0
        loopto      #1, $t1, BMP_WIDTH, #1
            ;first get pixel data from buffer
            laaryc  $t3, PIXEL, $t1
            loadc   $t3, ($t3)

            ;next get color from palette
            laary   $t3, PALETTE, $t3

            ;place pixel on screen3
            ;bitmaps are flipped on the y axis, so flip it
            sub         $t2, BMP_HEIGHT, $t0
            setpixel    #0, $t1, $t2, ($t3)

        loopback    #1
    loopback       #0

    ;done loading, close the file
    fatclose        #0

    ;now display the image
    flip            #0
    jump            HOLD_LOOP

    ;If the file couldn't be opened/found in FOPEN
    FAILED          print "Couldn't open file\n"

    ;stall the program so it doesn't exit until any key is pressed
    set             $t0, #0
    HOLD_LOOP       getkey $t0
        jmpp    FINAL
    jump    HOLD_LOOP

FINAL halt

;bitmap header info
BMP_WIDTH DATA #0
BMP_HEIGHT DATA #0
BMP_BIT_COUNT DATA #0
COLOR_COUNT DATA #0
BMP_OFFSET DATA #0

;bitmap palette
PALETTE ARRAY #256

;pixel buffer
PIXEL ARRAY '21
