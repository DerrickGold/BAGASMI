[Console 1]
;[Debug Lines]

;Registers
; $s0 - key state
; $s1 - bg scroll axis
; $s2 - bg scroll distances
; $s3 - screen to draw on


_start:
    ;draw bg on top screen!
    set     $s3, TOP_SCREEN

    print   "Loading background!"
    outc    '10

    ;lets load a background here to slot 0
    ldbg    #0, "zelda-map"

    print   "Background loaded!"
    outc    '10

    call    DrawScene, $s3, $0, $0, $0


;now lets make a loop to scroll it around
RenderLoop:
    getkey  $s0


    ;now to check direction keys to see where to scroll bg
    ;check left key
    chkkey  $s0, #32
    jmpp    ScrollLeft, #1

    ;check right key
    chkkey  $s0, #16
    jmpp    ScrollRight, #1

    ;check up key
    chkkey  $s0, #64
    jmpp    ScrollUp, #1

    ;check down key
    chkkey  $s0, #128
    jmpp    ScrollDown, #1

    jump    RenderLoop

    halt


;===================================
; Scene scrolling
;===================================
;first set which axis we want to scroll on (0 = x, 1 = y)
ScrollLeft:
    set     $s1,#0
    ;loop iterations (Screen Width / SCROLL_SPEED)
    set     $s2,#128

    ;flip the scroll speed to negative
    abs     SCROLL_SPEED, SCROLL_SPEED
    mul     SCROLL_SPEED, SCROLL_SPEED, #-1

    ;now do the scrolling
    jump    ScrollScene, #1
    jmpbk


ScrollRight:
    set     $s1, #0
    set     $s2, #128

    ;set scroll speed to positive
    abs     SCROLL_SPEED, SCROLL_SPEED

    jump    ScrollScene, #1
    jmpbk


ScrollUp:
    set     $s1,#1
    set     $s2,#96

    ;flip scroll speed
    abs     SCROLL_SPEED, SCROLL_SPEED
    mul     SCROLL_SPEED, SCROLL_SPEED, #-1
    jump    ScrollScene, #1
    jmpbk


ScrollDown:
    set     $s1,#1
    set     $s2,#96

    ;set scroll speed to positive
    abs     SCROLL_SPEED, SCROLL_SPEED

    jump    ScrollScene, #1
    jmpbk


ScrollScene:
    set     $t0, #0
loopto  #0, $t0, $s2, #1

    ;check which axis we are scrolling on
    ifgt    $s1, #0, _scrollSceneY

    ;scroll x direction
    add     $BGX0, $BGX0, SCROLL_SPEED
    jump    _scrollSceneDraw

    ;scroll y direction
    _scrollSceneY:  add     $BGY0, $BGY0, SCROLL_SPEED

    ;update scene
    _scrollSceneDraw: call DrawScene, $s3, $0, $0, $0

    loopback    #0
    jmpbk


;===================================
; Scene scrolling
;
;   $a0: screen to draw background on
;===================================
DrawScene:
    drwbg   $a0, #0
    flip    $a0
    vsync
    jumpr $ra

;declare an integer variable
SCROLL_SPEED DATA #2

TOP_SCREEN DATA #1
BOT_SCREEN DATA #0
