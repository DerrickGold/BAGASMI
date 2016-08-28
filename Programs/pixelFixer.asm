;DS2 settings
[Strict Registers]
[Flip Mode 2]
;disable the console on NDS
[Console 1]
[Debug Lines]
[Compile]
;$s0 = screen buffer
;$s1 = current screen
;$s2 = COLORS array address
;$t0 = loop counter, misc
;$t1 = current color to flash

;start of the program
_start:
	;set start screen to screen 0
	set 		$s0, $0

    ;clear the top screen (for DS)
    getlcd 		$s0, $s1
    gfxmemset 	$s0, #0, #49152
    flip 		$s1

    ;set up some colors to flash
    ;red
    laary 		$s2, COLORS, #0
    rgb 		($s2), #31, #0, #0
    ;green
    laary 		$s2, COLORS, #1
    rgb 		($s2), #0, #31, #0
    ;blue
    laary 		$s2, COLORS, #2
    rgb 		($s2), #0, #0, #31

mainWhile:
	set 	$t0, #0
    loopto 	#0, $t0, #3, #1
    	laary 		$s2, COLORS, $t0
        gfxmemset 	$s0, ($s2), #49152
        flip 		$s1
        call 		CHECK_INPUT, $0, $0, $0, $0
       	vsync
        loopback 	#0
    jump 	mainWhile

	halt



;$t0 = key pressed
CHECK_INPUT:
	getkey 		$a0
    ;check if R is pressed
    chkkeynew 	$a0, #256, _R_pressed

    ;check if start is pressed
    chkkey 		$a0, #8, _START_pressed

    ;otherwise exit this check
    jumpr 			$ra


;switch the screen if R is newly pressed
_R_pressed:
	;clear current screen before switching
 	gfxmemset 	$s0, #0, #49152
 	flip 		$s1

    ;now to switch the screen
    incr 		$s1
    ;check if its greater than 2
    iflt 		$s1, #2, _input_ScrnUpdate
    ;if it is, then set to 0
    set 		$s1, $0
    jump 		_input_ScrnUpdate

    ;exit program when start is pressed
_START_pressed:
	halt

_input_ScrnUpdate:
	getlcd 		$s0, $s1
	jumpr 			$ra




;array of colors to use
COLORS ARRAY #3
