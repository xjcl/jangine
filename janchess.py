#!/usr/local/bin/pypy3



# XXX automate is_inside checks on board

import argparse
import collections
# import contextlib
import functools
import itertools
import logging
import random
import sys
import time


parser = argparse.ArgumentParser(description='Plays chess poorly.')

parser.add_argument('-m', '--mode', type=str, default='quiescence',
                   help='random or quiescence (default)')

parser.add_argument('-t', '--test', action='store_true',
                   help='run tests???!!?')

args = parser.parse_args()



# XXX C-rewrite

logging.basicConfig(filename='janchess.log', level=logging.DEBUG, filemode='w')
logging.info(str(time.time()))

def print(*args, **kwargs):
    __builtins__.print(*args, **kwargs)
    logging.info('>>> ' + ' '.join(str(m) for m in args))

def input(*args, **kwargs):
    ret = __builtins__.input(*args, **kwargs)
    logging.info(ret)
    return ret



PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, WHITE, BLACK = 1, 2, 4, 8, 16, 32, 64, 128

PIECECHARS = {PAWN:'p', KNIGHT:'n', BISHOP:'b', ROOK:'r', QUEEN:'q'}
PIECECHARSINV = {v:k for k,v in PIECECHARS.items()}

inf = 33000
PIECEVALS = {PAWN: 100, KNIGHT: 300, BISHOP: 300, ROOK: 500, QUEEN: 900, KING: inf, 0: 0}

CASTLINGWHITE = (True, True, True)
CASTLINGBLACK = (True, True, True)

# used for en-passant
LASTMOVE = None

board = None
IM_WHITE = False
started = True
BUF = ''


# XXX instead use lru-cache?
KILLERHEURISTIC = collections.defaultdict(int)

# XXX currently castling-rights-agnostic!!
# XXX currently draw-by-rep-agnostic!!
TRANSPOSITIONS = collections.defaultdict(int)



def is_inside(i, j):
    return 0 <= i <= 7 and 0 <= j <= 7

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


gentuples = list(itertools.product(range(8), range(8)))

def input_is_move(i):
    return len(i) >= 4 and i[0] in 'abcdefgh' and i[1] in '12345678' \
        and i[2] in 'abcdefgh' and i[3] in '12345678'

def sq_to_str1(sq):
    if sq & BLACK: return 'B'
    if sq & WHITE: return 'W'
    return ' '

def sq_to_str2(sq):
    for k, v in [(PAWN, 'P'), (KNIGHT, 'N'), (BISHOP, 'B'), (ROOK, 'R'), (QUEEN, 'Q'), (KING, 'K')]:
        if sq & k: return v
    return ' '

def str_to_sq1(sq):
    return {'B':BLACK, 'W':WHITE, ' ':0}[sq]

def str_to_sq2(sq):
    return {'P':PAWN, 'N':KNIGHT, 'B':BISHOP, 'R':ROOK, 'Q':QUEEN, 'K':KING, ' ':0}[sq]

def pprint():
    print()
    for row in board:
        print(' '.join(sq_to_str1(sq) + sq_to_str2(sq) for sq in row))
    print()

def log_to_board(s):
    global board
    board = [[], [], [], [], [], [], [], []]
    for j, row in enumerate(s.split('\n')):
        i = row.find('>>>') + 4
        for k in range(8):
            if i+3*k+1 >= len(row):  # subl trims trailing whitespace...
                board[j] += [0] * (8 - k)
                break
            board[j].append(str_to_sq1(row[i+3*k]) + str_to_sq2(row[i+3*k+1]))








Evaluation = collections.namedtuple('Evaluation', 'value moves')
Evaluation.__eq__ = lambda self, other: self.value == other.value
Evaluation.__lt__ = lambda self, other: self.value < other.value
Evaluation.__gt__ = lambda self, other: self.value > other.value
Evaluation.__le__ = lambda self, other: self < other or self == other
Evaluation.__add__ = lambda self, other: Evaluation(self.value + other, self.moves)


