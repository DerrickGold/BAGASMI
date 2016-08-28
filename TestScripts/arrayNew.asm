;[Debug Text]
;[Compile]
;[Flip Mode 1]
;[CPU_freq 13]
[Console 1]

;start of the program
_start:
    ;get array address and store a test value at address
	laary      $t0, TESTARY, #3

    set         $t1, #3333
    store       $t1, $t0

    ;load value from address to new register and test if same
    load        $t3, $t0

    ;if same value in t3 and t1, then store and load were successful
    ifeq        $t1, $t3, SHORTTEST
    print       "Error store/load Word Array\n"
    jump        ENDPRGM


SHORTTEST:
    print "Word Array load/store Successful\n"

    set     $t1, #1337
    laaryh  $t0, SHORTARY, #6
    storeh  $t1, $t0


    loadh    $t3, $t0

    ifeq    $t1, $t3, SUCCESS
    print   "Error store/load Short Array\n"
    jump ENDPRGM



SUCCESS:   print "Short Array load/store Successful\n"
ENDPRGM:    halt

TESTARY     ARRAY #20
TESTVAR     DATA #13
SHORTARY    array   *20

