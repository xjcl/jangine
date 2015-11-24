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
                   help='random or minmax or quiescence (default)')

parser.add_argument('-t', '--test', action='store_true',
                   help='run tests???!!?')

args = parser.parse_args()





# XXX try to mate
# XXX ep
# XXX castling
# XXX ab-pruning
# XXX C-rewrite

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

_verbosity = 1  # depth of search to print
inf = 3000

# https://chessprogramming.wikispaces.com/Turochamp#Evaluation%20Features
def turochamp():
    eval = 0

    for i,j in gentuples():
        bijp = board[i][j] & ~WHITE & ~BLACK
        bij = board[i][j]
        eval += (1 if bij & WHITE else -1) * PIECEVALS[bijp]
        if bij & PAWN:
            eval += (-.1 * i if bij & WHITE else .1 * i)

    return eval



def minmax(COLOR, alpha=Evaluation(-inf+1, []), beta=Evaluation(+inf-1, []), depth=5):
    if not depth:   return Evaluation(turochamp(), [])

    best = Evaluation(-inf if COLOR == WHITE else inf, None)
    # if _verbosity > -1:  print(genlegals(COLOR))
    for mv in genlegals(COLOR):
        hit_piece = make_move(mv)
        rec = minmax(BLACK if COLOR == WHITE else WHITE, alpha, beta, depth - 1)
        # print((3 - depth) * '    ', mv, rec, alpha.value, beta.value)
        if ((rec > best) if COLOR == WHITE else (rec < best)):
            best = Evaluation(rec.value, [mv] + rec.moves)
        unmake_move(mv, hit_piece)

        if COLOR == WHITE and (best > alpha):   alpha = best
        if COLOR == BLACK and (best < beta) :   beta  = best
        if beta <= alpha:   break
    # if depth > 4: print('chose', best)

    if best.moves == None:
        return Evaluation(-inf+1 if COLOR == WHITE else +inf-1, [])

    return best


def eval_quies(COLOR, mv):
    NCOLOR = BLACK if COLOR == WHITE else WHITE

    quies = 30
    eval = 0

    if ((mv[0] > mv[2]) if COLOR == WHITE else (mv[0] < mv[2])):
        quies *= .6
    if ((mv[0] < mv[2]) if COLOR == WHITE else (mv[0] > mv[2])):
        quies *= 1.4

    if board[mv[2]][mv[3]] or len(mv) == 5:
        quies *= 0
        # we can't let that happen: else it calculates n-1 moves and then captures
        #   since it doesn't see the opponent's recapture, it evals it favorably

    hit_piece = make_move(mv)
    if not king_not_in_check(NCOLOR) or board[mv[0]][mv[1]] & KING:
        quies *= .3

    return hit_piece, quies, eval


def quiescence(COLOR, alpha=Evaluation(-inf+1, []), beta=Evaluation(+inf-1, []), quies=45, depth=0):

    if quies <= 0:   return Evaluation(turochamp(), [])

    best = Evaluation(-inf if COLOR == WHITE else inf, None)

    for mv in genlegals(COLOR):
        hit_piece, mv_quies, mv_eval = eval_quies(COLOR, mv)

        rec = quiescence(BLACK if COLOR == WHITE else WHITE, alpha, beta, quies - mv_quies, depth + 1)

        if ((rec > best) if COLOR == WHITE else (rec < best)):
            best = Evaluation(rec.value, [mv] + rec.moves)

        unmake_move(mv, hit_piece)

        if depth == 0:
            print(mv, rec)
        # if board[mv[2]][mv[3]] & QUEEN or board[mv[2]][mv[3]] & KNIGHT or True:
        #     print('   '*depth, mv, rec, alpha.value, beta.value)

        if COLOR == WHITE and (best > alpha):   alpha = best
        if COLOR == BLACK and (best < beta) :   beta  = best
        if beta <= alpha:   break

    # genlegals was empty
    if best.moves == None:
        return Evaluation(-inf+1 if COLOR == WHITE else +inf-1, [])


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

    for i, j in gentuples():
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