# https://chessprogramming.wikispaces.com/Turochamp#Evaluation%20Features
def turochamp():

    eval = 0

    for i,j in gentuples:
        # material counting
        bijp = board[i][j] & ~WHITE & ~BLACK
        bij = board[i][j]
        MUL = 1 if bij & WHITE else -1
        eval += MUL * PIECEVALS[bijp]

        # king safety
        if bijp == KING:
            i = 0
            for a,b in PIECEDIRS[QUEEN]:
                for k in PIECERANGE[QUEEN]:
                    if not is_inside(i+a*k, j+b*k) or board[i+a*k][j+b*k]:
                        break
                    # eval -= MUL * 8
                    i += 1
            eval -= MUL * 15 * round(i**.5)

        # pawn advancement
        if bij & PAWN:
            eval += 10 * (7-i if bij & WHITE else -i)


    # center control
    center = [board[3][3], board[3][4], board[4][3], board[4][4]]
    eval += 15 * len([x for x in center if x & WHITE])
    eval -= 15 * len([x for x in center if x & BLACK])

    # mobility (possible moves) + (implicitly) king in check
    for COLOR, MUL in [(WHITE, 1), (BLACK, -1)]:
        gl = genlegals(COLOR, ignore_king_capture=True)  # XXX maybe calc it directly and dont use gl
        if not gl:
            if king_not_in_check(COLOR):  # stalemate
                return 0
            return MUL * (-inf+1)
        j = 0
        while j < len(gl):
            ref_mv = gl[j]
            i = 0
            if not board[ref_mv[0]][ref_mv[1]] & QUEEN:
                while j < len(gl) and gl[j][0:2] == ref_mv[0:2]:
                    i += 1
                    if board[gl[j][2]][gl[j][3]]:  # captures count double
                        i += 1
                    j += 1
            eval += MUL * 13 * round(i**.5)
            j += 1


    # XXX didn't consider EP once

    # XXX maybe mobility thru friendly piece (/= pawn) counts half or so
    # XXX
    #   INFO:root:>>> BBBB  Evaluation(value=-162, moves=[(0, 4, 7, 4), (7, 4, 7, 6), (2, 2, 1, 2), (0, 3, 0, 4)])
    # INFO:root:>>>       BB BK BR
    # INFO:root:>>>       BP       BQ
    # INFO:root:>>>       WQ       WP
    # INFO:root:>>>       BP       BR
    # INFO:root:>>>    BP WP             WB
    # INFO:root:>>> BP WP    WP          WP
    # INFO:root:>>> WP    WK             WR
    # INFO:root:>>>                   WR



    # INFO:root:>>>
    # INFO:root:>>>       BK
    # INFO:root:>>>       BP
    # INFO:root:>>>
    # INFO:root:>>>
    # INFO:root:>>>    WB WP             WP
    # INFO:root:>>>          WP    WK
    # INFO:root:>>>
    # INFO:root:>>> BQ
    # INFO:root:>>>
    # INFO:root:>>> 1131
    # INFO:root:>>> (0, 2, 1, 3) Evaluation(value=-306, moves=[(5, 5, 5, 6), (7, 0, 4, 3)])
    # INFO:root:>>> (7, 0, 7, 3) Evaluation(value=-367, moves=[(5, 5, 5, 4), (7, 3, 7, 2), (4, 1, 6, 3), (0, 2, 0, 3)])
    # INFO:root:>>> (1, 2, 2, 2) Evaluation(value=-365, moves=[(5, 5, 4, 4), (7, 0, 7, 7), (4, 4, 3, 5)])
    # INFO:root:>>> (0, 2, 1, 1) Evaluation(value=-356, moves=[(4, 1, 3, 2), (7, 0, 3, 4)])
    # INFO:root:>>> (7, 0, 6, 0) Evaluation(value=-361, moves=[(5, 5, 4, 6), (6, 0, 0, 0)])
    # INFO:root:>>> (7, 0, 7, 7) Evaluation(value=-301, moves=[(5, 5, 5, 4), (5, 4, 6, 3)])
    # INFO:root:>>> (7, 0, 7, 4) Evaluation(value=-637, moves=[(7, 4, 4, 1), (5, 5, 5, 6)])
    # INFO:root:>>> (7, 0, 0, 0) Evaluation(value=-581, moves=[(5, 5, 5, 4), (0, 0, 4, 4), (5, 4, 6, 5), (4, 4, 5, 3), (5, 3, 4, 2), (4, 1, 6, 3)])
    # INFO:root:>>> (7, 0, 7, 1) Evaluation(value=-301, moves=[(5, 5, 5, 4), (5, 4, 6, 3)])
    # INFO:root:>>> (7, 0, 3, 4) Evaluation(value=-499, moves=[(5, 3, 4, 3), (3, 4, 4, 3), (5, 5, 6, 4)])
    # INFO:root:>>> (7, 0, 7, 2) Evaluation(value=-394, moves=[(0, 2, 1, 1)])
    # INFO:root:>>> (7, 0, 7, 6) Evaluation(value=-406, moves=[(5, 5, 4, 4), (7, 6, 4, 3), (4, 4, 3, 5)])
    # INFO:root:>>> (1, 2, 3, 2) Evaluation(value=-627, moves=[(5, 5, 5, 4), (3, 2, 4, 1), (5, 4, 6, 3)])
    # INFO:root:>>> (0, 2, 0, 3) Evaluation(value=-386, moves=[(5, 5, 5, 4), (7, 0, 3, 4), (5, 4, 6, 5)])
    # INFO:root:>>> (7, 0, 7, 5) Evaluation(value=-324, moves=[(5, 5, 5, 6)])
    # INFO:root:>>> (7, 0, 4, 0) Evaluation(value=-358, moves=[(5, 5, 5, 4), (5, 4, 6, 5)])
    # INFO:root:>>> (7, 0, 4, 3) Evaluation(value=-504, moves=[(5, 5, 6, 4), (4, 3, 5, 3), (6, 4, 6, 5)])
    # INFO:root:>>> (0, 2, 0, 1) Evaluation(value=-416, moves=[(5, 5, 5, 4), (7, 0, 3, 4), (5, 4, 6, 5)])
    # INFO:root:>>> (7, 0, 2, 0) Evaluation(value=-368, moves=[(5, 5, 5, 4), (5, 4, 6, 5)])
    # INFO:root:>>> (7, 0, 1, 6) Evaluation(value=-411, moves=[(5, 5, 5, 4), (1, 6, 1, 3)])
    # INFO:root:>>> (7, 0, 5, 0) Evaluation(value=-358, moves=[(5, 5, 5, 4), (5, 4, 6, 5)])
    # INFO:root:>>> (7, 0, 5, 2) Evaluation(value=-637, moves=[(4, 1, 5, 0), (5, 2, 5, 0), (5, 5, 5, 6)])
    # INFO:root:>>> (7, 0, 0, 7) Evaluation(value=-431, moves=[(5, 5, 5, 4), (0, 7, 4, 7), (5, 4, 6, 3)])
    # INFO:root:>>> (7, 0, 3, 0) Evaluation(value=-358, moves=[(5, 5, 5, 4), (5, 4, 6, 5)])
    # INFO:root:>>> (7, 0, 2, 5) Evaluation(value=-511, moves=[(5, 5, 4, 6), (2, 5, 5, 5), (4, 6, 3, 6), (5, 5, 5, 3), (3, 6, 2, 5)])
    # INFO:root:>>> (7, 0, 6, 1) Evaluation(value=-328, moves=[(5, 5, 5, 4), (5, 4, 4, 5)])
    # INFO:root:>>> (7, 0, 1, 0) Evaluation(value=-406, moves=[(5, 5, 4, 4), (1, 0, 4, 3), (4, 4, 3, 5)])
    # INFO:root:>>> BBBB  Evaluation(value=-637, moves=[(7, 0, 7, 4), (7, 4, 4, 1), (5, 5, 5, 6)])
    # INFO:root:>>> move a1e1
    # castling (will be weighed with king safety)


    # WE NEED THREE FOLD REP RULE!!!

    # INFO:root:>>>
    # INFO:root:>>> WK    BK
    # INFO:root:>>>
    # INFO:root:>>>          BQ
    # INFO:root:>>>
    # INFO:root:>>>
    # INFO:root:>>>
    # INFO:root:>>>
    # INFO:root:>>>
    # INFO:root:>>> 6
    # INFO:root:>>> (3, 3, 0, 0) Evaluation(value=0, moves=[(1, 0, 0, 0), (1, 2, 0, 2), (0, 0, 1, 0), (0, 2, 0, 3)])
    # INFO:root:>>> (3, 3, 2, 2) Evaluation(value=0, moves=[])
    # INFO:root:>>> (1, 2, 2, 2) Evaluation(value=-918, moves=[(1, 0, 0, 0), (2, 2, 1, 3), (0, 0, 1, 0)])
    # INFO:root:>>> (1, 2, 2, 3) Evaluation(value=-958, moves=[(1, 0, 0, 1), (2, 3, 1, 3)])
    # INFO:root:>>> (3, 3, 4, 4) Evaluation(value=-32999, moves=[(1, 0, 2, 0), (4, 4, 4, 0)])
    eval += 30 if (CASTLINGWHITE[1] and (CASTLINGWHITE[0] and not board[7][1] and not board[7][2] \
        and not board[7][3] or CASTLINGWHITE[2] and not board[7][5] and not board[7][6])) else 0
    eval -= 30 if (CASTLINGBLACK[1] and (CASTLINGBLACK[0] and not board[0][1] and not board[0][2] \
        and not board[0][3] or CASTLINGBLACK[2] and not board[0][5] and not board[0][6])) else 0

    return eval






