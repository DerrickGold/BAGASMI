;[Debug Text]
[Strict Registers]
;[Debug Lines]
;[Compile]
[Flip Mode 1]
[CPU_freq 13]
[Console 1]

;=============================================
_start:
;=============================================

;set initial stylus position to center screen
set         $PENX, #128
set         $PENY, #96


;=============================================
;main program loop
;=============================================
;load circle radius argument to a0 for DrawCircle
MainLoop:
    jump           CheckButtons, #1

    ;load x and y coord arguments for DrawCircle
    call            DrawCircle, #68, $PENX, $PENY, $0

	flip           $0                       ;flip the bottom screen
	jump           MainLoop                    ;continue main loop


;=============================================
;CheckButtons:
;   Handles all input. If start is pressed,
;   program exits. If the touch screen is pressed,
;   the circle position is updated accordingly.
;Registers:
;   t0 = current keys down
;=============================================
CheckButtons:
    getkey          $t0
    chkkeynew       $t0, #8

    ;if key is pressed then exit
    jmpp            ExitPrgm

    ;if bottom screen is touched, clear it
    chkkeynew       $t0, #4096
    jmpp            touched
    jump            noTouch
    ;if touched, clear screen
touched:
    clrscrn         $0
    ;jump back into the main loop
noTouch:
    jmpbk                                           ;exit function call


;=============================================
;DrawCircle:
;   Draws a multicolored circle by drawing circles
;   from a radius of 1 to radius $a0
;Registers:
;   function arguments:
;   a0 = max radius
;   a1 = circle x position
;   a2 = circle y position
;
;   local variabls:
;   t0 = f
;   t1 = ddF_x
;   t2 = ddF_y
;   t3 = x
;   t4 = y
;   t5 = temp pixel position
;   t6 = pixel draw x
;   t7 = pixel draw y
;   t8 = color of pixel
;   t9 = current circle radius
;==============================================
DrawCircle:
    set             $t9, #1                                     ;initialize radius to 1
    loopto          #0, $t9, $a0, #1
        ;pick a random color to draw as
        randm       $t8, #32768
        add         $t8, $t8, #32767

        ;some initialization
        sub         $t0, $t9, #1                                ;f = current radius - 1
        set         $t1, #1                                     ;set ddf_x var
        mul         $t2, $t9, #-2                               ;set ddf_y

        ;set x and y
        set         $t3, $0                                  ;x = 0
        set         $t4, $t9                                    ;y = current radius

        ;while x < y
        loopto          #1, $t3, $t4, #1                        ;for x < y; t3 += 1
            iflt        $t0, #0, continueCircle                 ;if f >= 0 then..
            decr        $t4, #1                                 ;y--
            add         $t2, $t2, #2                            ;ddf_y+=2
            add         $t0, $t0, $t2                           ;f += ddf_y

        continueCircle:
            add         $t1, $t1, #2                            ;ddF_x += 2
            add         $t0, $t0, $t1                           ; f += ddF_x

            ;===================
            ;plot the pixels now
            ;===================
            ;draw X0+X, Y0+y
            add         $t6, $a1, $t3                           ;x0 + x
            add         $t7, $a2, $t4                           ;y0 + y
            setpixel    $0, $t6, $t7, $t8

            ;draw X0-X, Y0+y
            sub         $t6, $a1, $t3                           ;x0 - x
            setpixel    $0, $t6, $t7, $t8

            ;draw X0+x, Y0-Y
            add         $t6, $a1, $t3                           ;x0 + x
            sub         $t7, $a2, $t4                           ;y0 - y
            setpixel    $0, $t6, $t7, $t8

            ;draw x0 - x, y0 - y
            sub         $t6, $a1, $t3                           ;x0 - x
            setpixel    $0, $t6, $t7, $t8

            ;draw x0+y, y0+x
            add         $t6, $a1, $t4                           ;x0 + y
            add         $t7, $a2, $t3                           ;y0 + x
            setpixel    $0, $t6, $t7, $t8

            ;draw x0 -y, y0 + x
            sub         $t6, $a1, $t4                           ;x0 - y
            setpixel    $0, $t6, $t7, $t8

            ;draw  x0 + y, y0 - x
            add         $t6, $a1, $t4                           ;x0 + y
            sub         $t7, $a2, $t3                           ; y0 - x
            setpixel    $0, $t6, $t7, $t8

            ;draw x0 - y, y0 - x
            sub         $t6, $a1, $t4                           ;x0 - y
            setpixel    $0, $t6, $t7, $t8

        loopback        #1

    loopback        $0                       ;continue with loop 0
    jumpr           $ra                         ;Exit function call



;stop the program here
ExitPrgm:
    halt
