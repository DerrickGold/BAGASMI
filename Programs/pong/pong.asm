;[Debug Text]
;[Compile]
;[Flip Mode 1]
;[CPU_freq 13]
;[Console 0]
[Debug Lines]


;SPRITES:
;0 = player
;1 = ball
;2 = ai
;REGISTER USES:
;s0 = key press
;s1 = lcd buffer

;t0 = serve turn
;t1 = game start
;t4 = p1 score
;t5 = p2 score

;t2 = ball vx
;t3 = ball vy

;t6 = random calculations



;start of the program
_start:

	print 			"Press Start/Enter to quit,\n"
	print 			"And A/Z to launch ball.\n"
	set 			$t0, #1

	;lets load a background here to slot 0
	ldbg 			$0, "zelda-map"
	set 			$BGX0, #128
	set 			$BGY0, #96

	;load some graphics and set initial positions
	ldspr 			$0, "paddle.bmp"				;player 1 paddle
	ldspr 			#1, "ball.bmp"					;the ball
	ldspr 			#2, "paddle.bmp"				;AI paddle
	jump 			DrawScreen, #1 					;draw everything to screen


;===========================
;Game loop to keep game moving
;===========================
GameLoop:
	ifeq 			$t1, $0, RallyStart, #1				;check if game hasn't started

	;If game started, then check for paddle movement
	getkey 			$s0
	chkkey 			$s0, #128, MovePlayerDown, #1
	chkkey 			$s0, #64, MovePlayerUp, #1

	;update background scrolling
	add				$BGX0, $BGX0, $t2
	add				$BGY0, $BGY0, $t3

	;update ball movement
	add 			BALL_X, BALL_X, $t2 			;ball x update
	jump 			DrawScreen, #1 					;now to draw all graphics
	add 			BALL_Y, BALL_Y, $t3 			;ball y update

	;check ball collisions
	jump 			BallScreenCol, #1 				;ball collision in screen boundaries
	jump 			PaddleCol, #1					;check paddle collisions

	;now process AI paddle
	jump 			AICODE, #1

	;continue game loop
	jump 			GameLoop

;===========================
;draw everything to screen
;===========================
DrawScreen:
	getlcd			$s1, $0
	drwbg 			$s1, $0

	;draw player paddle sprite
	setsprxy 		$0, P1_X, P1_Y
	drwspr 			$s1, #256, #192, $0

	;draw ball sprite
	setsprxy 		#1, BALL_X, BALL_Y
	drwspr 			$s1, #256, #192,  #1

	;draw ai paddle sprite
	setsprxy 		#2, P2_X, P2_Y
	drwspr	 		$s1, #256, #192, #2

	;update screen
	flip 			$0
	vsync
	jmpbk								;exit function

;===========================
;RallyStart
;Checks and starts ralley when A is pressed
;===========================
;player needs to press a to start rally
RallyStart:
	getkey 			$s0
	chkkey 			$s0, #1, startBall
	;continue looping until ralley started
	jump 			RallyStart

;otherwise, start the ball
startBall:
	set 			$t1, #1

	;set random X velocity for ball
	randm 			$t2, #4
	add 			$t2, $t2, #1
	mul 			$t2, $t2, $t0

;make sure the Y velocity is not 0
nonZeroLoop:
	randm 			$t3, #8
	sub 			$t3, $t3, #4

	;if it is zero, keep checking
	ifeq			$t3, $0, nonZeroLoop
	;otherwise, we can get back to game loop
	jmpbk										;exit function call

;===========================
;MovePlayerDown
;Down key pressed, move player (sprite 9)
;===========================
MovePlayerDown:
	ifgt 			P1_Y, #160, downDone 		;check to make sure sprite doesn't go off bottom of screen
	add 			P1_Y, P1_Y, #3				;otherwise, keep moving down
downDone:
	jmpbk										;exit function

;===========================
;MovePlayerUp
;Up key pressed, move player (sprite 9)
;===========================
MovePlayerUp:
	iflt 			P1_Y, $0, upDone 			;check to make sure sprite doesn't go off top of screen
	sub 			P1_Y, P1_Y, #3				;otherwise, keep moving up
upDone:
	jmpbk										;exit function


;===========================
;Player 2 Scores
;===========================
P2Score:
	print 			"Player 2 scores!"
	set 			$t0, #1 					;set serv turn to player
	outc 			'10 						;output newline for instruction text
	incr 			$t5 						;increase AI score
	jump 			ResetBall

;===========================
;Player 1 Scores
;===========================
P1Score:
	print 			"Player 1 scores!"
	set 			$t0, #-1 					;set serv turn to ai
	outc 			'10 						;output newline for instruction text
	incr 			$t4 						;increase score
	jump 			ResetBall

;===========================
;ball x screen collisions
;check x boundries for scoring
;===========================
BallScreenCol:
	iflte 			BALL_X, $0, P2Score 				;check left side, (p2 score)
	ifgte 			BALL_X, #248, P1Score 				;check right side, (p1 score)
	iflte 			BALL_Y, $0, BallYVelReverse, #1 	;check top of screen
	ifgte 			BALL_Y, #184, BallYVelReverse, #1 	;check bottom of screen
	jmpbk

;===========================
;paddle ball collision
;===========================
PaddleCol:
	;check collision between player and ball
	chkcol 			$0, #1
	jmpc 			BallXVelReverse, #1

	;then check collision between ai and ball
	chkcol 			#2, #1
	jmpc 			BallXVelReverse, #1

	jmpbk											;exit function call

;===========================
;reverse the ball x velocity on paddle collision
;	Only handles X velocity
;===========================
BallXVelReverse:
	set 			$t0, #-1
	mul 			$t2, $t2, $t0 					;multiply current x velocity by -1 to get the opposite value
	add 			BALL_X, BALL_X, $t2 			;start the ball off
	jmpbk

;===========================
;reverse the ball Y velocity on paddle collision
;	Only handles Y velocity
;===========================
BallYVelReverse:
	shl 			$t6, $t3, #1
	sub 			$t3, $t3, $t6
	jmpbk

;===========================
;fake ai handling
;set the AI y position to the ball position
;===========================
AICODE:
	set 			P2_Y, BALL_Y
	jmpbk

;===========================
;reset ball function
;===========================
ResetBall:
	set 			$t1, $0							;Set rallystart disabled
	set 			BALL_X, #124					;reset ball position
	set 			BALL_Y, #92
	jump 			DrawScreen, #1 					;redraw screen
	jump 			GameLoop 						;restart game loop


;end of game
halt


;===========================
;All Data
;===========================
;player 1 position
P1_X:			DATA #8
P1_Y: 			DATA #80

;player 2 position
P2_X: 			DATA #240
P2_Y: 			DATA #80

;ball position
BALL_X: 		DATA #124
BALL_Y: 		DATA #92
