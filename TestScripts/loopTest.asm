;[CPU_freq 13]
[Console 1]

;Registers
; $t0 = loop counter
; $t1 = target number of loops
; $t2 = counter variable to test loop execution

_start:
	set 		$t1, #5 								;number of loops to test

    ;loop from 0 to 4
    set 		$t0, $0
    set 	    $t2, $0

    loopto 		$0, $t0, $t1, #1
    	incr 		$t2
    	loopback 	$0


    ;test if looping was successful
    ifeq 		$t0, $t2, SUCCESS
    print 		"Loops failed!\n"
    jump 		ENDPRGM



SUCCESS: 		print "Loops Successful\n"
ENDPRGM: 		halt
