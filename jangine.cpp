#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdbool>
#include <cstdint>
#include <cinttypes>

#include <unistd.h>

#include <iostream>
#include <tuple>
#include <string>
#include <map>


// typedef int_fast8_t int;
typedef int_fast16_t num;

bool in(char a, char const* list, size_t listlen) {
    for (int i = 0; i < listlen; ++i)
        if (a == list[i])
            return true;
    return false;
}

#define is_inside(i, j) (0 <= i <= 7 and 0 <= j <= 7)
#define input_is_move(i) (strlen(i) >= 4 and in(i[0], "abcdefgh", 8) and in(i[2], "abcdefgh", 8) and in(i[1], "12345678", 8) and in(i[3], "12345678", 8))
#define gentuples for (num i = 0; i < 8; ++i) for (num j = 0; j < 8; ++j)

FILE *f = NULL;

void tee(char const *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
}

num PAWN = 1, KNIGHT = 2, BISHOP = 4, ROOK = 8, QUEEN = 16, KING = 32, WHITE = 64, BLACK = 128;
num inf = 32000;

typedef struct CASTLINGRIGHTS {
    bool lr;
    bool k;
    bool rr;
} CASTLINGRIGHTS;

typedef struct Move {
    num f0;
    num f1;
    num t0;
    num t1;
    num prom;
} Move;

// typedef int_fast16_t[64] Board;

CASTLINGRIGHTS CASTLINGWHITE = {true, true, true};
CASTLINGRIGHTS CASTLINGBLACK = {true, true, true};

struct Pair {
    num a;
    num b;
};

std::map<num, num*> PIECERANGE;
std::map<num, Pair*> PIECEDIRS;

num board[64] = {0};   // malloc 64 or even 100
bool IM_WHITE = false;
bool started = true;
char* BUF = NULL;

void pprint() {
    for (num i = 0; i < 8; ++i) {
        for (num j = 0; j < 8; ++j)
            tee("%3d ", board[8*i+j]);
        tee(" \n");
    }
}

