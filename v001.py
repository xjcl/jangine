#!/usr/bin/python3


# USAGE:
# INTERACTIVE USAGE
#   python3 v001.py
# XBOARD -- SETUP
#   chmod +x ./v001.py    # needed so xboard can execute us
# XBOARD
#   xboard -fcp ./v001.py -debug
#                       # this will log to 'xboard.debug'
#                       # in addition to 'v001.py.log' (this program)

# this example file only makes some example moves (might be illegal!)
#   as black then exits (unexpectedly)



import time
import logging
import sys

logging.basicConfig(filename=sys.argv[0]+'.log', level=logging.DEBUG)

# redefine builtins: now they also log to file
def print(*args, **kwargs):
    __builtins__.print(*args, **kwargs)
    logging.info('>>> ' + ' '.join(str(m) for m in args))

def input(*args, **kwargs):
    ret = __builtins__.input(*args, **kwargs)
    logging.info(ret)
    return ret



move_number = 0

moves = [
    'move e7e5',  # pawn
    'move f8e7',  # bishop
    'move g8f6',  # knight
    'move b8c6',  # knight
    'move e8g8',  # castling
    #'move e2e1q' # promotion (to queen)
]


while True:
    i = input()

    # work both interactively AND with xboard
    if 'xboard' in i:
        print('feature myname="sys.argv[0]"')
        print('feature sigint=0 sigterm=0')  # else xboard sends KeyboardInterrupts
        print('feature done=1')

    # answer pings
    if i.startswith('ping'):
        print(i.replace('i', 'o'))

    # is a move like 'e2e4'
    if len(i) == 4:
        if i == 'post' or i == 'hard':
            pass
        elif move_number < len(moves):
            print(moves[move_number])
            move_number += 1
        else:
            i = 'q'

    if i == 'q' or i == 'quit':
        logging.shutdown()
        exit()

    time.sleep(.01)




