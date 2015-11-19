#!/usr/bin/python3



# XXX automate is_inside checks on board

import time
import logging
import random
import sys
# import contextlib
import itertools

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



WHITEKINGMOVED = False
BLACKKINGMOVED = False

# XXX used for en-passant
LASTMOVE = None

board = None
IM_WHITE = False
started = True
BUF = ''



def is_inside(i, j):
    return 0 <= i <= 7 and 0 <= j <= 7

# DON'T NEGATE
def is_empty(i, j):
    return is_inside(i, j) and not board[i][j]


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


def gentuples():
    return itertools.product(range(7), range(7))

def input_is_move(i):
    return len(i) >= 4 and i[0] in 'abcdefgh' and i[1] in '12345678' \
        and i[2] in 'abcdefgh' and i[3] in '12345678'

def sq_to_str1(sq):
    if sq & BLACK: return 'B'
    if sq & WHITE: return 'W'
    return ' '

def sq_to_str2(sq):
    if sq & PAWN: return 'P'
    if sq & KNIGHT: return 'N'
    if sq & BISHOP: return 'B'
    if sq & ROOK: return 'R'
    if sq & QUEEN: return 'Q'
    if sq & KING: return 'K'
    return ' '

def pprint():
    print()
    for row in board:
        print(' '.join(sq_to_str1(sq) + sq_to_str2(sq) for sq in row))
    print()





def king_not_in_check(col=...):

    col  = WHITE if IM_WHITE else BLACK
    ncol = WHITE if not IM_WHITE else BLACK
    myking = KING + col
    ADD = -1 if IM_WHITE else 1

    for i, row in enumerate(board):
        for j, col in enumerate(row):
            bij = board[i][j]
            if bij & ncol:
                if bij & PAWN:
                    if is_inside(i-ADD, j+1) and board[i-ADD][j+1] == myking:
                        return False
                    if is_inside(i-ADD, j-1) and board[i-ADD][j-1] == myking:
                        return False
                elif bij & KNIGHT:
                    for x,y in ((1,2), (2,1), (-1,2), (2,-1), (-1,-2), (-2,-1), (1,-2), (-2,1)):
                        if is_inside(i+x, j+y):
                            if board[i+x][j+y] == myking:
                                return False
                elif bij & BISHOP or bij & QUEEN:
                    for a,b in ((1,1), (1,-1), (-1,1), (-1,-1)):
                        for k in range(1, 8):
                            if is_inside(i+a*k, j+b*k):
                                if board[i+a*k][j+b*k] == myking:
                                    return False
                                elif board[i+a*k][j+b*k]:
                                    break  # important
                            else:
                                break
                if bij & ROOK or bij & QUEEN:
                    for a,b in ((0,1), (1,0), (0,-1), (-1,0)):
                        for k in range(1, 8):
                            if is_inside(i+a*k, j+b*k):
                                if board[i+a*k][j+b*k] == myking:
                                    return False
                                elif board[i+a*k][j+b*k]:
                                    break  # important
                            else:
                                break
                elif bij & KING:
                    for a,b in ((0,1), (1,0), (0,-1), (-1,0), (1,1), (-1,-1), (1,-1), (-1,1)):
                        if is_inside(i+a, j+b):
                            if board[i+a][j+b] == myking:
                                return False

    return True


