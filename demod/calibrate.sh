#!/bin/sh

BAND=GSM900
KAL=~/radio/kalibrate-rtl/src/kal

if [ x"$1" = x"" ]
then
	DEVICE=0
else
	DEVICE=$1
fi
#ARFCN=`$KAL -d $DEVICE -s $BAND -e 44 1>&2 |sort -nk7 |tail -1 |cut -f2 -d: |cut -f2 -d ' '`
ARFCN=90
PPM=`$KAL -d $DEVICE -c $ARFCN -e 44 |grep 'average absolute error' |cut -f4 -d' '`
echo $PPM
