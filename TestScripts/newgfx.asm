[Console 1]
;[Debug Lines]

_start:
	;first lets store a color
	rgb 		$s0, #31, #31, #31					;color red
	blankspr 	#0, #256, #192


	jump 		DrawTest

	getsprgfx 	$t0, #0
	print 		"in sprite buf: "
	out 		$t0
	outc		'10
	gfxdrwline	$t0, #256, #192, #0, #0, #255, #191, $s0, #1

	getlcd 		$t0, #0
	drwspr		$t0, #256, #192, #0
	flip 		#0

	jump _testLoop







halt


_testLoop:
	vsync
	jump _testLoop







FillTest:
	;fill test
	getlcd 		$t0, $0
	print 		"lcd base addr: "
	out 		$t0
	outc 		'10


	gfxmemset	$t0, $s0, #49152
	flip 		$0

_waitLoop:
	vsync
	jump _waitLoop

halt





DrawTest:
	;first check if touch screen is pressed
	getkey 	$t0

	chkkey 	$t0, #4096, ScreenTouched, #1

	flip $0
	vsync
	jump 	DrawTest



ScreenTouched:
	print 	"Drawing!\n"
	;lets test getting the screen buffers :D
	;get screen 0 lcd address
	;getlcd 	$t0, $0




	;print	"lcd base: "
	;out 	$t0
	;outc 	'10

	;calculate pixel position to draw
	;mul 	$t1, $PENY, #256
	;add 	$t1, $t1, $PENX
	;mul 	$t1, $t1, #2

	;add 	$t0, $t0, $t1
	;laaryh 	$t0, $t0, $t1


	;write color to screen
	;print 	"Storing color: "
	;outhu	$s0
	;outc 	'10
	;storeh 	$s0, ($t0)

	getsprgfx 	$t4, $0
	gfxsetpix 	$t4, #256, $PENX, $PENY, $s0

	gfxgetpix 	$t5, $t4, #256, $PENX, $PENY
	print 		"screen pix: "
	out 		$t5
	outc 		'10

	getlcd 		$t0, $0

	drwspr 		$t0, #256, #192, $0
	;drwspr 		$0, $0
	jmpbk




exitPrgm HALT
