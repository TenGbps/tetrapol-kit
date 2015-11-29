#!/bin/sh

if ! [ -f cal ]; then
    echo "Missing file with calibration 'cal'"
    exit -1
fi

for S in 2464 2592 2720 2848
do
	START=$(( $S * 12500 + 358399864 ))
	rtl_power -g 42 -i 1 -f $START:$(( $START + 1600000 )):12500 -1 -w blackman-harris -p `cat cal` |grep $START|cut -f7- -d,|tr , \\n |tr -d ' ' | (ch=$(( ($START - 358399864)/12500 )); while read a; do echo $ch $a; ch=$(( $ch + 1 )); done) 
done |sort -nk2
