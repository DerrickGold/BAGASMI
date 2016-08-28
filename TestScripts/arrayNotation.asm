
[Console 1]


_start:

	;first print out string
	print	testString
	outc 	'10


	;now print out "numbers" array contents
	loopto	#0, $t0, #4, #1
		laary 		$t1, numbers, $t0
		out 		($t1)
		outc 		<,
		loopback 	#0


	halt

testString array '25 "Hello, World!"
numbers array #4 { 1, 3, 3, 7 }

