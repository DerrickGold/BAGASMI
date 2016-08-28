;[Debug Text]
[Strict Registers]
;[Debug Lines]
[Compile]
[Flip Mode 1]
[CPU_freq 13]
[Console 1]

;$s0 - current frame buffer

;=============================================
_start:
;=============================================

;set initial stylus position to center screen
;testing hex and binary assignments
set         $PENX, #0x80
set         $PENY, #0b01100000


;=============================================
;main program loop
;=============================================
;load circle radius argument to a0 for DrawCircle
MainLoop:
	getlcd 		   $s0, $0
    call           CheckButtons, $s0, $0, $0, $0

    ;load x and y coord arguments for DrawCircle
    call            drawSolidCircle, #68, $PENX, $PENY, $0

    flip           $0                       ;flip the bottom screen
    jump           MainLoop                    ;continue main loop


;stop the program here
ExitPrgm:          halt


;=============================================
;CheckButtons:
;   Handles all input. If start is pressed,
;   program exits. If the touch screen is pressed,
;   the circle position is updated accordingly.
;Registers:
;   t0 = current keys down
;	$a0 = current screen buffer to clear
;=============================================
CheckButtons:
    getkey          $t0
    chkkeynew       $t0, #8

    ;if key is pressed then exit
    jmpp            ExitPrgm

    ;if bottom screen is touched, clear it
    chkkeynew       $t0, #4096, touched
    jump            noTouch

    ;if touched, clear screen
touched: 	gfxmemset 	$a0, 0, #49512

    ;otherwise jump back into the main loop
noTouch:        jump 	$ra


;=============================================
;drawSolidCircle:
;   Draws a multicolored circle by drawing circles
;   from a radius of 1 to radius $a0
;Registers:
;   function arguments:
;   a0 = radius iterations
;   a1 = circle x position
;   a2 = circle y position
;
;   local variabls:
;   t0 = loop counter
;==============================================
drawSolidCircle:
    push            $ra                                 ;store return address

    set             $t0, #1                             ;initialize radius to 1
    loopto          #0, $t0, $a0, #1

        ;push some registers to the stack that may be clobbered by function calls
        push $t0
        push $a0

        ;generate random circle color
        randm       $a3, #32768
        add         $a3, $a3, #32767

        ;draw circle
        call        DrawCircle, $t0, $a1, $a2, $a3

        ;restore clobbered registers
        pop         $a0
        pop         $t0

    loopback        $0                                     ;continue with loop 0

    pop             $ra                                  ;restore return address
    jumpr           $ra                                      ;Exit function call






;=============================================
;DrawCircle:
;   Draws a circle outline
;
;Registers:
;   function arguments:
;   a0 = circle radius
;   a1 = circle x position
;   a2 = circle y position
;   a3 = circle color
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
;	t8 = screen buffer
;==============================================
DrawCircle:
    ;some initialization
    sub         $t0, $a0, #1                                ;f = current radius - 1
    set         $t1, #1                                     ;set ddf_x var
    mul         $t2, $a0, #-2                               ;set ddf_y

    ;set x and y
    set         $t3, $0                                  ;x = 0
    set         $t4, $a0                                    ;y = current radius

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
        gfxsetpix 	$s0, #256, $t6, $t7, $a3

        ;draw X0-X, Y0+y
        sub         $t6, $a1, $t3                           ;x0 - x
        gfxsetpix 	$s0, #256, $t6, $t7, $a3

        ;draw X0+x, Y0-Y
        add         $t6, $a1, $t3                           ;x0 + x
        sub         $t7, $a2, $t4                           ;y0 - y
        gfxsetpix 	$s0, #256, $t6, $t7, $a3

        ;draw x0 - x, y0 - y
        sub         $t6, $a1, $t3                           ;x0 - x
        gfxsetpix 	$s0, #256, $t6, $t7, $a3

        ;draw x0+y, y0+x
        add         $t6, $a1, $t4                           ;x0 + y
        add         $t7, $a2, $t3                           ;y0 + x
        gfxsetpix 	$s0, #256, $t6, $t7, $a3

        ;draw x0 -y, y0 + x
        sub         $t6, $a1, $t4                           ;x0 - y
        gfxsetpix 	$s0, #256, $t6, $t7, $a3

        ;draw  x0 + y, y0 - x
        add         $t6, $a1, $t4                           ;x0 + y
        sub         $t7, $a2, $t3                           ; y0 - x
        gfxsetpix 	$s0, #256, $t6, $t7, $a3

        ;draw x0 - y, y0 - x
        sub         $t6, $a1, $t4                           ;x0 - y
        gfxsetpix 	$s0, #256, $t6, $t7, $a3
    loopback        #1
    jump $ra