def eval_quies(COLOR, mv, hit_piece):
    NCOLOR = BLACK if COLOR == WHITE else WHITE

    quies = 20

    if ((mv[0] > mv[2]) if COLOR == WHITE else (mv[0] < mv[2])):
        quies *= .8
    # if ((mv[0] < mv[2]) if COLOR == WHITE else (mv[0] > mv[2])):
    #     quies *= 1.2

    if hit_piece or len(mv) == 5 and mv[4] != 'c':
        quies *= 0
        # we can't let that happen: else it calculates n-1 moves and then captures
        #   since it doesn't see the opponent's recapture, it evals it favorably

    # XXX  also 0-rate reactions to checks --> maybe in def quiescence?
    if not king_not_in_check(NCOLOR) or board[mv[0]][mv[1]] & KING:
        quies *= 0

    return quies





def killer_order(mvs):
    return sorted(mvs, key=lambda x: KILLERHEURISTIC[x], reverse=True)


def quiescence(COLOR, alpha=Evaluation(-inf+1, []), beta=Evaluation(+inf-1, []), quies=41, depth=0):

    global TRANSPOSITIONS

    if quies <= 0 or depth > 7:     return Evaluation(turochamp(), [])
    if depth == 0:
        print(len(TRANSPOSITIONS))
        TRANSPOSITIONS = collections.defaultdict(int)  # print: bursts the pipe lol

    best = Evaluation(-inf if COLOR == WHITE else inf, None)

    hashable_board = tuple(tuple(row) for row in board)
    if hashable_board in TRANSPOSITIONS:
        return TRANSPOSITIONS[hashable_board]

    for mv in killer_order(genlegals(COLOR)):
        hit_piece, c_rights = make_move(mv)

        rec = quiescence(BLACK if COLOR == WHITE else WHITE, alpha, beta, quies - eval_quies(COLOR, mv, hit_piece), depth + 1)

        if ((rec > best) if COLOR == WHITE else (rec < best)):
            best = Evaluation(rec.value, [mv] + rec.moves)

        unmake_move(mv, hit_piece, c_rights)

        if depth == 0:
            print(mv, rec)
        # if board[mv[2]][mv[3]] & QUEEN or board[mv[2]][mv[3]] & KNIGHT or True:
        #     print('   '*depth, mv, rec, alpha.value, beta.value)

        # save eval if we come across it from another move order

        if COLOR == WHITE and (best > alpha):   alpha = best
        if COLOR == BLACK and (best < beta) :   beta  = best
        if beta <= alpha:
            KILLERHEURISTIC[mv] += depth ** 2
            break

    # genlegals was empty
    if best.moves == None:
        if king_not_in_check(COLOR):  # stalemate
            return Evaluation(0, [])
        return Evaluation(-inf+1 if COLOR == WHITE else +inf-1, [])

    TRANSPOSITIONS[hashable_board] = best

    if depth == 0:
        print('BBBB ', best)

    return best
    # deeply explored moves are better
    # DON'T ADD ANYTHING HERE! ab is based on the minmax-assumption
    #   so defying the max and sneaking in rogue unexplored moves






