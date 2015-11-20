#!/usr/bin/python3



# XXX automate is_inside checks on board

import time
import logging
import random
import sys
# import contextlib
import itertools
import functools
import collections

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

PIECEMAP = {PAWN:'p', KNIGHT:'n', BISHOP:'b', ROOK:'r', QUEEN:'q'}
PIECEMAPINV = {v:k for k,v in PIECEMAP.items()}

PIECEVALS = {PAWN: 1, KNIGHT: 3, BISHOP: 3, ROOK: 5, QUEEN: 9, KING: 300, 0: 0}


# XXX castling
WHITEKINGMOVED = False
BLACKKINGMOVED = False
WHITELEFTROOKMOVED = False
BLACKLEFTROOKMOVED = False
WHITERIGHTROOKMOVED = False
BLACKRIGHTROOKMOVED = False

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
    return itertools.product(range(8), range(8))

# def genpieces():
#     return (KNIGHT, BISHOP, ROOK, QUEEN)

def genpieces():
    return ('n', 'b', 'r', 'q')

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









Evaluation = collections.namedtuple('Evaluation', 'value moves')
_verbosity = 1  # depth of search to print
inf = 3000

# https://chessprogramming.wikispaces.com/Turochamp#Evaluation%20Features
def turochamp():
    eval = 0
    for i,j in gentuples():
        bij = board[i][j] & ~WHITE & ~BLACK
        eval += (1 if board[i][j] & WHITE else -1) * PIECEVALS[bij]
    return eval

# 5 once ---> 55505 calls
def minmax(COLOR, _depth=3, _verbosity=1):
    if not _depth:   return Evaluation(turochamp(), [])

    best = Evaluation(-inf if COLOR == WHITE else inf, None)
    for mv in genlegals(COLOR):
        hit_piece = make_move(mv)
        rec = minmax(BLACK if COLOR == WHITE else WHITE, _depth - 1, _verbosity - 1)
        if ((rec.value > best.value) if COLOR == WHITE else (rec.value < best.value)):
            best = Evaluation(rec.value, [mv] + rec.moves)
        unmake_move(mv, hit_piece)
        if _verbosity > 0: print(mv, rec)
    if _verbosity > 0: print('chose', best)
    return best



def king_not_in_check(COLOR):
    NCOLOR = BLACK if COLOR == WHITE else WHITE
    ADD = -1 if COLOR == WHITE else 1
    myking = KING + COLOR

    for i, j in gentuples():
        bij = board[i][j]
        if not bij & NCOLOR:
            continue
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


def genlegals(COLOR):
    NCOLOR = BLACK if COLOR == WHITE else WHITE
    ADD = -1 if COLOR == WHITE else 1
    PROMRANK = 0 if COLOR == WHITE else 7

    ret = []

    for i, j in gentuples():
        bij = board[i][j]
        if not bij & COLOR:
            continue
        if bij & PAWN:
            lr = len(ret)
            if is_empty(i+ADD, j):
                ret.append((i,j,i+ADD,j))
            if is_inside(i+ADD, j+1) and board[i+ADD][j+1] & NCOLOR:
                ret.append((i,j,i+ADD,j+1))
            if is_inside(i+ADD, j-1) and board[i+ADD][j-1] & NCOLOR:
                ret.append((i,j,i+ADD,j-1))
            if i+ADD == PROMRANK:
                temp = []
                while lr < len(ret):
                    for p in genpieces():
                        temp.append(ret[lr] + (p,))
                    del ret[lr]
                ret += temp
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

        if king_not_in_check(COLOR):
            ret2.append(mv)

        unmake_move(mv, hit_piece)


    return ret2



def make_move(mv):
    piece = board[mv[0]][mv[1]]
    hit_piece = board[mv[2]][mv[3]]
    board[mv[0]][mv[1]] = 0
    board[mv[2]][mv[3]] = piece
    if len(mv) == 5:
        board[mv[2]][mv[3]] = PIECEMAPINV[mv[4]] + (WHITE if mv[2] == 0 else BLACK)
    return hit_piece


def unmake_move(mv, hit_piece):
    piece = board[mv[2]][mv[3]]
    board[mv[2]][mv[3]] = hit_piece
    board[mv[0]][mv[1]] = piece
    if len(mv) == 5:
        board[mv[0]][mv[1]] = PAWN + (WHITE if mv[2] == 0 else BLACK)


def make_move_str(mv):
    print('recv move ' + mv)

    a = 8 - int(mv[1])
    b = LETTERMAPINV[mv[0]]
    c = 8 - int(mv[3])
    d = LETTERMAPINV[mv[2]]
    if len(mv) == 5:
        to_mv = (a, b, c, d, mv[4])
    else:
        to_mv = (a, b, c, d)

    make_move(to_mv)
    LASTMOVE = to_mv


def calc_move():
    # legals = genlegals(WHITE if IM_WHITE else BLACK)
    # if not legals:
    #     print('out of moves')
    #     pprint()
    #     return  # either stalemate or checkmate
    # mv = random.choice(legals)

    mv = minmax(WHITE if IM_WHITE else BLACK).moves[0]

    make_move(mv)
    LASTMOVE = mv

    # [print(row) for row in board]
    return LETTERMAP[mv[1]] + str(8 - mv[0]) + LETTERMAP[mv[3]] + str(8 - mv[2]) + (mv[4] if len(mv) == 5 else '')









if len(sys.argv) >= 2:
    if sys.argv[1].startswith('-t'):
        t1 = [
            [BLACK + KING, 0, 0, 0, WHITE + QUEEN, 0, 0, 0],
            [0] * 8,
            [0] * 8,
            [0] * 8,
            [0] * 8,
            [0] * 8,
            [0] * 8,
            [0] * 8,
        ]

        board = t1
        assert not king_not_in_check()

        IM_WHITE = True

        t2 = [
            [0] * 8,
            [0] * 8,
            [0] * 8,
            [0] * 8,
            [0, 0, 0, 0, 0, 0, 0, BLACK + QUEEN],
            [0] * 8,
            [0] * 8,
            [0, 0, 0, 0, WHITE + KING, 0, 0, 0],
        ]

        board = t2
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

    elif i.startswith('result'):
        started = False

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
        make_move_str(i)
        mv = calc_move()
        if mv:  print('move ' + mv)

    # go/force stuff needed, else xboard skips second move
    elif i == 'go':
        started = True
        if BUF:
            make_move_str(BUF)
            BUF = ''
        mv = calc_move()
        if mv:  print('move ' + mv)

    elif i == 'quit' or i == 'q':
        pprint()
        logging.shutdown()
        exit()










