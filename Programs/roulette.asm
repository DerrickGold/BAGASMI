[Console 1]
[Debug Lines]

_start:
	;since the game is just starting, we can skip right to the beginning
	JUMP GameStart

	;game over text
	GameOver PRINT "*BANG!* You lost.\n"

	;initialize our revolver, jump with jump back flag initialized
	GameStart JUMP Reload, #0

		;then print out the game instructions
		PRINT "\n~BAGASMI Roulette!~\n"
		PRINT "Press A to shoot. Press B to exit.\n\n"

		;loop to keep game running
		GameLoop NOP

			;store current key presses in register 0
			GETKEY $R0

			;Check if B button (#2) is newly pressed to exit script
			CHKKEYNEW $R0, #2, Exit


			;if A button (#1 value) is newly pressed, check to see if shot is legit
			CHKKEYNEW $R0, #1, CheckChamber, #0

			VSYNC

		;otherwise, continue game loop
		JUMP GameLoop



;exit the script
Exit HALT


;check if chamber was empty or not
CheckChamber NOP
	;if our shot is equal to the chamber that is loaded,
	;then the game resets for another go :D
	IFEQ shotCounter, chamber, GameOver

	;otherwise, increase the shot counter, and try again
	INCR shotCounter

	;let the user know they are safe
	PRINT "*CLICK* Safe!\n"

	JMPBK



;loads a single shot into a random chamber
Reload NOP
	;pick a random chamber from 0 - 6 (6 chambers)
	RANDM chamber, #6

	;reset the shot counter
	SET shotCounter, #0

	JMPBK



;Set up our six shooter here
chamber DATA #0
shotCounter DATA #0
