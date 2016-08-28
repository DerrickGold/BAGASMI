;[Debug Tex]
;[Compile]
;[CPU_freq 13]
[Console 1]
[Debug Lines]


_start:

	;load sprites and initialize their velocity
	;sub 	MAX_VELOCITY, MAX_VELOCITY, #1
	decr 	MAX_VELOCITY

	;create a graphic object for the screen lines
	blankspr	$0, SCREEN_WIDTH, SCREEN_HEIGHT

	set 	$t0, #1
	loopto 		$0, $t0, BALL_COUNT, #1
		ldspr 	$t0, "ball.bmp"

		;generate random x velocity
		laary 	$t1, BALL_XVEL, $t0
		randm 	($t1), MAX_VELOCITY
		incr 	($t1)							;make sure velocity isn't 0

		;then increment the ball by the x velocity
		laary	$t2, BALL_XPOS, $t0
		add 	($t2), ($t2), ($t1)


		;generate random y velocity
		laary 	$t1, BALL_YVEL, $t0
		randm 	($t1), MAX_VELOCITY
		incr 	($t1)

		;increment ball y position by Y velocity
		laary 	$t2, BALL_YPOS, $t0
		add 	($t2), ($t2), ($t1)

		loopback 	#0


;===============================================================================
;GameLoop
;	Where the magic all happens
;
;	Registers:
;		s0: current ball number to update
;===============================================================================

GameLoop:
	getlcd 		$s1, $0
	gfxmemset 	$s1, 0, #49152 									;clear screen


	call 	Mouse_SwipeDirection, $0, $0, $0, $0
	call	LineDrawAttempt, $v0, $0, $0, $0

	;draw bg lines
	drwspr 	$s1, SCREEN_WIDTH, SCREEN_HEIGHT, $0

	set 	$s0, #1
	loopto 	#0, $s0, BALL_COUNT, #1

		call 	Ball_ScreenBoundaries, $s0, $0, $0, $0
		call 	Ball_Update, $s0, $0, $0, $0
		laary 	$t0, BALL_XPOS, $s0
		laary 	$t1, BALL_YPOS, $s0

		;draw the sprite
		setsprxy 	$s0, ($t0), ($t1)
		drwspr		$s1, SCREEN_WIDTH, SCREEN_HEIGHT, $s0

		loopback 	#0

	flip 	$0
	vsync
	jump 	GameLoop




LineDrawAttempt:
	push 	$ra

	;first check if the line is already being drawn and needs to be updated
	ifeq 	LINE_DIR, #0, _updateHorLine
	ifeq 	LINE_DIR, #1, _updateVerLine

	;otherwise, check for new swipes
	ifgt 	$a0, #-1, _initLine
	pop 	$ra
	jumpr 	$ra

_initLine:
	;initialize values for drawing line
	set 	LINE_DIR, $a0
	set 	LINE_X1, MOUSE_OLDX
	set 	LINE_X2, MOUSE_OLDX
	set 	LINE_Y1, MOUSE_OLDY
	set 	LINE_Y2, MOUSE_OLDY

	;set line thickness
	sub 	LINE_Y1, LINE_Y1, #2
	add 	LINE_Y2, LINE_Y2, #2
	sub 	LINE_X1, LINE_X1, #2
	add 	LINE_X2, LINE_X2, #2

	jumpr 	$ra

_updateHorLine:
	;print 	"update hor line\n"
	call 	_UpdateLineDrawHor, MOUSE_CURX, MOUSE_CURY, $0, $0
	pop 	$ra
	jumpr 	$ra

_updateVerLine:
	;print 	"update ver line\n"
	call 	_UpdateLineDrawVert, MOUSE_CURX, MOUSE_CURY, $0, $0

	pop 	$ra
	jumpr 	$ra


;===============================================================================
;UpdateLineDraw_Hor
;	Updates horizontal line drawing
;
;	Registers:
;		$a0: starting x coordinate for line
;		$a1: starting y coordinate for line
;
;		$t0: when set to 1, signifies the left side of the line is done drawing
;		$t1: when set to 1, signifies the right side of the line is done drawing
;		$t2: pointer to drawing surface
;		$t3: hold pixel color for line range checking
;===============================================================================
_UpdateLineDrawHor:
	set 		$t0, #0
	set 		$t1, #0
	getsprgfx 	$t2, BG_SURFACE_SPRITE



	;first check left side collisions with screen and other lines
	set 		$t4, LINE_X1
	decr 		$t4
	iflte 		$t4, #0, __lineSkipLeft
	gfxgetpix	$t3, $t2, SCREEN_WIDTH, $t4, $a1
	ifeq 		$t3, LINE_COL, __lineSkipLeft

	sub 		LINE_X1, LINE_X1, #2
 	;no left side collision
	jump 		__doRightSide