PIECEDIRS = {
    KNIGHT: ((1,2), (2,1), (-1,2), (2,-1), (-1,-2), (-2,-1), (1,-2), (-2,1)),
    BISHOP: ((1,1), (1,-1), (-1,1), (-1,-1)),
    ROOK: ((0,1), (1,0), (0,-1), (-1,0)),
    QUEEN: ((1,1), (1,-1), (-1,1), (-1,-1), (0,1), (1,0), (0,-1), (-1,0)),
    KING: ((1,1), (1,-1), (-1,1), (-1,-1), (0,1), (1,0), (0,-1), (-1,0)),
}

PIECERANGE = {
    KNIGHT: (1,),
    BISHOP: (1, 2, 3, 4, 5, 6, 7),
    ROOK: (1, 2, 3, 4, 5, 6, 7),
    QUEEN: (1, 2, 3, 4, 5, 6, 7),
    KING: (1,),
}

def king_not_in_check(COLOR):
    NCOLOR = BLACK if COLOR == WHITE else WHITE
    ADD = -1 if COLOR == WHITE else 1
    myking = KING + COLOR

    for i, j in gentuples:
        bij = board[i][j]
        bijpiece = bij & ~WHITE & ~BLACK
        if not bij & NCOLOR:
            continue
        if bij & PAWN:
            if is_inside(i-ADD, j+1) and board[i-ADD][j+1] == myking:
                return False
            if is_inside(i-ADD, j-1) and board[i-ADD][j-1] == myking:
                return False
        else:
            for a,b in PIECEDIRS[bijpiece]:
                for k in PIECERANGE[bijpiece]:
                    if is_inside(i+a*k, j+b*k):
                        if board[i+a*k][j+b*k] == myking:
                            return False
                        elif board[i+a*k][j+b*k]:
                            break  # important
                    else:
                        break

    return True

