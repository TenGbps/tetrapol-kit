#!/bin/sh

# This script is example setup for recording a processing of TETRAPOL.
# This simple setup allows recording of selected tetrapol channel(s).
# Recorded file constains demodulated channel. Decoding is done as
# postprocessing by 'tetrapol_dump' latter. For realtime decoding see
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
OUTPUT_DIR=../tetrapol_data

# start receiver
./demod/tetrapol_rx.py -f ${FREQ} -p ${PPM} -g ${GAIN} -s ${SAMPLE_RATE} -o ${OUTPUT_DIR}/channel%d.bits &
DEMOD_PID=$!
sleep 8

# list top 30 channels with strongest power
./demod/tetrapol_cli_pwr.py | head -n 30

# Following lines are templates for commands, you can uncomment them and set
# paramters for automatic set-up or use them from commandline interactively.

# Enable automatic fine-tuning. Replace N with channel number.
# Control channels should be used as freqency reference.
# ./demod/tetrapol_cli_auto_tune.py N
#
# Wait for stabilisation.
# sleep 6

# Enable recording of channel N (replace N with channel number(s))
#./demod/tetrapol_cli_output_enabled.py open N

# stop recording after specified time
# sleep 600
# kill ${DEMOD_PID}
# sleep 1
# FIXME: This is dirty way to kill receiving when somethings get wrong.
# killall python2.7

# Find and remove all empty recordings.
# find ${OUTPUT_DIR} -size 0 -exec rm \{\} \;

# Decode all channels. (For now it just dups some log, but it will change soon.)
# for f in ${OUTPUT_DIR}/channel*.bits; do echo $f; ./apps/tetrapol_dump -i $f; done | vimpager'