__lineSkipLeft:
	set 		$t0, #1

	;now check right stuff
__doRightSide:
	set 		$t4, LINE_X2
	incr 		$t4
	ifgte 		$t4, #255, __lineSkipRight
	gfxgetpix 	$t3, $t2, SCREEN_WIDTH, $t4, $a1
	ifeq		$t3, LINE_COL, __lineSkipRight

	;no right side collision, update line
	add 		LINE_X2, LINE_X2, #2
	;now check if line is drawn
	jump 		_isLineDone

__lineSkipRight:
	set 		$t1, #1
	jump 		_isLineDone


;============================

_UpdateLineDrawVert:
	set 		$t0, #0
	set 		$t1, #0
	getsprgfx 	$t2, BG_SURFACE_SPRITE

	;first check top side collisions with screens and other lines
	set 		$t4, LINE_Y1
	decr 		$t4

	iflte 		$t4, #0, __lineSkipTop
	gfxgetpix 	$t3, $t2, SCREEN_WIDTH, $a0, $t4
	ifeq 		$t3, LINE_COL, __lineSkipTop

	sub 		LINE_Y1, LINE_Y1, #2
	;no top collisions
	jump 		__doBotSide

__lineSkipTop:
	set 	$t0, #1

__doBotSide:
	set 		$t4, LINE_Y2
	incr 		$t4

	ifgte 		$t4, SCREEN_HEIGHT, __lineSkipBot
	gfxgetpix	$t3, $t2, SCREEN_WIDTH, $a0, $t4
	ifeq 		$t3, LINE_COL, __lineSkipBot

	;no bottom collisions
	add 		LINE_Y2, LINE_Y2, #2
	jump 		_isLineDone


__lineSkipBot:
	set 	$t1, #1


_isLineDone:
	and 		$t0, $t0, $t1
	ifeq 		$t0, $0, _DrawLine

	;done line, so fill in space
	jump 		_lineFillEmpty, #1
	set 		LINE_DIR, #-1


_DrawLine:
	getsprgfx 		$t0, BG_SURFACE_SPRITE

	ifeq 			$v0, #999, __clearLine

	gfxdrwrec 	$t0, SCREEN_WIDTH, SCREEN_HEIGHT, LINE_X1, LINE_Y1, LINE_X2, LINE_Y2, LINE_COL
	jumpr 	$ra 					;still uses ra position from LineDrawAttempt

__clearLine:
	gfxdrwrec 	$t0, SCREEN_WIDTH, SCREEN_HEIGHT, LINE_X1, LINE_Y1, LINE_X2, LINE_Y2, #0
	jumpr 	$ra 					;still uses ra position from LineDrawAttempt


;===================================
;_lineFillEmpty
;	Fills in the empty space after a line was successfully created
;
;	$a0: should have line start x pos
;	$a1: should have line start y pos
;
;	Registers:
;		$t0: axis to compare ball positions
;		$t1: address of axis position array
;		$t2: loop counter (aka sprite to check)
;		$t3: left/top side ball count
;		$t4: right/bottom side ball count
;		$t5: temp position to compare to
;		$t6: start range positional comparment
;		$t7: end range positional comparement
;		$t8: address of other axis range compare

;
; TODO: Need to fix the check empty so that it includes the free space width
;to see if other balls fit within the range
;
;

_lineFillEmpty:
	;determine which axis we are filling on
	ifeq 	LINE_DIR, #0, __lineFillHor
	ifeq 	LINE_DIR, #1, __lineFillVert

	print 	"no axis, wtf\n"
	jmpbk

;check above and below the line for horizontal filling
__lineFillHor:
	print	"horizontal fill\n"
	set 	$t0, LINE_Y1
	ldaddr 	$t1, BALL_YPOS

	;range to check if ball is below this current line
	set 	$t6, LINE_X1
	set 	$t7, LINE_X2
	ldaddr	$t8, BALL_XPOS

	jump 	___lineCheckEmpty

;otherwise, check left and right of the line for vertical filling
__lineFillVert:
	print	"vertical fill\n"
	set 	$t0, LINE_X1
	ldaddr 	$t1, BALL_XPOS

	;range to check if ball is next to line
	set 	$t6, LINE_Y1
	set 	$t7, LINE_Y2
	ldaddr 	$t8, BALL_YPOS

