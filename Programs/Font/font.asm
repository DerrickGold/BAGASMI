;[Debug Text]
[Debug Lines]
;[Compile]
;[CPU_freq 13]

;tbox init
;	tbox number (0 - 3)
;	x1 position
;	y1 position
;	x2 position
;   y2 position
;	alignment (0 = LEFT, 1 = RIGHT, 2 = X centered, 3 = Y centered, 4 = centered)
;	wrapping (0 = no wrapping, 1 = letter wrapping, 2 = word wrapping)
;   character Count ( < 0 = all characters in string, 0 = no characters in string, X | X > 0 X = characters in string


_start:


	PRINT "Loading font file\n"
	LDFNT #0, "arial_uni.fnt"

	;change the font color
	RGB $R0, #31, #0, #0
	FNTSETCOL #0, $R0

	JUMP loop_printing

	PRINT "Initializing textbox\n"
	;initialize a textbox to print in
	TBOXINIT #0, #0, #0, #256, #192, #2, #2, #-1

	PRINT "Printing!\n"
	;screen, font slot, txtbox slot, msg
	FNTPRINT #0, #0, #0, "Hello World!"

	FLIP #0
	VSYNC

HALT

loop_printing NOP
	;fill the string
	STRNCPY myString, "Brynne, you are the most awesomist awesome person ever!", #256

	SET $R0, #0
	;get length of the string
	STRLEN $R1, myString

	;loop from $R0 to $R1 (the length of the string)
	;and we will print out each letter
	LOOPTO #0, $R0, $R1, #1
		;clear screen
		CLRSCRN #0, #0

		;initialize text box, but only print out $R0 number of characters
		TBOXINIT #0, #0, #0, #256, #192, #2, #2, $R0

		;print to bottom screen
		FNTPRINT #0, #0, #0, myString

		;update screen
		FLIP #0
		VSYNC

	LOOPBACK #0


HALT

myString ARRAY '256