def genlegals(COLOR):
    NCOLOR = BLACK if COLOR == WHITE else WHITE
    ADD = -1 if COLOR == WHITE else 1
    STARTRANK = 6 if COLOR == WHITE else 1
    PROMRANK = 1 if COLOR == WHITE else 6
    EPRANK = 3 if COLOR == WHITE else 5

    ret = []

    for i, j in gentuples():
        bij = board[i][j]
        bijpiece = bij & ~WHITE & ~BLACK
        if not bij & COLOR:
            continue
        if bij & PAWN:
            lr = len(ret)
            if is_empty(i+ADD, j):
                ret.append((i,j,i+ADD,j))
                if is_empty(i+ADD+ADD, j) and i == STARTRANK:
                    ret.append((i,j,i+ADD+ADD,j))
            if is_inside(i+ADD, j+1) and board[i+ADD][j+1] & NCOLOR:
                ret.append((i,j,i+ADD,j+1))
            if is_inside(i+ADD, j-1) and board[i+ADD][j-1] & NCOLOR:
                ret.append((i,j,i+ADD,j-1))
            if i == PROMRANK:  # promotion
                temp = []
                while lr < len(ret):
                    for p in genpieces():
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
        if mv[4] == 'e':
            board[mv[0]][mv[3]] = 0
        else:
            board[mv[2]][mv[3]] = PIECEMAPINV[mv[4]] + (WHITE if mv[2] == 0 else BLACK)
    return hit_piece


def unmake_move(mv, hit_piece):
    piece = board[mv[2]][mv[3]]
    board[mv[2]][mv[3]] = hit_piece
    board[mv[0]][mv[1]] = piece
    if len(mv) == 5:
        if mv[4] == 'e':
            board[mv[0]][mv[3]] = PAWN + (WHITE if mv[2] == 6 else BLACK)
        else:
            board[mv[0]][mv[1]] = PAWN + (WHITE if mv[2] == 0 else BLACK)


def make_move_str(mv):
    print('recv move ' + mv)

    a = 8 - int(mv[1])
    b = LETTERMAPINV[mv[0]]
    c = 8 - int(mv[3])
    d = LETTERMAPINV[mv[2]]
    if len(mv) == 5:
        to_mv = (a, b, c, d, mv[4])
    elif b != d and board[a][b] & PAWN and not board[c][d]:  # en passant
        to_mv = (a, b, c, d, 'e')
    else:
        to_mv = (a, b, c, d)

    print('calc\'d move', to_mv)

    make_move(to_mv)
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
        mv = quiescence(WHITE if IM_WHITE else BLACK).moves[0]

    make_move(mv)
    LASTMOVE = mv

    # [print(row) for row in board]
    return LETTERMAP[mv[1]] + str(8 - mv[0]) + LETTERMAP[mv[3]] + str(8 - mv[2]) + (mv[4] if len(mv) == 5 else '')









