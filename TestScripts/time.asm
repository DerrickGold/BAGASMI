[Console 1]
;[Debug Lines]

_start:

	;grab time and store it in "currentTime" array
	timeLoop GETTIME currentTime

		;print out hours
		LDARYC $R0, currentTime, #2
		OUT $R0
		OUTC <:
		;print minutes
		LDARYC $R0, currentTime, #1
		OUT $R0
		OUTC <:
		;print seconds
		LDARYC $R0, currentTime, #0
		OUT $R0
		OUTC <\r

		GETKEY $R1
		CHKKEY $R1, #1, exitPrgm

	;JUMP timeLoop

exitPrgm HALT

;make an array of 7 bytes to store the current time
;0 = seconds
;1 = minutes
;2 = hours
;3 = weekday
;4 = day
;5 = month
;6 = year
currentTime ARRAY '8
