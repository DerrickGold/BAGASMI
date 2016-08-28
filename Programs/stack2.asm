[Console 1]


_start:

	set 		$t0, #48765


	jump 		printSP, #0
	subu 		$sp, $sp, #8
	store 		$t0, ($sp)

	print 		"stored: "
	outu 		($sp)
	outc 		<\n

	jump 		printSP, #0


	pop 		$t1
	outu 		$t1
	outc 		<\n

	jump 		printSP, #0

halt

printSP:
	print 		"Stack Pointer: "
	outu 		$sp
	outc 		<\n
	jmpbk
