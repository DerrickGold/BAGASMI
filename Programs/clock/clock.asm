;[Console 1]
;[Debug Lines]
;[Debug Text]

;$R0 -temp storage for time variables
;$R1 - current hour 1's frame
;$R2 - current hour 10's frame
;$R3 - current minute 1's frame
;$R4 - current minute 10's frame

_start:
	;load graphics first
	jump loadGfx, #1

	;main loop, which updates and draws the clock
	mainLoop:
	 	jump updateTime, #1
		;set clock position to stylus positions
		GETKEY $R15
		SET clockX, $PENX
		SET clockY, $PENY

		JUMP drawGFX, #1

	JUMP mainLoop
HALT

;clear screen to white first
drawGFX:
	SETSCRNCOL clockScreen, #65535
	;update clock number sprite frames
	JUMP updateSprites, #1


	;draw clock bg first
	;center the clock bg x position with the clock gfx
	SUB $R0, clockX, #3
	SUB $R1, clockY, #3
	SETSPRXY #5, $R0, $R1

	DRWSPR clockScreen, #5

	;draw all clock number sprites
	SET $R0, clockX ;store clock position in $R0

	SET $R1, #0
	LOOPTO #0, $R1, #5, #1
		SETSPRXY $R1, $R0, clockY
		DRWSPR clockScreen, $R1
		ADD $R0, $R0, #24
	LOOPBACK #0

	;flip and wait
	FLIP clockScreen
	VSYNC

JMPBK


;set sprites to their frames based on
;the digits stored in the respective registers
updateSprites:

	;hour 10's
	SPRFRAME #0, #2, hours10

	;hour 1's
	SPRFRAME #1, #2, hours1

	;minutes 10's
	SPRFRAME #3, #2, minutes10

	;minutes 1's
	SPRFRAME #4, #2, minutes1
JMPBK


;grab time and store it in "currentTime" array
;then split up the digits of hours and minutes into
;their respective registers
updateTime:
	 GETTIME currentTime

		;grab current hour
		LDARYC $R15, currentTime, #2

		;get hour 1's slot
		REM hours1, $R15, #10

		;get hour 10's slot
		DIV hours10, $R15, #10

		;grab minutes
		LDARYC $R15, currentTime, #1

		;grab minutes 1's slot
		REM minutes1, $R15, #10

		;get minutes 10's slot
		DIV minutes10, $R15, #10
JMPBK


;load all the graphics necessary
loadGfx NOP


	;load the initial number sprites. This sprite will be cloned
	;to save memory for the rest of the sprites that use
	;the same numbers

	;sprite 0, will be the 10's digit of hours
	LDSPR #0, "numbers.bmp"
	SPRFRAMEDIM #0, #24, #36
	SETSPRXY #0, clockX, clockY

	;sprite 1 will be the 1's digit of hours
	CLONESPR #1, #0

	;sprite 2 will be our colon sprite
	LDSPR #2, "colon.bmp"

	;sprite 3 will be the 10's digit of minutes
	CLONESPR #3, #0

	;sprite 4 will be the 1's digit of minutes
	CLONESPR #4, #0


	;load clock bg in sprite 5
	LDSPR #5, "bg.bmp"
	;set bg half transparent
	SETSPRALPHA #5, #128

JMPBK

;make an array of 7 bytes to store the current time
;0 = seconds
;1 = minutes
;2 = hours
;3 = weekday
;4 = day
;5 = month
;6 = year
currentTime ARRAY '8

;clock x and y position
clockX DATA #42
clockY DATA #56

;screen for the clock
clockScreen DATA #1

;time variables
hours10 DATA #0
hours1  DATA #0
minutes10 DATA #0
minutes1 DATA #0
