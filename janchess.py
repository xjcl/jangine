#!/usr/bin/python3


import time
import logging
import random
import sys
import contextlib

logging.basicConfig(filename='janchess.log', level=logging.DEBUG)
logging.info(str(time.time()))

def print(*args, **kwargs):
    __builtins__.print(*args, **kwargs)
    logging.info('>>> ' + ' '.join(str(m) for m in args))

def input(*args, **kwargs):
    ret = __builtins__.input(*args, **kwargs)
    logging.info(ret)
    return ret



PAWN = 1
KNIGHT = 2
BISHOP = 4
ROOK = 8
QUEEN = 16
KING = 32

WHITE = 64
BLACK = 128

LETTERMAP = {0:'a', 1:'b', 2:'c', 3:'d', 4:'e', 5:'f', 6:'g', 7:'h'}
LETTERMAPINV = {v:k for k,v in LETTERMAP.items()}


def new_board():
    return [
        [BLACK + ROOK, BLACK + KNIGHT, BLACK + BISHOP, BLACK + QUEEN,
            BLACK + KING, BLACK + BISHOP, BLACK + KNIGHT, BLACK + ROOK],
        [BLACK + PAWN] * 8,
        [0] * 8,
        [0] * 8,
        [0] * 8,
        [0] * 8,
        [WHITE + PAWN] * 8,
        [WHITE + ROOK, WHITE + KNIGHT, WHITE + BISHOP, WHITE + QUEEN,
            WHITE + KING, WHITE + BISHOP, WHITE + KNIGHT, WHITE + ROOK],
    ]


def input_is_move(i):
    return len(i) >= 4 and i[0] in 'abcdefgh' and i[1] in '12345678' \
        and i[2] in 'abcdefgh' and i[3] in '12345678'

def pprint():
    [print(row) for row in board]



def gen_legals(white_turn=False):
    COLOR = WHITE if white_turn else BLACK
    ADD = -1 if white_turn else 1

    ret = list()
    for i, row in enumerate(board):
        for j, col in enumerate(row):
            if 1 <= i <= 6 and 0 <= j <= 7:
                if board[i][j] & PAWN and board[i][j] & COLOR:
                    if not board[i+ADD][j]:
                        ret.append((i,j,i+ADD,j))

    # pprint()
    # print(ret)

    if not ret:
        print('Error (no legal moves): undo')

    return ret


def make_move(mv, white_turn=True):
    piece = board [8 - int(mv[1])] [ LETTERMAPINV[mv[0]] ]
    board [8 - int(mv[1])] [ LETTERMAPINV[mv[0]] ] = 0
    board [8 - int(mv[3])] [ LETTERMAPINV[mv[2]] ] = piece



def calc_move(white_turn=False):
    mv = random.choice(gen_legals())

    piece = board[mv[0]][mv[1]]
    board[mv[0]][mv[1]] = 0
    board[mv[2]][mv[3]] = piece

    # [print(row) for row in board]

    return LETTERMAP[mv[1]] + str(8 - mv[0]) + LETTERMAP[mv[3]] + str(8 - mv[2])




board = new_board()

while True:
    i = input()
    ins = i.split(' ')[1:]

    if i == 'xboard':
        print('feature myname="janchess"')
        print('feature done=1')
        print('feature sigint=0 sigterm=0')  # else xboard sends KeyboardInterrupts

    elif i.startswith('time'):
        total_time = int(ins[0])
        time_per_move = total_time / 100

    elif i == 'new':
        board = new_board()

    elif i.startswith('ping'):
        print(i.replace('i', 'o'))

    # elif i.startswith('MOVE') or i == '?':
    elif input_is_move(i):
        make_move(i)
        time.sleep(.1)
        print('move ' + calc_move())

    elif i == 'quit' or i == 'q':
        logging.shutdown()
        exit()

    time.sleep(.01)









