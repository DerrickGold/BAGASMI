#!/bin/bash

echo "****************************************"
echo "Testing basic string output"
echo "****************************************"
bagasmi helloworld.asm

echo "****************************************"
echo "Testing hardware loops"
echo "****************************************"
bagasmi loopTest.asm

echo "****************************************"
echo "Testing jumps and jumpbacks"
echo "****************************************"
bagasmi jumpback.asm

echo "****************************************"
echo "Testing arrays"
echo "****************************************"
bagasmi arrayNew.asm

echo "****************************************"
echo "Testing ARGV"
echo "****************************************"
bagasmi argv.asm "Test String" "1337"

echo "****************************************"
echo "Testing pointers stuff"
echo "****************************************"
bagasmi pointers.asm

echo "****************************************"
echo "Testing Stack"
echo "****************************************"
bagasmi stack.asm

echo "****************************************"
echo "Testing SubScripts"
echo "****************************************"
bagasmi subScript.asm

echo "****************************************"
echo "Testing default array notations"
echo "****************************************"
bagasmi arrayNotation.asm
