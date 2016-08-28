[Console 1]
;[Debug Text]


_start:
	;execute 	"subScript.asm"

	call 		getWordSize, $0, $0, $0, $0


	ifeq 		$v0, #4, SUCCESS
	ifeq 		$v0, #8, SUCCESS

	print 		"Error!\n"
	jump		ENDPRGM


SUCCESS:	print 	"Success!\n"
ENDPRGM:	halt


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
