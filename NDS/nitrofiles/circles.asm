;[Debug Text]
[Debug Lines]
;[Compile]
[Flip Mode 1]
[CPU_freq 13]
[Console 1]
;SPRITES:

;REGISTER USES:
;R0 = random radius
;R1 = f
;R2 = ddF_x
;R3 = ddF_y
;R4 = x
;R5 = y
;R6 = temp pixel position

;VX0 and VY0 registers are deprecated
;R7 = VX0
;R8 = VY0

;R13 = color of pixel
;R15 = exit button checking

;start of the program
_start:
;set initial stylus position to center screen
SET $PENX,#128
SET $PENY,#96

;main program loop
MAINLOOP JUMP CHECK_BUTTONS
    BUTTONS_DONE JUMP DRAW_CIRCLE, #1
    FLIP #0
JUMP MAINLOOP


;check if start key is pressed
CHECK_BUTTONS GETKEY $R15
    CHKKEYNEW $R15,#8
    ;if key is pressed then exit
    JMPP ENDPRGM

    ;if bottom screen is touched, clear it
    CHKKEYNEW $R15,#4096
    JMPZ NO_TOUCH
    ;if touched, clear screen
    CLRSCRN #0
    ;jump back into the main loop
    NO_TOUCH JUMP BUTTONS_DONE

;the circle drawing algorithm
;randomly generate a radius with a minimum of 2
DRAW_CIRCLE RANDM $R0,#68
    ADD $R0,$R0,#2

    ;pick a random color to draw as
    RANDM $R13,#32768
    ADD $R13,$R13,#32767

    ;set f variable
    SUB $R1,$R0,#1
    ;set ddf_x var
    SET $R2,#1
    ;set ddf_y
    MUL $R3,$R0,#-2
    ;set x and y
    SET $R4,#0
    SET $R5,$R0

    ;set some axis points on the circle
    ;center of circle is the stylus position
    ADD $R8,$R0,$PENY
    SETPIXEL #0,$PENX,$R8,$R13

    SUB $R8,$PENY,$R0
    SETPIXEL #0,$PENX,$R8,$R13

    ADD $R7,$PENX,$R0
    SETPIXEL #0,$R7,$PENY,$R13

    SUB $R7,$PENX,$R0
    SETPIXEL #0,$R7,$PENY,$R13

    ;while x < y
    LOOPTO #0,$R4,$R5,#1
        ;if f >= 0
        IFLT $R1, #0, CONT_CIRCLE
        ;y--
        DECR $R5,#1
        ;ddf_y+=2
        ADD $R3,$R3,#2
        ;f += ddf_y
        ADD $R1,$R1,$R3

        ;x++
        CONT_CIRCLE ADD $R2,$R2,#2
        ADD $R1,$R1,$R2

        ;plot the pixels now

        ;set both X0-X and X0+X as variables on stack
        ADD $R14,$PENX,$R4
        STORE $R14,X0+X
        SUB $R14,$PENX,$R4
        STORE $R14,X0-X

        ;now set Y0-Y and Y0+Y
        ADD $R14,$PENY,$R5
        STORE $R14,Y0+Y
        SUB $R14,$PENY,$R5
        STORE $R14,Y0-Y

        ;now for X0+Y and X0-Y
        ADD $R14,$PENX,$R5
        STORE $R14,X0+Y
        SUB $R14,$PENX,$R5
        STORE $R14,X0-Y

        ;now for Y0+X and Y0-Y
        ADD $R14,$PENY,$R4
        STORE $R14,Y0+X
        SUB $R14,$PENY,$R4
        STORE $R14,Y0-X


        ;now to place the pixels
        SETPIXEL #0,X0+X,Y0+Y,$R13
        SETPIXEL #0,X0-X,Y0+Y,$R13
        SETPIXEL #0,X0+X,Y0-Y,$R13
        SETPIXEL #0,X0-X,Y0-Y,$R13

        SETPIXEL #0,X0+Y,Y0+X,$R13
        SETPIXEL #0,X0-Y,Y0+X,$R13
        SETPIXEL #0,X0+Y,Y0-X,$R13
        SETPIXEL #0,X0-Y,Y0-X,$R13

    LOOPBACK #0
JMPBK

;stop the program here
ENDPRGM HALT

;some calculated positions
X0-X DATA #0
X0+X DATA #0
Y0-Y DATA #0
Y0+Y DATA #0
X0+Y DATA #0
X0-Y DATA #0
Y0+X DATA #0
Y0-X DATA #0
