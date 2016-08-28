;[Debug Text]
[Console 1]
_start:
	set 		$t0, #3333
	push 		$t0

	pop 		$t1

	ifeq 		$t1, $t0, SUCCESS

	print 		"Push/Pop Failed\n"
	jump 		EXITPRGM




SUCCESS: 		print "Push/Pop Success\n"
EXITPRGM: 		halt



printSP:
	print 		"Stack Pointer: "
	outu 		$sp
	outc 		<\n
	jmpbk

