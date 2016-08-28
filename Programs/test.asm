[Debug Text]
[Debug Lines]
;[Compile]
[Flip Mode 1]
[CPU_freq 13]
[Console 1]

;draw a moving 32x32 square across the screen

;R0 = loop counter/x1 of rect
;R1 = x2 of rect
_start:
	;start from 0, end position is 256-32
	LOOPTO #0, $R0, #224, #1

		;clear the screen
		CLRSCRN #0

		;calculate the x2 portion of our rectangle (x1 +_ width)
		ADD $R1, $R0, #32

		;draw a white rectangle from $R0,52 to $R1,84 on bottom screen
		DRWRECT #0, $R0, #52, $R1, #84, #65535

		;update screen
		FLIP #0

		;sync display to 60 fps
		VSYNC
	LOOPBACK #0


HALT
