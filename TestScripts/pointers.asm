;[Debug Text]
;[Compile]
;[Flip Mode 1]
;[CPU_freq 13]
[Console 1]

;start of the program
_start:
	ldaddr 		$t0, testvar
	set 		($t0), #1337

	ifeq 		($t0), #1337, SUCCESS
	print 		"Pointer/Dereference Failure\n"
	jump 		ENDPRGM

SUCCESS: 		print 		"Pointers/Dereferencing Successful\n"
ENDPRGM: 		halt

testvar data #26
