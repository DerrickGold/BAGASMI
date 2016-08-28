;[Debug Text]
;[Debug Lines]
[ARGS 2]
[Console 1]


_start:
	;print out first string
	getargv 		firstString, #0
	print 			firstString
	outc 			<\n

	;print out second number

	getargv 		secondString, #1
	atoi 			$t0, secondString
	out 			$t0
	outc 			<\n


	halt

firstString ARRAY #30
secondString ARRAY #16