def genlegals(COLOR, ignore_king_capture=False):
    NCOLOR = BLACK if COLOR == WHITE else WHITE
    ADD = -1 if COLOR == WHITE else 1
    STARTRANK = 6 if COLOR == WHITE else 1
    PROMRANK = 1 if COLOR == WHITE else 6
    EPRANK = 3 if COLOR == WHITE else 5
    CASTLERANK = 7 if COLOR == WHITE else 0

    ret = []
    num_allowed_moves = collections.defaultdict(int)

    for i, j in gentuples:
        bij = board[i][j]
        bijpiece = bij & ~WHITE & ~BLACK
        if not bij & COLOR:
            continue
        if bij & PAWN:  # we can forego most is_inside checks here (promotion etc)
            lr = len(ret)
            if not board[i+ADD][j]:
                ret.append((i,j,i+ADD,j))
                if i == STARTRANK and not board[i+ADD+ADD][j]:
                    ret.append((i,j,i+ADD+ADD,j))
            if j < 7 and board[i+ADD][j+1] & NCOLOR:
                ret.append((i,j,i+ADD,j+1))
            if j > 0 and board[i+ADD][j-1] & NCOLOR:
                ret.append((i,j,i+ADD,j-1))
            if i == PROMRANK:  # promotion
                temp = []
                while lr < len(ret):
                    for p in ('n', 'b', 'r', 'q'):
                        temp.append(ret[lr] + (p,))
                    del ret[lr]
                ret += temp
            if i == EPRANK:  # en passant
                if LASTMOVE == (i+ADD+ADD, j-1, i, j-1):
                    ret.append((i, j, i+ADD, j-1, 'e'))
                if LASTMOVE == (i+ADD+ADD, j+1, i, j+1):
                    ret.append((i, j, i+ADD, j+1, 'e'))
        else:
            for a,b in PIECEDIRS[bijpiece]:
                for k in PIECERANGE[bijpiece]:
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

    if ((CASTLINGWHITE[0] and CASTLINGWHITE[1]) if COLOR == WHITE
    else (CASTLINGBLACK[0] and CASTLINGBLACK[1])):
        if not board[CASTLERANK][3] and not board[CASTLERANK][2] and not board[CASTLERANK][1]:
            print('castle left',  board[CASTLERANK][3] ,  board[CASTLERANK][2] ,  board[CASTLERANK][1] )
            board[CASTLERANK][2:4] = [COLOR + KING]*2
            if king_not_in_check(COLOR):
                ret.append((CASTLERANK, 4, CASTLERANK, 2, 'c'))
            board[CASTLERANK][2:4] = [0]*2
    if ((CASTLINGWHITE[2] and CASTLINGWHITE[1]) if COLOR == WHITE
    else (CASTLINGBLACK[2] and CASTLINGBLACK[1])):
        if not board[CASTLERANK][5] and not board[CASTLERANK][6]:
            print('castle right',  board[CASTLERANK][5] ,  board[CASTLERANK][6] )
            board[CASTLERANK][5:7] = [COLOR + KING]*2
            if king_not_in_check(COLOR):
                ret.append((CASTLERANK, 4, CASTLERANK, 6, 'c'))
            board[CASTLERANK][5:7] = [0]*2

    ret2 = []

    for mv in ret:
        hit_piece, c_rights = make_move(mv)
        assert ((not hit_piece & KING) if ignore_king_capture == False else True)

        if king_not_in_check(COLOR):
            ret2.append(mv)

        unmake_move(mv, hit_piece, c_rights)


    return ret2



