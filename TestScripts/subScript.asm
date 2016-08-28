[Console 1]
;[Debug Text]

_start:
	execute 		"subScriptFun2.asm"

	;output returned value from subScriptFun
	;outu 		$v0
	;print 		" bytes.\n"


	ifeq 		$v0, #1337, SUCCESS
	print 		"SubScript Failure\n"
	jump 		ENDPRGM


SUCCESS: 		print 		"SubScript Success\n"
ENDPRGM:		halt
