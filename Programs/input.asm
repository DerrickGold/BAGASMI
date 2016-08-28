;Program:
;Print if A is pressed, or released
;and Exit if Start is pressed.

;[Debug Text]
[Debug Lines]
;[Compile]
;[CPU_freq 13]
[Console 1]

_start:
;create a loop for the program to run in
;Lets store the current frame input in register 0.
inputLoop GETKEY $R0

	;Print if A is newly pressed
	CHKKEYNEW $R0, #1, A_Pressed, #1

	;Print if A is newly released
	CHKKEYOLD $R0, #1, A_Released, #1

	;exit if Start is pressed
	CHKKEYNEW $R0, #8, Exit_Script

	VSYNC
;if no input detected, continue loop
JUMP inputLoop

A_Pressed PRINT "A is pressed!"
	OUTC '10 ;newline character
	JMPBK ;return back into the main loop

A_Released PRINT "A has been released!"
	OUTC '10 ;newline character
	JMPBK ;return back into the main loop

;exit the script
Exit_Script HALT
