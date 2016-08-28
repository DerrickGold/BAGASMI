[Console 1]

_start:
    jump        TEST1, #1
    print       ": Jumpbacks Successful!\n"

    halt

TEST1:
    jump        TEST2, #1
    out         #5
    jmpbk

TEST2:
    jump        TEST3, #1
    out         #4
    jmpbk

TEST3:
    jump        TEST4, #1
    out         #3
    jmpbk

TEST4:
    jump        TEST5, #1
    out         #2
    jmpbk

TEST5:
    out         #1
    jmpbk

