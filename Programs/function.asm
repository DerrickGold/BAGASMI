[Console 1]


_start:

	print 		"Adding two numbers (3 + 5)...\n"
	call 		addition, #3, #5, $0, $0

	print 		"Result: "
	out 		$v0
	outc		<\n

	print 		"Size of word on this system is: "
	call 		getWordSize, $0, $0, $0, $0
	outu 		$v0
	print 		" bytes.\n"


	halt


;===============================================================================
;Function to add two numbers
;
;	Registers:
;		a0: first number to add
;		a1: second number to add to first
;
;		v0: returned value
;===============================================================================
addition:
	add 		$v0, $a0, $a1
	jumpr 		$ra



;===============================================================================
;Function that returns the size of a word on this system
;
;	Registers:
;		v0: returned word size
;
;===============================================================================
getWordSize:
	set 		$t0, $sp 			;get initial stack position
	push 		$t0					;push a word to stack
	set 		$t1, $sp			;get new stack position
	pop 		$t0					;restore stack frame
	subu 		$v0, $t0, $t1		;get stack position difference in bytes
	jumpr 		$ra 				;exit function call
