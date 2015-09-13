#!/usr/bin/env python

# Copyright 2012 Dimitri Stolnikov <horiz0n@gmx.net>

# Usage:
# src$ ./demod/python/osmosdr-tetra_demod_fft.py -o /dev/stdout | ./float_to_bits /dev/stdin /dev/stdout | ./tetra-rx /dev/stdin
#
# Adjust the center frequency (-f) and gain (-g) according to your needs.
# Use left click in Wideband Spectrum window to roughly select a TETRA carrier.
# In Wideband Spectrum you can also tune by 1/4 of the bandwidth by clicking on the rightmost/leftmost spectrum side.
# Use left click in Channel Spectrum windows to fine tune the carrier by clicking on the left or right side of the spectrum.


import sys
import math
from gnuradio import gr, gru, eng_notation, blocks, filter, digital, analog
from gnuradio.eng_option import eng_option
from optparse import OptionParser
import osmosdr
import time
import threading
import subprocess

# applies frequency translation, resampling and demodulation

class top_block(gr.top_block):
  def __init__(self):
    gr.top_block.__init__(self)

    bitrate = 8000
    channel_bw = 12500
    chan0_freq = 358399864


    options = get_options()

    self.rfgain = options.gain

    self.channels = []
    for ch in options.channels.split(','):
        self.channels.append(int(ch))

    if options.frequency is None:
        self.ifreq = chan0_freq + (max(self.channels) + min(self.channels)) / 2 * channel_bw - 100000
    else:
        self.ifreq = options.frequency

    self.src = osmosdr.source(options.args)
    self.src.set_center_freq(self.ifreq)
    self.src.set_sample_rate(options.sample_rate)
    self.src.set_freq_corr(options.ppm, 0)

    if self.rfgain is None:
        self.src.set_gain_mode(True, 0)

        self.iagc = 1
        self.rfgain = 0
    else:
        self.iagc = 0
        self.src.set_gain_mode(0)
        self.src.set_gain(self.rfgain)
        self.src.set_if_gain(37)

    # may differ from the requested rate
    sample_rate = int(self.src.get_sample_rate())
    sys.stderr.write("sample rate: %d\n" % (sample_rate))

    first_decim = int(options.sample_rate / bitrate / 2)
    sys.stderr.write("decim: %d\n" % (first_decim))

    out_sample_rate=sample_rate/first_decim
    sys.stderr.write("output sample rate: %d\n" % (out_sample_rate))

    sps=out_sample_rate/bitrate
    sys.stderr.write("samples per symbol: %d\n" % (sps))

    self.tuners = []
    self.afc_probes = []
    for ch in range(0,len(self.channels)):
        bw = (9200 + options.afc_ppm_threshold)/2
        taps = filter.firdes.low_pass(1.0, sample_rate, bw, bw*options.transition_width, filter.firdes.WIN_HANN)
        offset = chan0_freq + channel_bw * self.channels[ch] - self.ifreq
        sys.stderr.write("channel[%d]: %d frequency=%d, offset=%d Hz\n" % (ch, self.channels[ch], self.ifreq+offset, offset))


        tuner = filter.freq_xlating_fir_filter_ccc(first_decim, taps, offset, sample_rate)
        self.tuners.append(tuner)

        demod = digital.gmsk_demod(samples_per_symbol=sps)

        if options.output_pipe is None:
            file = options.output_file.replace('%%', str(self.channels[ch]))
            output = blocks.file_sink(gr.sizeof_char, file)
        else:
            cmd = options.output_pipe.replace('%%', str(self.channels[ch]))
            pipe = subprocess.Popen(cmd, stdin=subprocess.PIPE, shell=True)
            fd = pipe.stdin.fileno()
            output = blocks.file_descriptor_sink(gr.sizeof_char, fd)

        self.connect((self.src, 0), (tuner, 0))
        self.connect((tuner, 0), (demod, 0))
        self.connect((demod, 0), (output, 0))

        fm_demod = analog.fm_demod_cf(sample_rate/first_decim, 1, 5000, 3000, 4000)
        integrate = blocks.integrate_ff(32000)
        afc_probe = blocks.probe_signal_f()
        self.afc_probes.append(afc_probe)

        self.connect((tuner, 0), (fm_demod,0))
        self.connect((fm_demod, 0), (integrate,0))
        self.connect((integrate, 0), (afc_probe, 0))

    def _variable_function_probe_0_probe():
        while True:
            time.sleep(options.afc_period)
            for ch in range(0,len(self.channels)):
                err = self.afc_probes[ch].level()
                if abs(err) < options.afc_ppm_threshold:
                    continue
                freq = self.tuners[ch].center_freq() + err * options.afc_gain
                self.tuners[ch].set_center_freq(freq)
                sys.stderr.write("Chan %d freq err: %5.0f\tfreq: %f\n" % (self.channels[ch], err, freq))
            sys.stderr.write("\n")
    _variable_function_probe_0_thread = threading.Thread(target=_variable_function_probe_0_probe)
    _variable_function_probe_0_thread.daemon = True
    _variable_function_probe_0_thread.start()



def get_options():
    parser = OptionParser(option_class=eng_option)

    parser.add_option("-a", "--args", type="string", default="", help="gr-osmosdr device arguments")
    parser.add_option("-s", "--sample-rate", type="eng_float", default=1024000, help="receiver sample rate (default %default)")
    parser.add_option("-f", "--frequency", type="eng_float", default=None, help="receiver center frequency (default %default)")
    parser.add_option("-g", "--gain", type="eng_float", default=None, help="set receiver gain")
    parser.add_option("-c", "--channels", type="string", default=None, help="channel numbers")
    parser.add_option("-p", "--ppm", dest="ppm", type="eng_float", default=eng_notation.num_to_str(0), help="Frequency correction")
    parser.add_option("-t", "--transition-width", type="eng_float", default=0.2, help="low pass transition width (default %default)")
    parser.add_option("-G", "--afc-gain", type="eng_float", default=0.2, help="afc gain (default %default)")
    parser.add_option("-P", "--afc-period", type="eng_float", default=2, help="afc period (default %default)")
    parser.add_option("-T", "--afc-ppm-threshold", type="eng_float", default=100, help="afc threshold (default %default)")
    parser.add_option("-o", "--output-file", type="string", default="channel%%.bits", help="specify the bit output file")
    parser.add_option("-O", "--output-pipe", type="string", default=None, help="specify shell pipe to send output")
    (options, args) = parser.parse_args()
    if len(args) != 0:
        parser.print_help()
        raise SystemExit, 1

    return (options)

if __name__ == '__main__':
        tb = top_block()
#        tb.run(True)
        tb.start()
        tb.wait()
