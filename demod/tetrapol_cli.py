#!/usr/bin/python3

"""Remote control for tetrapol_rx.py"""

from xmlrpc import client
import sys


def print_help(prgname=''):
    print("usage: %s [ help | %s ]" % (
        prgname, ModAutoTune.name, ))


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


if __name__ == '__main__':
    if len (sys.argv) < 2:
        print_help(sys.argv[0])
        exit(1)

    if sys.argv[1] == ModAutoTune.name:
        mod = ModAutoTune(sys.argv[2:])
        exit(0)
    else:
        print_help(sys.argv[0])
        exit(1)