def make_move(mv):

    global CASTLINGWHITE
    global CASTLINGBLACK

    piece = board[mv[0]][mv[1]]
    hit_piece = board[mv[2]][mv[3]]
    board[mv[0]][mv[1]] = 0
    board[mv[2]][mv[3]] = piece

    CASTLERANK = 7 if piece & WHITE else 0
    if len(mv) == 5:
        if mv[4] == 'e':
            board[mv[0]][mv[3]] = 0  # en passant
        elif mv[4] == 'c':
            print(mv, piece & WHITE, piece & BLACK, CASTLINGWHITE, CASTLINGBLACK)
            pprint()
            assert mv[0:2] == (CASTLERANK, 4)
            if mv[3] == 2:
                board[CASTLERANK][3] = board[CASTLERANK][0]
                board[CASTLERANK][0] = 0
            else:
                assert mv[3] == 6
                board[CASTLERANK][5] = board[CASTLERANK][7]
                board[CASTLERANK][7] = 0
            board[CASTLERANK][4] = 0
            pprint()
        else:
            board[mv[2]][mv[3]] = PIECECHARSINV[mv[4]] + (WHITE if mv[2] == 0 else BLACK)

    old_cr = CASTLINGWHITE, CASTLINGBLACK
    cr = (CASTLINGWHITE if piece & WHITE else CASTLINGBLACK)
    if mv[0] == CASTLERANK:
        cr = (mv[1] != 0 and cr[0],  mv[1] != 4 and cr[1],  mv[1] != 7 and cr[2])
        if piece & WHITE:   CASTLINGWHITE = cr
        else:               CASTLINGBLACK = cr

    return hit_piece, old_cr


