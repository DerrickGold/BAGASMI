;=======================================================
;	BAGMenu Progress Bar
;		Script
;
;	Written by BassAceGold
;
;Renders a progress bar on screen
;based on the following arguments.
;
;
;ARGV arguments
;	barArgs[0] = screen to render to
;	barArgs[4] = x position to draw bar at
;	barArgs[8] = y position to draw bar at
;	barArgs[12] = width of progress bar
;	barArgs[16] = height of progress bar
;	barArgs[20] = progress bar outline color
;	barArgs[24] = progress bar fill color
;	barArgs[28] = current progress
;	barArgs[32] = total progress
;
;
;The calculation used for generating a percent is:
;	percent = (current progress * 100) / total progress
;
;
;
;=======================================================

;interpreter pragma
;[Script]
[ARGS 9]
[Compile]
[Console 1]
[Debug Text]
[Debug Lines]

;misc regs uses
;$R10 = calculated x2 position
;$R11 = calculated y2 position
;$R15 = final percentage filled
;$R16 = progress bar width
;$R17 = check if boarder color is transparent
;$R18 = storage of the transparent color


;program start
_start:

;first, collecting the values passed
;through argv.

LOOPTO #0, $R0, #9, #1
	GETARGV argStr, $R0
	ATOI $R1, argStr
	STARY $R1, barArgs, $R0
LOOPBACK #0

;then some misc calculations that'll help later on
;add width to x position to get x2 position
ADD $R10, barArgs[4], barArgs[12]

;add height to y position to get y2 position
ADD $R11, barArgs[8], barArgs[16]

;Now to draw progress bar outline
;If the outline color is set to magenta
;we'll consider that as transparent.

;So lets check if the boarder color is transparent
;if it is transparent (, skip boarder drawing

IFEQ barArgs[20], #64543, no_boarder
	;otherwise, continue drawing outline

	;top line
	;screen, x1, y1, x2, y2, color, size
	DRWLINE barArgs[0], barArgs[4], barArgs[8], $R10,barArgs[8], barArgs[20], #1

	;bottom line
	DRWLINE barArgs[0], barArgs[4], $R11, $R10, $R11, barArgs[20], #1

	;left line
	;x2 is the same as x1
	DRWLINE barArgs[0], barArgs[4], barArgs[8], barArgs[4], $R11, barArgs[20], #1

	;right line
	;x1 is the same as x2
	DRWLINE barArgs[0], $R10, barArgs[8], $R10, $R11, barArgs[20], #1

;no boarder here
;now we calculate how much to draw inside the box
;first we need to do a check to make sure we aren't going
;to divide by 0

;check if the current progress is 0
no_boarder IFGT barArgs[28], #0, not_zero
	;if the current progress is 0, set it to 1
	SET barArgs[28], #1

	;now calculate a percentage
	;percent($R15) = (current progress($R7) * 100)/total progress($R8)
	not_zero MUL $R15, barArgs[28], #100
		DIV $R15, $R15, barArgs[32]

		;now using the percentage, we'll determine how much of the
		;progress bar to fill
		;(bar width = percentage * bar width) / 100
		MUL $R16, $R15, barArgs[12]
		DIV $R16, $R16, #100


		;finally we can draw the interior of the progress bar
		;we are going to leave a 1 pixel boarder around the interior
		;so lets calculate the bar position
		;x + 1
		INCR barArgs[4]
		;y + 1
		INCR barArgs[8]
		;x2 = x + width
		ADD $R10, barArgs[4], $R16

		;then draw the rectangle
		;screen, x1, y1, x2, y2, color
		DRWRECT barArgs[0], barArgs[4], barArgs[8], $R10, $R11, barArgs[24]

		;update the screen
		FLIP barArgs[0]

HALT
;have a string 32 characters long
argStr ARRAY #32
;store space for 9 integers + 1 extra
barArgs ARRAY #9