bool startswith(const char *pre, const char *str) {
    size_t lenpre = strlen(pre), lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

std::map<Move, num> KILLERHEURISTIC;

num default_board[64] = {
    BLACK + ROOK, BLACK + KNIGHT, BLACK + BISHOP, BLACK + QUEEN,
        BLACK + KING, BLACK + BISHOP, BLACK + KNIGHT, BLACK + ROOK,
    BLACK + PAWN, BLACK + PAWN, BLACK + PAWN, BLACK + PAWN,
        BLACK + PAWN, BLACK + PAWN, BLACK + PAWN, BLACK + PAWN,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    WHITE + PAWN, WHITE + PAWN, WHITE + PAWN, WHITE + PAWN,
        WHITE + PAWN, WHITE + PAWN, WHITE + PAWN, WHITE + PAWN,
    WHITE + ROOK, WHITE + KNIGHT, WHITE + BISHOP, WHITE + QUEEN,
        WHITE + KING, WHITE + BISHOP, WHITE + KNIGHT, WHITE + ROOK
};

void new_board() {
    for (int i = 0; i < 64; ++i)
        board[i] = default_board[i];
}

// gentuples = list(itertools.product(range(8), range(8)))

// def sq_to_str1(sq):
//     if sq & BLACK: return 'B'
//     if sq & WHITE: return 'W'
//     return ' '

// def sq_to_str2(sq):
//     for k, v in [(PAWN, 'P'), (KNIGHT, 'N'), (BISHOP, 'B'), (ROOK, 'R'), (QUEEN, 'Q'), (KING, 'K')]:
//         if sq & k: return v
//     return ' '

// def str_to_sq1(sq):
//     return {'B':BLACK, 'W':WHITE, ' ':0}[sq]

// def str_to_sq2(sq):
//     return {'P':PAWN, 'N':KNIGHT, 'B':BISHOP, 'R':ROOK, 'Q':QUEEN, 'K':KING, ' ':0}[sq]

// def pprint():
//     print()
//     for row in board:
//         print(' '.join(sq_to_str1(sq) + sq_to_str2(sq) for sq in row))
//     print()

// def log_to_board(s):
//     global board
//     board = [[], [], [], [], [], [], [], []]
//     for j, row in enumerate(s.split('\n')):
//         i = row.find('>>>') + 4
//         for k in range(8):
//             if i+3*k+1 >= len(row):  # subl trims trailing whitespace...
//                 board[j] += [0] * (8 - k)
//                 break
//             board[j].append(str_to_sq1(row[i+3*k]) + str_to_sq2(row[i+3*k+1]))

















struct Returner {
    num value;
    num move;
};






bool king_not_in_check(num COLOR) {}


num eval_quies(num COLOR, Move mv, num hit_piece) {
    num NCOLOR = (COLOR == WHITE ? BLACK : WHITE);

    num quies = 20;

    if (COLOR == WHITE ? mv.f0 > mv.t0 : mv.f0 < mv.t0)
        quies *= .8;
    if (COLOR == WHITE ? mv.f0 < mv.t0 : mv.f0 > mv.t0)
        quies *= 1.2;

    if (hit_piece or mv.prom != '\0' and mv.prom != 'c')
        quies *= 0;

    if (not king_not_in_check(NCOLOR) or board[8*mv.f0+mv.f1] & KING)
        quies *= 0;

    return quies;
}



int killer_cmp(const void* a, const void* b) {
    // XXX
    return true;
}

void killer_order(Move* mvs, size_t mvs_len) {
    qsort(mvs, mvs_len, sizeof(Move), killer_cmp);
}





Move calc_move(void) {
    // if args.mode == 'random':
    legals = genlegals(IM_WHITE ? WHITE : BLACK)
    if not legals:
        print('out of moves');
        pprint();
        return NULL;

    i = rand() % ...;
    mv = random.choice(legals);

    // else:
    //     ans = quiescence(WHITE if IM_WHITE else BLACK)
    //     if ans and ans.moves:
    //         mv = ans.moves[0]
    //     else:
    //         print('out of moves')
    //         pprint()
    //         return  # either stalemate or checkmate

    // make_move(mv)
    // LASTMOVE = mv

    // return chr(ord('a') + mv[1]) + str(8 - mv[0]) + chr(ord('a') + mv[3]) + str(8 - mv[2]) + (mv[4] if len(mv) == 5 else '')
}










num* range2(size_t count) {
    num* p = (num*)malloc(count * sizeof(num));
    for (int i = 0; i < count; ++i)
        p[i] = i+1;
    return p;
}

void init_data(void) {
    new_board();

    PIECEDIRS[ROOK] = (Pair*)malloc(4 * sizeof(Pair));
    PIECEDIRS[ROOK][0] = {0, 1};
    PIECEDIRS[ROOK][1] = {1, 0};
    PIECEDIRS[ROOK][2] = {0, -1};
    PIECEDIRS[ROOK][3] = {-1, 0};

    PIECEDIRS[BISHOP] = (Pair*)malloc(4 * sizeof(Pair));
    PIECEDIRS[BISHOP][0] = {1, 1};
    PIECEDIRS[BISHOP][1] = {1, -1};
    PIECEDIRS[BISHOP][2] = {-1, 1};
    PIECEDIRS[BISHOP][3] = {-1, -1};

    PIECEDIRS[KNIGHT] = (Pair*)malloc(8 * sizeof(Pair));
    PIECEDIRS[KNIGHT][0] = {1, 2};
    PIECEDIRS[KNIGHT][1] = {2, 1};
    PIECEDIRS[KNIGHT][2] = {-1, 2};
    PIECEDIRS[KNIGHT][3] = {2, -1};
    PIECEDIRS[KNIGHT][4] = {-1, -2};
    PIECEDIRS[KNIGHT][5] = {-2, -1};
    PIECEDIRS[KNIGHT][6] = {1, -2};
    PIECEDIRS[KNIGHT][7] = {-2, 1};

    PIECEDIRS[QUEEN] = (Pair*)malloc(8 * sizeof(Pair));
    PIECEDIRS[QUEEN][0] = {0, 1};
    PIECEDIRS[QUEEN][1] = {1, 0};
    PIECEDIRS[QUEEN][2] = {0, -1};
    PIECEDIRS[QUEEN][3] = {-1, 0};
    PIECEDIRS[QUEEN][4] = {1, 1};
    PIECEDIRS[QUEEN][5] = {1, -1};
    PIECEDIRS[QUEEN][6] = {-1, 1};
    PIECEDIRS[QUEEN][7] = {-1, -1};

    PIECEDIRS[KING] = PIECEDIRS[QUEEN];

    PIECERANGE[KNIGHT] = range2(1);
    PIECERANGE[BISHOP] = range2(7);
    PIECERANGE[ROOK] = range2(7);
    PIECERANGE[QUEEN] = range2(7);
    PIECERANGE[KING] = range2(1);
}



int main(int argc, char const *argv[])
{
    f = fopen("jangine.log", "w");

    setbuf(stdout, NULL);
    init_data();

    char* line = (char*)calloc(1024, 1);
    tee(line);

    while (true) {

        fgets(line, 1023, stdin);

        if (strcmp(line, "xboard\n") == 0) {
            tee("feature myname=\"jangine\"\n");
            tee("feature sigint=0 sigterm=0\n");
            tee("feature done=1\n");
        }

        // if (strcmp(line, "force\n") == 0) {
        //     started = false;
        // }

        if (input_is_move(line) and started) {
            // make_move_str(line);
            // Move mv = calc_move();
            tee("move e7e5\n");
        }


        if (startswith("ping", line)) {
            line[1] = 'o';
            tee(line);
        }


        if (strcmp(line, "quit\n") == 0 or strcmp(line, "q\n") == 0) {
            pprint();
            fclose(f);
            return 0;
        }
    }

    return 0;
}

// while __name__ == '__main__' and True:
//     i = input()
//     ins = i.split(' ')[1:]

//     elif i == 'force' or i.startswith('result'):
//         started = False

//     elif i == 'white':
//         IM_WHITE = True
//         board = new_board()

//     elif i == 'new':
//         IM_WHITE = False
//         board = new_board()
//         KILLERHEURISTIC = collections.defaultdict(int)

//     elif input_is_move(i) and not started:
//         BUF = i

//     elif input_is_move(i) and started:
//         make_move_str(i)
//         mv = calc_move()
//         if mv:  print('move ' + mv)

//     # go/force stuff needed, else xboard skips second move
//     elif i == 'go':
//         started = True
//         if BUF:
//             make_move_str(BUF)
//             BUF = ''
//         mv = calc_move()
//         print(mv)
//         if mv:  print('move ' + mv)

//     elif i.startswith('time'):
//         total_time = int(ins[0])
//         time_per_move = total_time / 100








/*

Evaluation = collections.namedtuple('Evaluation', 'value moves')
Evaluation.__eq__ = lambda self, other: self.value == other.value
Evaluation.__lt__ = lambda self, other: self.value < other.value
Evaluation.__gt__ = lambda self, other: self.value > other.value
Evaluation.__le__ = lambda self, other: self < other or self == other
Evaluation.__add__ = lambda self, other: Evaluation(self.value + other, self.moves)


# https://chessprogramming.wikispaces.com/Turochamp#Evaluation%20Features
def turochamp(loud=False):

    eval = 0

    for i,j in gentuples:
        # material counting
        bijp = board[i][j] & ~WHITE & ~BLACK
        bij = board[i][j]
        MUL = 1 if bij & WHITE else -1

        eval += MUL * PIECEVALS[bijp]

        # king safety
        # if bijp == KING:
        #     i = 0
        #     for a,b in PIECEDIRS[QUEEN]:
        #         for k in PIECERANGE[QUEEN]:
        #             if not is_inside(i+a*k, j+b*k) or board[i+a*k][j+b*k]:
        #                 break
        #             # eval -= MUL * 8
        #             i += 1
        #     eval -= MUL * 350 * round(i**.5)

        # pawn advancement
        # if bij & PAWN:
        #     eval += 8 * (6-i if bij & WHITE else 1-i)

    if loud:
        print('pieces and pawns', eval)

    for i,j in gentuples:
        bijp = board[i][j] & ~WHITE & ~BLACK
        bij = board[i][j]
        MUL = 1 if bij & WHITE else -1
        if bijp == KING:
            c = 0
            for a,b in PIECEDIRS[QUEEN]:
                for k in PIECERANGE[QUEEN]:
                    if not is_inside(i+a*k, j+b*k) or board[i+a*k][j+b*k]:
                        break
                    # eval -= MUL * 8
                    c += 1
            eval -= MUL * 30 * round(c**.5)

    if loud:
        print('king safety', eval)

    # center control
    center = [board[3][3], board[3][4], board[4][3], board[4][4]]
    eval += 8 * len([x for x in center if x & WHITE and not x & QUEEN and not x & ROOK])
    eval -= 8 * len([x for x in center if x & BLACK and not x & QUEEN and not x & ROOK])

    if loud:
        print('center', eval)

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


    if loud: print('mobility', eval)

    # eval += 30 if (CASTLINGWHITE[1] and (CASTLINGWHITE[0] and not board[7][1] and not board[7][2] \
    #     and not board[7][3] or CASTLINGWHITE[2] and not board[7][5] and not board[7][6])) else 0
    # eval -= 30 if (CASTLINGBLACK[1] and (CASTLINGBLACK[0] and not board[0][1] and not board[0][2] \
    #     and not board[0][3] or CASTLINGBLACK[2] and not board[0][5] and not board[0][6])) else 0

    if loud: print('castling', eval)

    return eval





def quiescence(COLOR, alpha=Evaluation(-inf+1, []), beta=Evaluation(+inf-1, []), quies=41, depth=0, lastmv=()):

    global TRANSPOSITIONS

    # hashable_board = tuple(tuple(row) for row in board) + (COLOR,)  # COLOR! else it might 'go twice'
    # if hashable_board in TRANSPOSITIONS:
    #     return TRANSPOSITIONS[hashable_board]

    if quies <= 0 or depth > 7:     return Evaluation(turochamp(), [])
    if depth == 0:
        print(len(TRANSPOSITIONS))
        print(CASTLINGWHITE)
        TRANSPOSITIONS = dict()  # print: bursts the pipe lol

    best = Evaluation(-inf if COLOR == WHITE else inf, None)

    if depth == 0:
        print(genlegals(BLACK))
        print(genlegals(COLOR))
        print(killer_order(genlegals(COLOR)))

    for mv in killer_order(genlegals(COLOR)):

        hit_piece, c_rights = make_move(mv)

        rec = quiescence(BLACK if COLOR == WHITE else WHITE, alpha, beta, quies - eval_quies(COLOR, mv, hit_piece), depth + 1, mv)

        unmake_move(mv, hit_piece, c_rights)

        if ((rec > best) if COLOR == WHITE else (rec < best)):
            best = Evaluation(rec.value, [mv] + rec.moves)

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

    # TRANSPOSITIONS[hashable_board] = best

    if depth == 0:
        print('BBBB ', best)

    return best
    # deeply explored moves are better
    # DON'T ADD ANYTHING HERE! ab is based on the minmax-assumption
    #   so defying the max and sneaking in rogue unexplored moves






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
    EPRANK = 3 if COLOR == WHITE else 4
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
                    print('epadd')
                    ret.append((i, j, i+ADD, j-1, 'e'))
                if LASTMOVE == (i+ADD+ADD, j+1, i, j+1):
                    print('epadd')
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
            board[CASTLERANK][2:4] = [COLOR + KING]*2
            if king_not_in_check(COLOR):
                ret.append((CASTLERANK, 4, CASTLERANK, 2, 'c'))
            board[CASTLERANK][2:4] = [0]*2
    if ((CASTLINGWHITE[2] and CASTLINGWHITE[1]) if COLOR == WHITE
    else (CASTLINGBLACK[2] and CASTLINGBLACK[1])):
        if not board[CASTLERANK][5] and not board[CASTLERANK][6]:
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
            assert mv[0:2] == (CASTLERANK, 4)
            if mv[3] == 2:
                board[CASTLERANK][3] = board[CASTLERANK][0]
                board[CASTLERANK][0] = 0
            else:
                assert mv[3] == 6
                board[CASTLERANK][5] = board[CASTLERANK][7]
                board[CASTLERANK][7] = 0
            board[CASTLERANK][4] = 0
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

    global CASTLINGWHITE
    global CASTLINGBLACK

    piece = board[mv[2]][mv[3]]
    board[mv[2]][mv[3]] = hit_piece
    board[mv[0]][mv[1]] = piece

    if len(mv) == 5:
        if mv[4] == 'e':
            board[mv[0]][mv[3]] = PAWN + (WHITE if mv[2] == 5 else BLACK)
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







*/