;now to check if any sides of the line are empty
___lineCheckEmpty:
	set 	$t3, $0
	set 	$t4, $0
	;loop 0 handles main game logic
	set 	$t2, #1
	loopto 		#1, $t2, BALL_COUNT, #1
		;first check if the ball is in range of the line that was drawn,
		;otherwise, it is in a different section and can be skipped.
		laary 		$t5, ($t8), $t2
		;less than bottom range
		iflte		($t5), $t6, _____linenextLoop
		;greater than top range
		ifgte 		($t5), $t7, _____linenextLoop

		;otherwise, ball is in range, so tally up what side its on

		laary 		$t5, ($t1), $t2
		iflte 		($t5), $t0, ____incrleft
		jump 		____incrRight
____incrleft:
		print		"incr top/left\n"
		incr 		$t3
		jump 		_____linenextLoop
____incrRight:
		print 		"incr bot/right\n"
		incr 		$t4


_____linenextLoop	loopback	#1

;=========================
;once loop is done figure out which side has no balls
;flipped cause you fill the opposite direction
	ifeq 	$t3, $0, ___fillLeft
	ifeq 	$t4, $0, ___fillRight

	print 	"no empty sides\n"
	jmpbk

___fillLeft:
	print 	"filling left/top!\n"
	getsprgfx 		$t0, BG_SURFACE_SPRITE

	;figure out what side of the line to
	ifeq 	LINE_DIR, #0, _____fillTop

;Fill in Left Screen Area
	;calculate the width of area to fill
	sub 			LINE_X1, LINE_X1, #2
______widthLoopLeft:
	iflt 			LINE_X1, $0, ______widthLoopLeft0
	gfxgetpix 		$t1, $t0, SCREEN_WIDTH, LINE_X1, $a1
	ifeq 			$t1, LINE_COL, ______widthLoopLeftExit
	sub 			LINE_X1, LINE_X1, #1
	jump  			______widthLoopLeft

______widthLoopLeft0: set 	LINE_X1, $0
______widthLoopLeftExit:
	print 		"filling left\n"
	gfxdrwrec	$t0, SCREEN_WIDTH, SCREEN_HEIGHT, LINE_X1, LINE_Y1, LINE_X2, LINE_Y2, LINE_COL
	jmpbk


_____fillTop:
	;calculate the width of area to fill
	sub 			LINE_Y1, LINE_Y1, #2
______widthLoopTop:
	iflt			LINE_Y1, $0, ______widthLoopTop0
	gfxgetpix 		$t1, $t0, SCREEN_WIDTH, $a0, LINE_Y1
	ifeq 			$t1, LINE_COL, ______widthLoopTopExit
	sub 			LINE_Y1, LINE_Y1, #1
	jump  			______widthLoopTop

______widthLoopTop0: set 	LINE_Y1, $0
______widthLoopTopExit:
	gfxdrwrec	$t0, SCREEN_WIDTH, SCREEN_HEIGHT, LINE_X1, LINE_Y1, LINE_X2, LINE_Y2, LINE_COL
	jmpbk




___fillRight:
	print 	"filling right/bottom!\n"
	getsprgfx 		$t0, BG_SURFACE_SPRITE
	;figure out which side of the line to fill
	ifeq 	LINE_DIR, #0, _____fillBot


;fill right side of screen
	;calculate the width of area to fill
	add 			LINE_X2, LINE_X2, #2
______widthLoopRight:
	ifgt			LINE_X2, SCREEN_WIDTH, ______widthLoopRight0
	gfxgetpix 		$t1, $t0, SCREEN_WIDTH, LINE_X2, $a1
	ifeq 			$t1, LINE_COL, ______widthLoopRightExit
	add 			LINE_X2, LINE_X2, #1
	jump  			______widthLoopRight

______widthLoopRight0: set 	LINE_X2, SCREEN_WIDTH
______widthLoopRightExit:
	gfxdrwrec	$t0, SCREEN_WIDTH, SCREEN_HEIGHT, LINE_X1, LINE_Y1, LINE_X2, LINE_Y2, LINE_COL
	jmpbk


;fill bottom side of screen
_____fillBot:
	;calculate the width of area to fill
	add 			LINE_Y2, LINE_Y2, #2
______widthLoopRBot:

	ifgt			LINE_Y2, SCREEN_HEIGHT, ______widthLoopBot0
	gfxgetpix 		$t1, $t0, SCREEN_WIDTH, $a0, LINE_Y2
	ifeq 			$t1, LINE_COL, ______widthLoopBotExit
	add 			LINE_Y2, LINE_Y2, #1
	jump  			______widthLoopRBot

