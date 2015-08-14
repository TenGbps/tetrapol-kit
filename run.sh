#!/bin/sh

# This script is example setup for recording a processing of TETRAPOL.
# This simple setup allows recording of selected tetrapol channel(s).
# Recorded .bin file constain demodulated channel data. Decoding is done
# as postprocessing by 'tetrapol_dump' latter. For realtime decoding see
# paragraph bellow.

# Tips for realtime processing:
# For realtime processing you can create pipes with propper channels names
# and use them as input for tetrapol_dump. (Produced demodulated channel
# data are writen into this pipes instead of regular files.)

# Settings
FREQ=393e6
PPM=0
GAIN=40
SAMPLE_RATE=2000000
OUTPUT_DIR=../tetrapol_rec

if [ ! -d ${OUTPUT_DIR} ]; then
    echo "'${OUTPUT_DIR}' is not a directory, create it first"
    exit 1
fi

# start receiver
./demod/tetrapol_rx.py -f ${FREQ} -p ${PPM} -g ${GAIN} -s ${SAMPLE_RATE} -o ${OUTPUT_DIR}/channel%d.bits &
DEMOD_PID=$!
sleep 2

print_help()
{
	echo "a <CHANNEL>    - automatic fine tunning to channel C"
	echo "d <CHANNEL>    - disable demodulation for channel C"
	echo "e <CHANNEL>    - enable demodulation for channel C"
	echo "h              - print help"
	echo "l              - list top 30 channels by PWR"
	echo "x              - exit"
}

print_help

while true; do
	read -p "Command: " CMD ARG1
	case "$CMD" in
		"a")
			./demod/tetrapol_cli.py autotune $ARG1
		;;

		"d")
			./demod/tetrapol_cli.py output close $ARG1
		;;

		"e")
			./demod/tetrapol_cli.py output open $ARG1
		;;


		"h")
			print_help
		;;

		"l")
			# list top 30 channels with strongest power
			./demod/tetrapol_cli.py power | head -n ${ARG1:-30}
		;;

		"x")
			kill ${DEMOD_PID}
			break
		;;

		*)
			echo "Invalid command $CMD"
		;;
	esac
done

# FIXME: This is dirty way to kill receiving when somethings get wrong.
# killall python2.7

# Find and remove all empty recordings.
find ${OUTPUT_DIR} -size 0 -exec rm \{\} \;

# Decode all channels. (For now it just dups some log, but it will change soon.)
for f in ${OUTPUT_DIR}/channel*.bits; do
    echo "Processing: $f"
    ./build/apps/tetrapol_dump -i "$f" >"${f%.bits}.log"
done

