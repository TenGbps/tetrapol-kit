#!/bin/sh

FREQ=393e6
GAIN=30.4
MAX_CHANNELS=8
OUT_DIR=tmp
SAMP_RATE=2400000
# set lenght of record in seconds, use 0 for infinity
TIMEOUT=30

CH_BW=12500
CHANNELS_FILE=channels.json


if ! [ -d "${OUT_DIR}" ]; then
    echo "OUT_DIR (${OUT_DIR}) does not exist"
    exit 1
fi

echo "Detecting TETRAPOL channels"
python2 tetrapol_detector.py \
    -f ${FREQ} \
    -g ${GAIN} \
    -i osmo-sdr:// \
    -o "${OUT_DIR}/${CHANNELS_FILE}"  \
    -r 1 \
    -s ${SAMP_RATE}

FREQS=`grep freq tmp/channels.json  | sed -e 's/.*freq": //' -e 's/\\..*//' | head -n ${MAX_CHANNELS} | tr \\\\012 ,`

echo "Receiving selected TETRAPOL channels"
timeout ${TIMEOUT} \
    python2 demod.py \
        -g ${GAIN} \
        -s ${SAMP_RATE} \
        -o ${OUT_DIR}/channel%%.bits \
        -l "${FREQS}"

for f in `echo "${FREQS}" | tr , ' '`; do
    ../build/apps/tetrapol_dump -t CCH -i ${OUT_DIR}/channel${f}.bits \
            >${OUT_DIR}/cch_${f}.json 2>${OUT_DIR}/cch_${f}.log
    ../build/apps/tetrapol_dump -t TCH -i ${OUT_DIR}/channel${f}.bits \
            >${OUT_DIR}/tch_${f}.json 2>${OUT_DIR}/tch_${f}.log
done