if args.test:
    t0 = [
        [BLACK + KING, 0, 0, 0, WHITE + QUEEN, 0, 0, 0],
        [0] * 8,
        [0] * 8,
        [0] * 8,
        [0] * 8,
        [0] * 8,
        [0] * 8,
        [0] * 8,
    ]

    board = t0
    assert not king_not_in_check(BLACK)

    t1 = [
        [0] * 8,
        [0] * 8,
        [0] * 8,
        [0] * 8,
        [0, 0, 0, 0, 0, 0, 0, BLACK + QUEEN],
        [0] * 8,
        [0] * 8,
        [0, 0, 0, 0, WHITE + KING, 0, 0, 0],
    ]

    board = t1
    assert not king_not_in_check(WHITE)




    g0 =  [
        [0, BLACK + ROOK, BLACK + BISHOP, BLACK + QUEEN,
            BLACK + KING, BLACK + BISHOP, BLACK + KNIGHT, BLACK + ROOK],
        [BLACK + PAWN] * 4 + [0] + [BLACK + PAWN] * 3,
        [0] * 4 + [WHITE + PAWN] + [0] * 3,
        [0] * 8,
        [0, BLACK + KNIGHT, 0, 0, WHITE + PAWN] + [0] * 3,
        [0] * 8,
        [WHITE + PAWN] * 3 + [WHITE + BISHOP, 0] + [WHITE + PAWN] * 3,
        [WHITE + ROOK, WHITE + KNIGHT, 0, WHITE + QUEEN,
            WHITE + KING, WHITE + BISHOP, WHITE + KNIGHT, WHITE + ROOK],
    ]

    board = g0
    # find good enough move
    # assert (1, 5, 2, 4) in quiescence(BLACK)



    m0 =  [
        [0, BLACK + ROOK, BLACK + BISHOP, 0, BLACK + KING, 0, 0, BLACK + ROOK],
        [BLACK + PAWN] * 4 + [BLACK + QUEEN] + [BLACK + PAWN] * 3,
        [0] * 2 + [WHITE + PAWN] + [0] * 2 + [BLACK + KNIGHT] + [0] * 2,
        [0] * 8,
        [0, BLACK + BISHOP] + [0] * 6,
        [0, 0, WHITE + PAWN] + [0] * 6,
        [WHITE + PAWN] * 2 + [0] * 2 + [WHITE + QUEEN] + [WHITE + PAWN] * 3,
        [WHITE + ROOK, WHITE + KNIGHT, WHITE + BISHOP, 0,
            WHITE + KING, WHITE + BISHOP, WHITE + KNIGHT, WHITE + ROOK],
    ]

    board = m0

    # bestmove = quiescence(BLACK).moves[0]
    # print('bestmove', bestmove)

    # assert bestmove[0] == 5 and bestmove[1] == 2
    # with depth=3 this will be 'wrong' as it thinks pulling back
    #   the bishop will result in the loss of the Q

    # log_to_board("""INFO:root:>>> BR       BQ BK BB BN BR
    #     INFO:root:>>> BP BP BP    BP BP BP BP
    #     INFO:root:>>>
    #     INFO:root:>>>          WP
    #     INFO:root:>>>    BN             BB
    #     INFO:root:>>> WP    WN    WP
    #     INFO:root:>>>    WP WP WQ       WP WP
    #     INFO:root:>>> WR    WB    WK WB WN WR """)


    # bestmove = quiescence(BLACK).moves[0]
    # print('bestmove', bestmove)
    # assert bestmove == (4, 1, 2, 0)



    # log_to_board("""INFO:root:>>> BR    BB BQ BK BB BN BR
    #     INFO:root:>>> BP BP BP BP    BP BP BP
    #     INFO:root:>>>       BN    BP
    #     INFO:root:>>>
    #     INFO:root:>>>          WP WP
    #     INFO:root:>>>                WN
    #     INFO:root:>>> WP WP WP       WP WP WP
    #     INFO:root:>>> WR WN WB WQ WK WB    WR""")

    # bestmove = quiescence(BLACK).moves[0]
    # print('bestmove', bestmove)
    # assert bestmove != (2, 2, 3, 4)



    #     log_to_board("""INFO:root:>>> BR    BB BQ BK BB    BR
    # INFO:root:>>> BP    BP    BP BP BP BP
    # INFO:root:>>> BN BP    BP          BN
    # INFO:root:>>>
    # INFO:root:>>>       WB WP WP
    # INFO:root:>>>                WN
    # INFO:root:>>> WP WP WP       WP WP WP
    # INFO:root:>>> WR WN WB WQ       WK WR""")

    #     # for mv in genlegals(BLACK):
    #     #     hp = make_move(mv)
    #     #     for mv2 in genlegals(WHITE):
    #     #         hp2 = make_move(mv2)
    #     #         print(mv, mv2, turochamp())
    #     #         unmake_move(mv2, hp2)
    #     #     unmake_move(mv, hp)

    #     bestmove = quiescence(BLACK).moves[0]
    #     print('bestmove', bestmove)
    #     assert bestmove != (0, 2, 4, 6)

    # make_move((0, 2, 4, 6))

    # bestmove = quiescence(WHITE).moves[0]
    # print('bestmove', bestmove)
    # assert bestmove == (4, 2, 2, 0) or bestmove == (7, 2, 2, 7)

    # y u no see white's  (4,2,2,0)



    # make_move((0, 2, 4, 6))
    # make_move((4, 2, 5, 3))  # XXX
    # print(turochamp())




    # log_to_board("""INFO:root:>>> BR    BB BQ BK BB BN BR
    #     INFO:root:>>> BP BP BP BP BP BP BP BP
    #     INFO:root:>>>       BN
    #     INFO:root:>>>          WP
    #     INFO:root:>>>
    #     INFO:root:>>>
    #     INFO:root:>>> WP WP WP    WP WP WP WP
    #     INFO:root:>>> WR WN WB WQ WK WB WN WR""")

    # bestmove = quiescence(BLACK).moves[0]
    # print('bestmove', bestmove)


    log_to_board("""INFO:root:>>> BR    BB BQ BK BB BN BR
        INFO:root:>>> BP    BP BP BP BP BP BP
        INFO:root:>>> BP
        INFO:root:>>>
        INFO:root:>>>             WP
        INFO:root:>>>                WN
        INFO:root:>>> WP WP WP WP    WP WP WP
        INFO:root:>>> WR WN WB WQ WK       WR""")

    bestmove = quiescence(BLACK).moves[0]
    print('bestmove', bestmove)


    exit()




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

    elif i.startswith('time'):
        total_time = int(ins[0])
        time_per_move = total_time / 100

    # answer pings
    elif i.startswith('ping'):
        print(i.replace('i', 'o'))

    elif i == 'quit' or i == 'q':
        pprint()
        logging.shutdown()
        exit()