# XXX add col
def gen_legals():
    COLOR  = WHITE if IM_WHITE else BLACK
    NCOLOR = BLACK if IM_WHITE else WHITE
    ADD = -1 if IM_WHITE else 1

    ret = []

    for i, row in enumerate(board):
        for j, col in enumerate(row):
            bij = board[i][j]
            if bij & COLOR:
                if bij & PAWN:
                    if is_empty(i+ADD, j):
                        ret.append((i,j,i+ADD,j))
                    if is_inside(i+ADD, j+1) and board[i+ADD][j+1] & NCOLOR:
                        ret.append((i,j,i+ADD,j+1))
                    if is_inside(i+ADD, j-1) and board[i+ADD][j-1] & NCOLOR:
                        ret.append((i,j,i+ADD,j-1))
                elif bij & KNIGHT:
                    for x,y in ((1,2), (2,1), (-1,2), (2,-1), (-1,-2), (-2,-1), (1,-2), (-2,1)):
                        if is_inside(i+x, j+y):
                            if not board[i+x][j+y] or board[i+x][j+y] & NCOLOR:
                                ret.append((i,j,i+x,j+y))
                elif bij & BISHOP or bij & QUEEN:
                    for a,b in ((1,1), (1,-1), (-1,1), (-1,-1)):
                        for k in range(1, 8):
                            if is_inside(i+a*k, j+b*k):
                                if not board[i+a*k][j+b*k]:
                                    ret.append((i,j,i+a*k,j+b*k))
                                elif board[i+a*k][j+b*k] & NCOLOR:
                                    ret.append((i,j,i+a*k,j+b*k))
                                    break  # important
                                elif board[i+a*k][j+b*k] & COLOR:
                                    break  # important
                            else:
                                break
                if bij & ROOK or bij & QUEEN:
                    for a,b in ((0,1), (1,0), (0,-1), (-1,0)):
                        for k in range(1, 8):
                            if is_inside(i+a*k, j+b*k):
                                if not board[i+a*k][j+b*k]:
                                    ret.append((i,j,i+a*k,j+b*k))
                                elif board[i+a*k][j+b*k] & NCOLOR:
                                    ret.append((i,j,i+a*k,j+b*k))
                                    break  # important
                                elif board[i+a*k][j+b*k] & COLOR:
                                    break  # important
                            else:
                                break
                elif bij & KING:
                    for a,b in ((0,1), (1,0), (0,-1), (-1,0), (1,1), (-1,-1), (1,-1), (-1,1)):
                        if is_inside(i+a, j+b):
                            if not board[i+a][j+b] or board[i+a][j+b] & NCOLOR:
                                ret.append((i,j,i+a,j+b))



    ret2 = []

    for mv in ret:
        hit_piece = make_move(mv)
        assert not hit_piece & KING

        if king_not_in_check():
            ret2.append(mv)

        unmake_move(mv, hit_piece)

    if not ret2:
        print('result 1/2-1/2')  # stalemate
        logging.shutdown()
        exit()


    return ret2



def make_move(mv):
    piece = board[mv[0]][mv[1]]
    hit_piece = board[mv[2]][mv[3]]
    board[mv[0]][mv[1]] = 0
    board[mv[2]][mv[3]] = piece
    return hit_piece


def unmake_move(mv, hit_piece):
    piece = board[mv[2]][mv[3]]
    board[mv[2]][mv[3]] = hit_piece
    board[mv[0]][mv[1]] = piece


def make_move_str(mv):
    print('making move ' + mv)

    a = 8 - int(mv[1])
    b = LETTERMAPINV[mv[0]]
    c = 8 - int(mv[3])
    d = LETTERMAPINV[mv[2]]

    make_move((a, b, c, d))

    LASTMOVE = (a,b,c,d)


def calc_move():
    mv = random.choice(gen_legals())

    piece = board[mv[0]][mv[1]]
    board[mv[0]][mv[1]] = 0
    board[mv[2]][mv[3]] = piece

    LASTMOVE = mv

    # [print(row) for row in board]

    return LETTERMAP[mv[1]] + str(8 - mv[0]) + LETTERMAP[mv[3]] + str(8 - mv[2])









if len(sys.argv) >= 2:
    if sys.argv[1].startswith('-t'):
        king_check_test = [
            [BLACK + KING, 0, 0, 0, WHITE + QUEEN, 0, 0, 0],
            [0] * 8,
            [0] * 8,
            [0] * 8,
            [0] * 8,
            [0] * 8,
            [0] * 8,
            [0] * 8,
        ]

        board = king_check_test
        assert not king_not_in_check()

        exit()




while True:
    i = input()
    ins = i.split(' ')[1:]

    # work both interactively AND with xboard
    if i == 'xboard':
        print('feature myname="janchess"')
        print('feature sigint=0 sigterm=0')  # else xboard sends KeyboardInterrupts
        print('feature done=1')

    elif i == 'white':
        IM_WHITE = True
        board = new_board()

    elif i.startswith('time'):
        total_time = int(ins[0])
        time_per_move = total_time / 100

    elif i == 'force':
        started = False

    elif i == 'new':
        IM_WHITE = False
        board = new_board()

    # answer pings
    elif i.startswith('ping'):
        print(i.replace('i', 'o'))

    elif input_is_move(i) and not started:
        BUF = i

    elif input_is_move(i) and started:
        print('RECV MOVE', i)
        make_move_str(i)
        print('move ' + calc_move())

    # go/force stuff needed, else xboard skips second move
    elif i == 'go':
        started = True
        if BUF:
            make_move_str(BUF)
            print('move ' + calc_move())
            BUF = ''
        else:
            print('move ' + calc_move())

    elif i == 'quit' or i == 'q':
        pprint()
        logging.shutdown()
        exit()










