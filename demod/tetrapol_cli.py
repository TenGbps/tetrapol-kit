#!/usr/bin/python3

"""Remote control for tetrapol_rx.py"""

from xmlrpc import client
import sys


def print_help(prgname=''):
    print("usage: %s [ help | %s | %s | %s ]" % (
        prgname, ModAutoTune.name, ModOutput.name, ModPower.name))


class ModBase:
    def __init__(self, args):
        self.rpc = client.Server("http://localhost:60100")


class ModAutoTune(ModBase):
    name= 'autotune'

    def __init__(self, args):
        ModBase.__init__(self, args)
        if len(args) == 0:
            print(self.rpc.get_auto_tune())
        elif len(args) == 1:
            self.rpc.set_auto_tune(int(args[0]))
        else:
            ModAutoTune.help()

    @staticmethod
    def help():
        print("""Use signal on <CHANNEL> as frequency reference
    autotune            - get current autotune channel
    autotune <CHANNEL>  - set channel to tune to (use -1 to disable autotune)""")


class ModOutput:
    name = 'output'

    def __init__(self, args):
        ModBase.__init__(self, args)

        if len(args) < 2:
            ModOutput.help()
            return

        if args[0] in ("true", "enable", "open", "on"):
            enabled = True
        elif args[0] in ("false", "disable", "close", "off"):
            enabled = False
        else:
            ModOutput.help()
            return
        channels = list(set([int(c) for c in args[1:]]))
        self.rpc.set_output_enabled(channels, enabled)

    @staticmethod
    def help():
        print("""Enable/disable demodulator output(s):
    output open <CHANNEL> [<CHANNEL> [<CHANNEL> ... ]]
    output close <CHANNEL> [<CHANNEL> [<CHANNEL> ... ]]""")


class ModPower:
    """Print out channels power level in dB."""
    name = 'power'

    def __init__(self, args):
        ModBase.__init__(self, args)

        if len(args) > 1:
            pwr = self.rpc.get_channels_pwr([int(ch) for ch in args[1:]])
        else:
            pwr = self.rpc.get_channels_pwr()
        pwr.sort(reverse=True)
        for p in pwr:
            print(p)

    @staticmethod
    def help():
        print("""Print channel power levels
    power [<CHANNEL> [<CHANNEL> ... ]]""")


class ModStop:
    """Stop server."""
    name = 'stop'

    def __init__(self, args):
        ModBase.__init__(self, args)
        if len(args) > 1:
            self.help()
            return
        self.rpc.stop()

    @staticmethod
    def help():
        print("""Stop the server
    stop""")


if __name__ == '__main__':
    if len (sys.argv) < 2:
        print_help(sys.argv[0])
        exit(1)

    if sys.argv[1] == ModAutoTune.name:
        mod = ModAutoTune(sys.argv[2:])
        exit(0)
    if sys.argv[1] == ModOutput.name:
        mod = ModOutput(sys.argv[2:])
        exit(0)
    if sys.argv[1] == ModPower.name:
        mod = ModPower(sys.argv[2:])
        exit(0)
    if sys.argv[1] == ModStop.name:
        mod = ModStop(sys.argv[2:])
        exit(0)
    else:
        print_help(sys.argv[0])
        exit(1)

