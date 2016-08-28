[Console 1]

_start:
	print 	"pushing to stack!\n"
	loopto 	$0, $t0, #1024, #1
		push 	$t0
		loopback #0


	print 	"popping from stack\n"
	set 	$t0, $0
	loopto 	$0, $t0, #1024, #1
		pop 	$t1
		print 	"popped: "
		out 	$t1
		outc 	'10
		loopback 	$0


	halt