______widthLoopBot0: set 	LINE_Y2, #191
______widthLoopBotExit:
	gfxdrwrec	$t0, SCREEN_WIDTH, SCREEN_HEIGHT, LINE_X1, LINE_Y1, LINE_X2, LINE_Y2, LINE_COL
	jmpbk



;===============================================================================
;Update Ball
;	Updates the balls position based on its velocity, and checks for ball collision
;
;	Registers:
;		$a0:		ball number to update
;
;		$t0:		current X position
;		$t1:		curreny y position
; 		$t2: 		current X and Y Velocity
;		$t3: 		sprite buffer
;		$t4:		bg pixel
;		$t5: 		temp position for math
; 		$t6: 		second temp position for math
;===============================================================================
Ball_Update:
	;grab ball coordinates
	laary 		$t0, BALL_XPOS, $a0
	laary 		$t1, BALL_YPOS, $a0

	;check any wall collisions
	getsprgfx	$t3, BG_SURFACE_SPRITE

	;X axis checks
	;
	;first check left side x-1, y
	laary 		$t2, BALL_XVEL, $a0
	set 		$t5, ($t0)
	decr 		$t5

	;get middle of ball position
	set 		$t6, BALL_HEIGHT
	shr 		$t6, $t6, #1
	add 		$t6, $t6, ($t1)

	gfxgetpix 	$t4, $t3, SCREEN_WIDTH, $t5, $t6
	ifeq 		$t4, LINE_COL, ____posVelocity, #1

	;next check right side x + ball width + 1, y + ball width/2
	add 		$t5, $t5, #2
	add 		$t5, $t5, BALL_WIDTH

	gfxgetpix 	$t4, $t3, SCREEN_WIDTH, $t5, $t6
	ifeq 		$t4, LINE_COL, ____negVelocity, #1

	;update x position
	add 	($t0), ($t0), ($t2)

	;YAxis checks
	;
	;next check top side x, y - 1
	laary 		$t2, BALL_YVEL, $a0
	set 		$t5, ($t1)
	decr 		$t5

	;get middle of ball position
	set 		$t6, BALL_WIDTH
	shr 		$t6, $t6, #1
	add 		$t6, $t6, ($t0)

	gfxgetpix 	$t4, $t3, SCREEN_WIDTH, $t6, $t5
	ifeq 		$t4, LINE_COL, ____posVelocity, #1

	;next check bottom x, y + ball height + 1
	;add 		$t5, $t5, #1
	add 		$t5, $t5, BALL_HEIGHT

	gfxgetpix 	$t4, $t3, SCREEN_WIDTH, $t6, $t5
	ifeq 		$t4, LINE_COL, ____negVelocity, #1

	;update y position of current ball
	add 	($t1), ($t1), ($t2)


	jumpr 	$ra 						;exit function call

____negVelocity:
	abs		$t7, ($t2)
	mul		($t2), $t7, #-1
	jmpbk

____posVelocity:
	abs 	$t7, ($t2)
	set 	($t2), $t7
	jmpbk

;===============================================================================
;Ball_ScreenBoundaries
;	Checks ball collisions with the screen boundaries
;
;	Registers:
;		$a0: 		current ball to check
;
;		$t0:  		position address in x or y array
;		$t1: 		position in pixels for x or y axis
;		$t2: 		address for flipping velocity
;===============================================================================
Ball_ScreenBoundaries:
	push 	$ra 									;preserve return address

;first check on the x axis
	laary	$t0, BALL_XPOS, $a0
	set 	$t1, ($t0) 								;ensure we aren't modifying the array value

	iflte 	$t1, #0, _checkLeft, #1 				;check left side
	add 	$t1, $t1, BALL_WIDTH
	ifgte 	$t1, SCREEN_WIDTH, _checkRight, #1 		;check right side

;then check on the y axis
	laary	$t0, BALL_YPOS, $a0
	set 	$t1, ($t0)

	iflte 	$t1, #0, _checkTop, #1 				;check top side
	add 	$t1, $t1, BALL_HEIGHT
	ifgte 	$t1, SCREEN_HEIGHT, _checkBot, #1 	;check bottom of screen

;exit function
	pop 	$ra
	jumpr 	$ra


_checkRight:
	;make sure the x position is within screen boundaries
	set 	($t0), SCREEN_WIDTH
	decr 	($t0) 						;decress position by one to allow for pixel collision detection
	sub		($t0), ($t0), BALL_WIDTH
	jump 	___xflip

