;[Debug Text]
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
    BUTTONS_DONE JUMP DRAW_CIRCLE
    DONE_CIRCLE FLIP #0
JUMP MAINLOOP


;check if start key is pressed
CHECK_BUTTONS GETKEY $R15
    CHKKEY $R15,#8
    ;if key is pressed then exit
    JMPP ENDPRGM

    ;if bottom screen is touched, clear it
    CHKKEY $R15,#4096
    JMPZ NO_TOUCH
    ;if touched, clear screen
    CLRSCRN #0
    ;jump back into the main loop
    NO_TOUCH JUMP BUTTONS_DONE

;the circle drawing algorithm
;randomly generate a radius with a minimum of 2
DRAW_CIRCLE RANDM $R0,#68
    ADD $R0,#2

    ;pick a random color to draw as
    RANDM $R13,#32768
    ADD $R13,#32767

    ;set f variable
    SET $R1,$R0
    SUB $R1,#1
    ;set ddf_x var
    SET $R2,#1
    ;set ddf_y
    SET $R3,#-2
    MUL $R3,$R0
    ;set x and y
    SET $R4,#0
    SET $R5,$R0

    ;set some axis points on the circle
    ;center of circle is the stylus position
    SET $R7,$PENX
    SET $R8,$PENY
    ADD $R8,$R0
    SETPIXEL #0,$R7,$R8,$R13

    LOAD $R8,$PENY
    SUB $R8,$R0
    SETPIXEL #0,$R7,$R8,$R13

    LOAD $R8,$PENY
    ADD $R7,$R0
    SETPIXEL #0,$R7,$R8,$R13

    LOAD $R7,$PENX
    SUB $R7,$R0
    SETPIXEL #0,$R7,$R8,$R13

    ;while x < y
    CIRCLE_LOOP SET $R6,$R4
        SUB $R6,$R5
        JMPP END_CIRCLE

        ;if f >= 0
        SUB $R1,#0
        JMPN CONT_CIRCLE

        ;y--
        SUB $R5,#1
        ;ddf_y+=2
        ADD $R3,#2
        ;f += ddf_y
        ADD $R1,$R3

        ;x++   
        CONT_CIRCLE ADD $R4,#1
        ADD $R2,#2
        ADD $R1,$R2

        ;plot the pixels now

        ;set both X0-X and X0+X as variables on stack
        LOAD $R14,$PENX
        STORE $R14,X0-X
        ADD $R14,$R4
        STORE $R14,X0+X
        LOAD $R14,X0-X
        SUB $R14,$R4
        STORE $R14,X0-X

        ;now set Y0-Y and Y0+Y
        LOAD $R14,$PENY
        STORE $R14,Y0-Y
        ADD $R14,$R5
        STORE $R14,Y0+Y
        LOAD $R14,Y0-Y
        SUB $R14,$R5
        STORE $R14,Y0-Y

        ;now for X0+Y and X0-Y
        LOAD $R14,$PENX
        STORE $R14,X0-Y
        ADD $R14,$R5
        STORE $R14,X0+Y
        LOAD $R14,X0-Y
        SUB $R14,$R5
        STORE $R14,X0-Y

        ;now for Y0+X and Y0-Y
        LOAD $R14,$PENY
        STORE $R14,Y0-X
        ADD $R14,$R4
        STORE $R14,Y0+X
        LOAD $R14,Y0-X
        SUB $R14,$R4
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

        JUMP CIRCLE_LOOP

    END_CIRCLE JUMP DONE_CIRCLE

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
