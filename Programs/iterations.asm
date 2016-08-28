[CPU_freq 13]
[Console 1]

;loop 5000 times
_start:
    CLR $R1
    LOOPTO #0,$R1,#5000,#1
    LOOPBACK #0
    OUT $R1
HALT