_checkLeft:
	set 	($t0), $0
	incr 	($t0)						;increase position by one to allow for pixel collision detection

___xflip:
	laary 	$t2, BALL_XVEL, $a0
	jump _flipVel

_checkBot:
	set 	($t0), SCREEN_HEIGHT
	decr 	($t0)
	sub 	($t0), ($t0), BALL_HEIGHT
	jump 	___yflip

_checkTop:
	set 	($t0), $0
	incr 	($t0)

___yflip:	laary 	$t2, BALL_YVEL, $a0


_flipVel:
	mul 	($t2), ($t2), #-1
	jmpbk


;===============================================================================
;Mouse_SwipeDirection
;	Get the axis in which the mouse was swiped
;
;	Registers:
;		$t0: current keys state
;
;	Returns:
;		-1: for no axis swiped
;		0: for x axis
;		1: for y axis
;===============================================================================
Mouse_SwipeDirection:
	set 	$v0, #-1 								;currently, nothing touched

	getkey 	$t0

	;if new screen touch, store the old position to be compared
	chkkeynew 	$t0, INPUT_SCREEN, _storeOldMouse, #1

	;if screen touch released, then we do the math to figure out which axis
	chkkeyold 	$t0, INPUT_SCREEN, _getSwipeDir, #1

	chkkey 		$t0, INPUT_SCREEN, __updateMouse, #1

	jumpr	$ra 									;no swipe, exit function

__updateMouse:
	set 		MOUSE_CURX, $PENX
	set 		MOUSE_CURY, $PENY
	jmpbk


_getSwipeDir:
	;make sure mouse position is even
	and 	$t0, MOUSE_OLDX, #1
	ifgt 	$t0, $0, __mouse_evenx, #1

	and 	$t0, MOUSE_OLDY, #1
	ifgt 	$t0, $0, __mouse_eveny

	jump 	__calcSwipDir
__mouse_evenx:
	print 	"even x\n"
	sub 	MOUSE_OLDX, MOUSE_OLDX, #1
	jmpbk

__mouse_eveny:
	print 	"even y\n"
	sub 	MOUSE_OLDY, MOUSE_OLDY, #1

__calcSwipDir:
	print 	"New Mouse: "
	out 	MOUSE_CURX
	outc 	<,
	out 	MOUSE_CURY
	outc 	'10

	;first get differences in positions
	sub 	$t0, MOUSE_CURX, MOUSE_OLDX
	abs		$t0, $t0

	sub 	$t1, MOUSE_CURY, MOUSE_OLDY
	abs 	$t1, $t1


	;now determine which axis had the biggest difference
	ifgt 	$t0, $t1, _returnXAxis
	set 	$v0, #1 								;vertical swipe, return 1
	jmpbk

_returnXAxis:
	set 	$v0, #0 								;horizontal swipe, return 0

_storeOldMouse:
	store 	$PENX, MOUSE_OLDX
	store 	$PENY, MOUSE_OLDY

	print 	"Old Mouse: "
	out 	MOUSE_OLDX
	outc 	<,
	out 	MOUSE_OLDY
	outc 	'10

	jmpbk






ENDPRG: 	halt


;===============================================================================
;Data Declarations
;===============================================================================

SCREEN_WIDTH 	data 	#256
SCREEN_HEIGHT 	data 	#192


MAX_VELOCITY 	data	#5			;max velocity of 5 pixels per frame

;Ball Data
BALL_COUNT 		data 	#3

BALL_XVEL 		array   #10
BALL_YVEL 		array 	#10

BALL_XPOS 		array 	#10 	{0,0,0,0,0,0,0,0,0,0}
BALL_YPOS 		array 	#10 	{0,0,0,0,0,0,0,0,0,0}

BALL_WIDTH 		data 	#8
BALL_HEIGHT 	data 	#8


MOUSE_OLDX 		data 	#0
MOUSE_OLDY 		data 	#0
MOUSE_CURX 		data 	#0
MOUSE_CURY 		data 	#0

;Line drawing data
LINE_X1 		data 	#0
LINE_Y1 		data 	#0
LINE_X2 		data 	#0
LINE_Y2 		data 	#0
LINE_DIR 		data 	#-1
LINE_COL		data 	#0x8f00 					;hopefully red


;hold ID for the bg surface sprite
BG_SURFACE_SPRITE 	data 	#0

;===============================================================================
;Button Values
;===============================================================================
INPUT_SCREEN 	data 	#4096

