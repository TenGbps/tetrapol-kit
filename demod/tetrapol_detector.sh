#!/bin/sh

CH_BW=12500
FREQ=393e6
SAMP_RATE=2400000
OUT_DIR=tmp
CHANNELS_FILE=channels.json
MAX_CHANNELS=8
GAIN=30.4


if ! [ -d "${OUT_DIR}" ]; then
    echo "OUT_DIR (${OUT_DIR}) does not exist"
    exit 1
fi

python2 tetrapol_detector.py \
    -f ${FREQ} \
    -g ${GAIN} \
    -i osmo-sdr:// \
    -o "${OUT_DIR}/${CHANNELS_FILE}"  \
    -r 1 \
    -s ${SAMP_RATE}

FREQS=`grep freq tmp/channels.json  | sed -e 's/.*freq": //' -e 's/\\..*//' | head -n ${MAX_CHANNELS} | tr \\\\012 ,`

timeout 30 \
    python2 demod.py \
        -g ${GAIN} \
        -s ${SAMP_RATE} \
        -o ${OUT_DIR}/channel%%.bits \
        -l "${FREQS}"

for f in `echo "${FREQS}" | tr , ' '`; do
    ../build/apps/tetrapol_dump -t CCH -i ${OUT_DIR}/channel${f}.bits 2>${OUT_DIR}/cch_${f}.log
    ../build/apps/tetrapol_dump -t TCH -i ${OUT_DIR}/channel${f}.bits 2>${OUT_DIR}/tch_${f}.log
done