def unmake_move(mv, hit_piece, c_rights):
    piece = board[mv[2]][mv[3]]
    board[mv[2]][mv[3]] = hit_piece
    board[mv[0]][mv[1]] = piece

    if len(mv) == 5:
        if mv[4] == 'e':
            board[mv[0]][mv[3]] = PAWN + (WHITE if mv[2] == 6 else BLACK)
        elif mv[4] == 'c':
            if mv[3] == 2:
                board[mv[0]][0] = board[mv[0]][3]
                board[mv[0]][3] = 0
            else:
                assert mv[3] == 6
                board[mv[0]][7] = board[mv[0]][5]
                board[mv[0]][5] = 0
        else:
            board[mv[0]][mv[1]] = PAWN + (WHITE if mv[2] == 0 else BLACK)

    CASTLINGWHITE = c_rights[0]
    CASTLINGBLACK = c_rights[1]



def make_move_str(mv):
    #XXX receiv castling
    print('recv move ' + mv)

    a = 8 - int(mv[1])
    b = ord(mv[0]) - ord('a')
    c = 8 - int(mv[3])
    d = ord(mv[2]) - ord('a')
    if len(mv) == 5:
        to_mv = (a, b, c, d, mv[4])
    elif b != d and board[a][b] & PAWN and not board[c][d]:  # en passant
        to_mv = (a, b, c, d, 'e')
    elif board[a][b] & KING and abs(d - b) > 1:
        to_mv = (a, b, c, d, 'c')
    else:
        to_mv = (a, b, c, d)

    print('parsed input as', to_mv)

    make_move(to_mv)
    print(to_mv)
    pprint()
    LASTMOVE = to_mv


def calc_move():
    if args.mode == 'random':
        legals = genlegals(WHITE if IM_WHITE else BLACK)
        if not legals:
            print('out of moves')
            pprint()
            return  # either stalemate or checkmate
        mv = random.choice(legals)

    else:
        ans = quiescence(WHITE if IM_WHITE else BLACK)
        if ans and ans.moves:
            mv = ans.moves[0]
        else:
            print('out of moves')
            pprint()
            return  # either stalemate or checkmate

    make_move(mv)
    LASTMOVE = mv

    return chr(ord('a') + mv[1]) + str(8 - mv[0]) + chr(ord('a') + mv[3]) + str(8 - mv[2]) + (mv[4] if len(mv) == 5 else '')









if args.test:
    import logic_tests

while True:
    i = input()
    ins = i.split(' ')[1:]

    # work both interactively AND with xboard
    if i == 'xboard':
        print('feature myname="janchess"')
        print('feature sigint=0 sigterm=0')  # else xboard sends KeyboardInterrupts
        print('feature done=1')

    elif i == 'force' or i.startswith('result'):
        started = False

    elif i == 'white':
        IM_WHITE = True
        board = new_board()

    elif i == 'new':
        IM_WHITE = False
        board = new_board()
        KILLERHEURISTIC = collections.defaultdict(int)

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
        print(mv)
        if mv:  print('move ' + mv)

    elif i.startswith('time'):
        total_time = int(ins[0])
        time_per_move = total_time / 100

    # answer pings
    elif i.startswith('ping'):
        print(i.replace('i', 'o'))

    elif i == 'quit' or i == 'q':
        print('quitted')
        pprint()
        logging.shutdown()
        exit()  # xboard waits for us on exit










