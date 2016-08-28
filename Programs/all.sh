#!/bin/bash

echo "****************************************"
echo "Circles.asm"
echo "****************************************"
bagasmi circles2.asm


echo "****************************************"
echo "pixelFixer.asm"
echo "****************************************"
bagasmi pixelFixer.asm

echo "****************************************"
echo "backgrounds.asm"
echo "****************************************"
bagasmi background/backgrounds.asm

echo "****************************************"
echo "pong.asm"
echo "****************************************"
bagasmi pong/pong.asm

echo "****************************************"
echo "clock.asm"
echo "****************************************"
bagasmi clock/clock.asm

