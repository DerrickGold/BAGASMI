;[Debug Text]
[Debug Lines]
;[Compile]
[Flip Mode 1]
[CPU_freq 13]
[Console 1]


;SPRITES:
;0 = player
;1 = ball
;2 = ai
;REGISTER USES:
;R0 = game start
;R1 = ball vx
;R2 = ball vy
;R3 = p1 score
;R4 = p2 score
;R5 = key press
;R6 = random calculations
;R7 = serve turn
;R8 = time for AI

;start of the program
_start:

	PRINT "Press Start/Enter to quit,"
	OUTC '10
	PRINT "And A/Z to launch ball."
	OUTC '10
	SET $R7,#1


	;lets load a background here to slot 0
    LDBG #0, "zelda-map"


	;load some graphics and set initial positions
	;player 1 paddle
	LDSPR #0, "paddle.bmp"
	;SETSPRXY #0, P1X, P1Y

	;the ball
	LDSPR #1, "ball.bmp"
	;SETSPRXY #1, BALLX, BALLY

	;AI paddle
	LDSPR #2, "paddle.bmp"
	;SETSPRXY #2, P2X, P2Y

	;draw everything to screen
	JUMP DRAWGFX, #1

	;everything loaded, enter game loop
	GAMELOOP JUMP CHECK_EXIT, #1
		;check if game hasn't started
		IFEQ $R0, #0, RALLYSTART, #1

			;ball has been launched so enter game logic
			;first to check player input for movement
			GETKEY $R5
			CHKKEY $R5, #128, MOVEDOWN
			CHKKEY $R5, #64, MOVEUP

			;jump from movements to here to continue loop
			;update ball movement
			MOVEDONE ADD BALLX, BALLX, $R1
				;now to draw all graphics

					JUMP DRAWGFX, #1

					ADD BALLY, BALLY, $R2
					;now check ball collision
					JUMP UpdateBall

					;done checking screen collisions
					;check paddle collisions
					DONESCRNCOL JUMP PADDLECOL

					;now process AI paddle
					DONEPADCOL JUMP AICODE, #1

	;end of game loop
	JUMP GAMELOOP

;draw gfx to screen
DRAWGFX DRWBG #0, #0

	SETSPRXY #0, P1X, P1Y
	DRWSPR #0, #0

	SETSPRXY #1, BALLX, BALLY
	DRWSPR #0, #1

	SETSPRXY #2, P2X, P2Y
	DRWSPR #0, #2

	FLIP #0
	VSYNC
JMPBK





;player needs to press a to start rally
RALLYSTART GETKEY $R5
	CHKKEY $R5, #1, STARTBALL
	;then jump back to game loop
	JUMP GAMELOOP


;start ball function
STARTBALL SET $R0, #1
	;set random velocity for ball
	RANDM $R1, #4
	ADD $R1, $R1, #1
	MUL $R1, $R1, $R7
	;random value from -4 to 4, make sure its not zero with a loop
	NEWY RANDM $R2, #8
	SUB $R2, $R2, #4
	JMPZ NEWY
JMPBK ;jump back to where RALLYSTART was called

;Move sprite 0 down
MOVEDOWN IFGT P1Y, #160, MOVEDONE ;check to make sure sprite doesn't go off bottom of screen
	;otherwise, keep moving down
	ADD P1Y, P1Y, #3
	JUMP MOVEDONE

;Move sprite 0 up
MOVEUP IFLT P1Y, #0, MOVEDONE ;check to make sure sprite doesn't go off top of screen
	SUB P1Y, P1Y, #3
	JUMP MOVEDONE


;if player 2 scores, add to score and reset ball
P2SCORE PRINT "Player 2 scores!"
	;set player turn for starting
	SET $R7, #1
	OUTC '10
	INCR $R4
	JUMP RESETBALL

P1SCORE PRINT "Player 1 scores!"
	SET $R7, #-1
	OUTC '10
	INCR $R3
	JUMP RESETBALL


;ball x screen collisions
;check x boundries for scoring
UpdateBall IFLTE BALLX, #0, P2SCORE ;check left side, (p2 score)
	IFGTE BALLX, #248, P1SCORE ;check right side, (p1 score)
	IFLTE BALLX, #0, SCRNYREVERSE ;check top of screen
	IFGTE BALLY, #184, SCRNYREVERSE ;check bottom of screen

	DRWSPR #0, #1 ;draw the ball sprite
	JUMP DONESCRNCOL

;flipping the ball velocity from y collision
SCRNYREVERSE SHL $R6, $R2, #1
	SUB $R2, $R2, $R6
	JUMP DONESCRNCOL

;paddle ball collision
PADDLECOL CHKCOL #0, #1
	;check collision between player and ball
	JMPC PADDLE_REVERSE
	CHKCOL #2, #1
	JMPC PADDLE_REVERSE
	JUMP DONEPADCOL

;reverse the ball x velocity on paddle collision
PADDLE_REVERSE SET $R7, #-1
	;need to reverse the x velocity
	MUL $R1, $R1, $R7
	ADD BALLX, BALLX, $R1
	JUMP DONEPADCOL

;fake ai handling
;set the AI y position to the ball position
AICODE SET P2Y, BALLY
	JMPBK

;reset ball function
RESETBALL SET $R0, #0
	SET BALLX, #124
	SET BALLY, #92

	;redraw the screen
	JUMP DRAWGFX, #1
	JUMP GAMELOOP

;check if player wants to leave game
CHECK_EXIT GETKEY $R5
	CHKKEY $R5,#8, ENDGAME

	;if no button pressed, continue game loop
JMPBK


;end of game
ENDGAME HALT

P1Y DATA #80
P1X DATA #8

P2Y DATA #80
P2X DATA #240

BALLY DATA #92
BALLX DATA #124
