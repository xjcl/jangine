#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdbool>
#include <cstdint>
#include <cinttypes>
#include <cmath>
#include <unistd.h>
#include <map>


// typedef int_fast8_t int;
typedef int_fast16_t num;

bool in(char a, char const* list, size_t listlen) {
    for (size_t i = 0; i < listlen; ++i)
        if (a == list[i])
            return true;
    return false;
}

#define is_inside(i, j) (0 <= i and i <= 7 and 0 <= j and j <= 7)
#define input_is_move(i) (strlen(i) >= 4 and in(i[0], "abcdefgh", 8) and in(i[2], "abcdefgh", 8) and in(i[1], "12345678", 8) and in(i[3], "12345678", 8))
#define gentuples for (num i = 0; i < 8; ++i) for (num j = 0; j < 8; ++j)
#define store(a, b, c, d, e) \
    { *mvsend = (Move*)malloc(sizeof(Move)); \
    Move x = {a, b, c, d, e}; \
    memcpy(*mvsend, &x, sizeof(Move)); \
    mvsend++; }

FILE *f = NULL;

void tee(char const *fmt, ...) {
    // f = fopen("jangine.log", "a");

    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);

    // fclose(f);
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

    // needed for std::map
    bool operator<( const Move & that ) const {
        return 8*this->f0 + this->f1 < 8*that.f0 + that.f1;
    }
} Move;

typedef struct PiecePlusCatling {
    num hit_piece;
    CASTLINGRIGHTS c_rights_w;
    CASTLINGRIGHTS c_rights_b;
} PiecePlusCatling;

// typedef int_fast16_t[64] Board;

CASTLINGRIGHTS CASTLINGWHITE = {true, true, true};
CASTLINGRIGHTS CASTLINGBLACK = {true, true, true};

Move LASTMOVE = {0};

struct Pair {
    num a;
    num b;
};

Pair** PIECEDIRS = NULL;
num* PIECERANGE = NULL;
num* PIECEVALS = NULL;

num board[64] = {0};   // malloc 64 or even 100
bool IM_WHITE = false;
bool started = true;

int64_t NODES = 0;

void pprint() {
    for (num i = 0; i < 8; ++i) {
        for (num j = 0; j < 8; ++j)
            tee("%3d ", board[8*i+j]);
        tee(" \n");
    }
    tee(" \n");
}

bool startswith(const char *pre, const char *str) {
    size_t lenpre = strlen(pre), lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

std::map<Move, int_fast32_t> KILLERHEURISTIC;

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

typedef struct ValuePlusMoves {
    num value;
    Move** moves;
    Move** movesend;
} ValuePlusMoves;

typedef struct ValuePlusMove {
    num value;
    Move move;
} ValuePlusMove;


PiecePlusCatling make_move(Move mv) {

    if (mv.f0 == 0 and mv.f1 == 0 and mv.t0 == 0 and mv.t1 == 0)
        tee("XXX DANGEROUS! NULL MOVE");

    num piece = board[8*mv.f0+mv.f1];
    num hit_piece = board[8*mv.t0+mv.t1];
    board[8*mv.f0+mv.f1] = 0;
    board[8*mv.t0+mv.t1] = piece;

    num CASTLERANK = piece & WHITE ? 7 : 0;

    // tee("start");

    if (mv.prom) {
        if (mv.prom == 'e') {
            board[8*mv.f0+mv.t1] = 0;
        } else if (mv.prom == 'c') {
            if (mv.t1 == 2) {
                board[8*CASTLERANK+3] = board[8*CASTLERANK];
                board[8*CASTLERANK] = 0;
            } else {
                board[8*CASTLERANK+5] = board[8*CASTLERANK+7];
                board[8*CASTLERANK+7] = 0;
            }
            board[8*CASTLERANK+4] = 0;
        } else {
            num p;
            switch (mv.prom) {
                case 'q':  p = QUEEN;   break;
                case 'r':  p = ROOK;    break;
                case 'b':  p = BISHOP;  break;
                case 'n':  p = KNIGHT;  break;
            }
            board[8*mv.t0+mv.t1] = p + (mv.t0 == 0 ? WHITE : BLACK);
        }
    }

    // tee("promotion done");

    CASTLINGRIGHTS old_cr_w = CASTLINGWHITE;
    CASTLINGRIGHTS old_cr_b = CASTLINGBLACK;

    CASTLINGRIGHTS cr = (piece & WHITE ? CASTLINGWHITE : CASTLINGBLACK);

    if (mv.f0 == CASTLERANK) {
        cr = {mv.f1 != 0 and cr.lr, mv.f1 != 4 and cr.k,  mv.f1 != 7 and cr.rr};
        if (piece & WHITE)  CASTLINGWHITE = cr;
        else                CASTLINGBLACK = cr;
    }

    // XXX len-5-moves
    return {hit_piece, old_cr_w, old_cr_b};
}



void unmake_move(Move mv, num hit_piece, CASTLINGRIGHTS c_rights_w, CASTLINGRIGHTS c_rights_b) {

    num piece = board[8*(mv.t0)+mv.t1];
    board[8*(mv.t0)+mv.t1] = hit_piece;
    board[8*(mv.f0)+mv.f1] = piece;

    if (mv.prom)
    {
        if (mv.prom == 'e') {
            board[8*mv.f0+mv.t1] = PAWN + (mv.t0 == 5 ? WHITE : BLACK);
        } else if (mv.prom == 'c') {
            if (mv.t1 == 2) {
                board[8*mv.f0] = board[8*mv.f0+3];
                board[8*mv.f0+3] = 0;
            }
            else {
                board[8*mv.f0+7] = board[8*mv.f0+5];
                board[8*mv.f0+5] = 0;
            }
        } else {
            board[8*mv.f0+mv.f1] = PAWN + (mv.t0 == 0 ? WHITE : BLACK);
        }
    }

    CASTLINGWHITE = c_rights_w;
    CASTLINGBLACK = c_rights_b;
}



void make_move_str(const char* mv) {

    num a = 8 - (mv[1] - '0');
    num b = (num)(mv[0] - 'a');
    num c = 8 - (mv[3] - '0');
    num d = (num)(mv[2] - 'a');

    char e = mv[4];
    if (abs(d - b) > 1 and board[8*a+b] & KING)  // castling
        e = 'c';
    if (b != d and board[8*a+b] & PAWN and not board[8*c+d])  // en passant
        e = 'e';
    if (e == '\n')
        e = '\0';

    Move to_mv = {a, b, c, d, e};
    make_move(to_mv);
    LASTMOVE = to_mv;  // copies the struct(?)
}



bool king_not_in_check(num COLOR) {
    num NCOLOR = COLOR == WHITE ? BLACK : WHITE;
    num ADD = COLOR == WHITE ? -1 : 1;
    num myking = KING + COLOR;

    gentuples {
        num bij = board[8*i+j];
        num bijpiece = bij & ~WHITE & ~BLACK;
        if (not (bij & NCOLOR))
            continue;
        if (bij & PAWN) {
            if (is_inside(i-ADD, j+1) and board[8*(i-ADD)+j+1] == myking)
                return false;
            if (is_inside(i-ADD, j-1) and board[8*(i-ADD)+j-1] == myking)
                return false;
        } else {
            for (num l = 0; PIECEDIRS[bijpiece][l].a != 0 or PIECEDIRS[bijpiece][l].b != 0; ++l) {
                num a = PIECEDIRS[bijpiece][l].a;
                num b = PIECEDIRS[bijpiece][l].b;
                for (num k = 1; k <= PIECERANGE[bijpiece]; ++k)
                {
                    if (is_inside(i+a*k, j+b*k)) {
                        // tee("%d %d %d %d\n", (i+a*k), j+b*k, board[8*(i+a*k)+j+b*k], myking);
                        if (board[8*(i+a*k)+j+b*k] == myking)
                            return false;
                        if (board[8*(i+a*k)+j+b*k])
                            break;
                    } else {
                        break;
                    }
                }
            }
        }
    }

    return true;
}



ValuePlusMoves genlegals(num COLOR) {

    Move** mvs = (Move**)calloc(128, sizeof(Move*));  // 9 Qs have a lot of moves...
        // also inits to NULLptrs
    Move** mvsend = mvs;

    num NCOLOR = COLOR == WHITE ? BLACK : WHITE;
    num ADD = COLOR == WHITE ? -1 : 1;
    num STARTRANK = COLOR == WHITE ? 6 : 1;
    num PROMRANK = COLOR == WHITE ? 1 : 6;
    num EPRANK = COLOR == WHITE ? 3 : 4;
    num CASTLERANK = COLOR == WHITE ? 7 : 0;

    gentuples {
        num bij = board[8*i+j];
        num bijpiece = bij & ~WHITE & ~BLACK;
        if (not (bij & COLOR))
            continue;
        if (bij & PAWN) {
            // lr = len(ret)
            Move** badmvs = mvsend;

            if (not board[8*(i+ADD)+j]) {
                store(i, j, i+ADD, j, '\0');
                if (i == STARTRANK and not board[8*(i+ADD+ADD)+j]) {
                    store(i, j, i+ADD+ADD, j, '\0');
                }
            }
            if (j < 7 and board[8*(i+ADD)+j+1] & NCOLOR)
                store(i, j, i+ADD, j+1, '\0');
            if (j > 0 and board[8*(i+ADD)+j-1] & NCOLOR)
                store(i, j, i+ADD, j-1, '\0');

            if (i == PROMRANK) {
                Move** mvsendcpy = mvsend;
                while (mvsendcpy > badmvs) {
                    mvsendcpy--;
                    // store(*mvsendcpy->f0, *mvsendcpy->f1, *mvsendcpy->t0, *mvsendcpy->t1, 'n');
                    // store(*mvsendcpy->f0, *mvsendcpy->f1, *mvsendcpy->t0, *mvsendcpy->t1, 'b');
                    // store(*mvsendcpy->f0, *mvsendcpy->f1, *mvsendcpy->t0, *mvsendcpy->t1, 'r');
                    *mvsend = (Move*)malloc(sizeof(Move)); memcpy(*mvsend, *mvsendcpy, sizeof(Move));  (*mvsend)->prom = 'n';  mvsend++;
                    *mvsend = (Move*)malloc(sizeof(Move)); memcpy(*mvsend, *mvsendcpy, sizeof(Move));  (*mvsend)->prom = 'b';  mvsend++;
                    *mvsend = (Move*)malloc(sizeof(Move)); memcpy(*mvsend, *mvsendcpy, sizeof(Move));  (*mvsend)->prom = 'r';  mvsend++;
                    (*mvsendcpy)->prom = 'q';
                }
            }

            if (i == EPRANK) {
                if (LASTMOVE.f0 == i+ADD+ADD and LASTMOVE.f1 == j-1 and LASTMOVE.t0 == i and LASTMOVE.t1 == j-1) {
                    store(i, j, i+ADD, j-1, '\0');
                }
                if (LASTMOVE.f0 == i+ADD+ADD and LASTMOVE.f1 == j+1 and LASTMOVE.t0 == i and LASTMOVE.t1 == j+1) {
                    store(i, j, i+ADD, j+1, '\0');
                }
            }
        }
        else {
            for (num l = 0; PIECEDIRS[bijpiece][l].a != 0 or PIECEDIRS[bijpiece][l].b != 0; ++l) {
                num a = PIECEDIRS[bijpiece][l].a;
                num b = PIECEDIRS[bijpiece][l].b;
                for (num k = 1; k <= PIECERANGE[bijpiece]; ++k)
                {
                    if (is_inside(i+a*k, j+b*k)) {
                        if (not board[8*(i+a*k)+j+b*k]) {
                            // tee("%d%d%d%d %d%d\n", i, j, i+a*k, j+b*k, a, b);
                            store(i, j, i+a*k, j+b*k, '\0');
                        } else if (board[8*(i+a*k)+j+b*k] & NCOLOR) {
                            // tee("%d%d%d%d %d%d\n", i, j, i+a*k, j+b*k, a, b);
                            store(i, j, i+a*k, j+b*k, '\0');
                            break;
                        } else if (board[8*(i+a*k)+j+b*k] & COLOR) {
                            break;
                        }
                    } else {
                        break;
                    }
                }
            }
        }
    }

    // XXX castling


    // if ((CASTLINGWHITE[0] and CASTLINGWHITE[1]) if COLOR == WHITE
    // else (CASTLINGBLACK[0] and CASTLINGBLACK[1])):
    //     if not board[CASTLERANK][3] and not board[CASTLERANK][2] and not board[CASTLERANK][1]:
    //         board[CASTLERANK][2:4] = [COLOR + KING]*2
    //         if king_not_in_check(COLOR):
    //             ret.append((CASTLERANK, 4, CASTLERANK, 2, 'c'))
    //         board[CASTLERANK][2:4] = [0]*2
    // if ((CASTLINGWHITE[2] and CASTLINGWHITE[1]) if COLOR == WHITE
    // else (CASTLINGBLACK[2] and CASTLINGBLACK[1])):
    //     if not board[CASTLERANK][5] and not board[CASTLERANK][6]:
    //         board[CASTLERANK][5:7] = [COLOR + KING]*2
    //         if king_not_in_check(COLOR):
    //             ret.append((CASTLERANK, 4, CASTLERANK, 6, 'c'))
    //         board[CASTLERANK][5:7] = [0]*2



    Move** mvscpy = mvs;
    while (mvscpy != mvsend) {
        PiecePlusCatling ppc = make_move(**mvscpy);
        bool illegal = !king_not_in_check(COLOR);
        unmake_move(**mvscpy, ppc.hit_piece, ppc.c_rights_w, ppc.c_rights_b);

        if (illegal) {
            *mvscpy = NULL;
        }

        mvscpy++;
    }

    // remove trailing NULLs
    mvsend--;
    while (!(*mvsend) and mvsend > mvs) {
        mvsend--;
    }

    // we can't -1 if size is 0
    if (!(*mvsend))
        return {0, mvs, mvs};

    mvsend++;

    return {0, mvs, mvsend};
}



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

std::string move_to_str(Move mv) {
    char c0 = 'a' + (char) (mv.f1);
    char c1 = '0' + (char) (8 - mv.f0);
    char c2 = 'a' + (char) (mv.t1);
    char c3 = '0' + (char) (8 - mv.t0);
    char c4 = (char)mv.prom;
    std::string ret{c0, c1, c2, c3, c4, '\0'};
    return ret;
}

void tee_move(Move mv) {

    num kh;
    auto it = KILLERHEURISTIC.find(mv);
    if (it == KILLERHEURISTIC.end()) {
        kh = 0;
    }
    else {
        kh = KILLERHEURISTIC[mv];
    }

    std::string move_str = move_to_str(mv);
    tee("MOVE %5s (KH %8d)", move_str.c_str(), kh);
}

void tee_moves(Move** mvs, num count) {
    for (int i = 0; i < count; ++i)
    {
        if (mvs[i] != NULL) {
            tee("%d ", i);
            tee_move(*(mvs[i]));
            tee("\n");
        }
        else{
            tee("MISSING\n");
        }
    }
}

// https://chessprogramming.wikispaces.com/Turochamp#Evaluation%20Features
num turochamp(num depth) {

    num eval = 0;

    gentuples {
        // material counting
        num bijp = board[8*i+j] & ~WHITE & ~BLACK;
        num bij = board[8*i+j];
        num MUL = bij & WHITE ? 1 : -1;

        eval += MUL * PIECEVALS[bijp];

        // king safety
        if (bijp == KING) {
            num c = 0;
            for (num l = 0; PIECEDIRS[QUEEN][l].a != 0 or PIECEDIRS[QUEEN][l].b != 0; ++l) {
                num a = PIECEDIRS[QUEEN][l].a;
                num b = PIECEDIRS[QUEEN][l].b;
                for (num k = 1; k <= PIECERANGE[QUEEN]; ++k)
                {
                    if (not is_inside(i+a*k, j+b*k) or board[8*(i+a*k)+j+b*k])
                        break;
                    ++c;
                }
            }
            eval -= MUL * 30 * round(sqrt(c));
        }

        // pawn advancement
        if (bij & PAWN)
            eval += 8 * (bij & WHITE ? 6-i : 1-i);
    }

    // center control
    for (int i = 3; i < 5; ++i)
        for (int j = 3; j < 5; ++j)
        {
            num bij = board[8*i+j];
            if (bij and not(bij & ROOK) and not(bij & QUEEN))
                eval += 8 * (bij & WHITE ? 1 : -1);
        }

    // mobility
    for (num MUL = -1; MUL < 2; MUL += 2)
    {
        num COLOR = (MUL == 1 ? WHITE : BLACK);

        ValuePlusMoves gl = genlegals(COLOR);

        num mvs_len = ((num)gl.movesend - (num)gl.moves) / sizeof(Move*);
        if (!mvs_len) {
            if (king_not_in_check(COLOR))
                return 0;
            return MUL * (-inf+20+depth);
        }

        for (num j = 0; j < mvs_len; ++j)
        {
            if (gl.moves[j] == NULL)
                continue;
            // tee("%p ", gl.moves[j]);
            Move ref_mv = *(gl.moves[j]);
            num i = 0;
            if (not (board[8*ref_mv.f0+ref_mv.f1] & QUEEN)) {
                while (true) {
                    if (gl.moves[j] == NULL) {
                        ++j;
                        continue;
                    }
                    if (not(j < mvs_len and (*(gl.moves[j])).f0 == ref_mv.f0 and (*(gl.moves[j])).f1 == ref_mv.f1))
                        break;
                    ++i;
                    if (board[8* (*(gl.moves[j])).t0 + (*(gl.moves[j])).t1])
                        ++i;
                    ++j;
                }
            }
            eval += MUL * 13 * round(sqrt(i));
        }


        for (int i = 0; i < 128; ++i)
            if (gl.moves[i])
                free(gl.moves[i]);
        free(gl.moves);
    }

    return eval;
}



// better move is smaller (since lists are sorted small->large)

int killer_cmp(const void* a, const void* b) {
    // we don't care

    // XXX what wastes more time
    //  swapping NULLs  or
    //  not getting advantages in quiescence from branch predicting

    Move **a_fuck_cpp = (Move **)a;
    Move **b_fuck_cpp = (Move **)b;
    if (!*b_fuck_cpp) {
        return -1;
    }
    if (!*a_fuck_cpp) {
        return 1;
    }

    Move mva = **(a_fuck_cpp);
    Move mvb = **(b_fuck_cpp);

    auto ita = KILLERHEURISTIC.find(mva);
    auto itb = KILLERHEURISTIC.find(mvb);
    if (ita == KILLERHEURISTIC.end()) {
        if (itb == KILLERHEURISTIC.end())
            return 0;
        return 1;
    }
    if (itb == KILLERHEURISTIC.end())
        return -1;
    return KILLERHEURISTIC[mvb] - KILLERHEURISTIC[mva];
}


ValuePlusMove quiescence(num COLOR, num alpha, num beta, num quies, num depth, num noise) {

    NODES += 1;

    if (quies <= 0 or depth > 5)
        return {turochamp(depth), {0}};

    ValuePlusMove best = {COLOR == WHITE ? -inf : inf, {0}};

    ValuePlusMoves gl = genlegals(COLOR);
    Move** gl_moves_backup = gl.moves;

    num mvs_len = ((num)gl.movesend - (num)gl.moves) / sizeof(Move*);

    if (!mvs_len) {
        if (king_not_in_check(COLOR))
            return {0, {0}};
        return {(COLOR == WHITE ? 1 : -1) * (-inf+20+depth), {0}};
    }

    // tee("BEFORE ALL\n");
    // tee(" mvs_len: %d\n", mvs_len);
    // tee_moves(gl.moves, mvs_len);
    qsort(gl.moves, mvs_len, sizeof(Move*), killer_cmp);
    // tee_moves(gl.moves, mvs_len);
    // tee("AFTER QSORT\n");

    while (gl.moves != gl.movesend) {

        if (!*(gl.moves)) {
            gl.moves++;
            continue;
        }

        Move mv = **(gl.moves);

        PiecePlusCatling ppc = make_move(mv);

        ValuePlusMove rec = quiescence(
            COLOR == WHITE ? BLACK : WHITE,
            alpha,
            beta,
            quies - eval_quies(COLOR, mv, ppc.hit_piece), depth + 1,
            false
        );

        unmake_move(mv, ppc.hit_piece, ppc.c_rights_w, ppc.c_rights_b);

        if (COLOR == WHITE ? rec.value > best.value : rec.value < best.value)
            best = {rec.value, mv};


        if (depth == 0) {
            tee_move(mv);
            tee(" EVAL %5d\n", rec.value);
        }


        if (COLOR == WHITE and best.value > alpha)
            alpha = best.value;
        if (COLOR == BLACK and best.value < beta)
            beta = best.value;
        if (beta <= alpha) {
            // XXX KILLER
            auto it = KILLERHEURISTIC.find(mv);
            if (it == KILLERHEURISTIC.end()) {
                KILLERHEURISTIC[mv] = 0;
            }
            KILLERHEURISTIC[mv] += depth * depth;
            break;
        }

        gl.moves++;
    }


    for (int i = 0; i < 128; ++i)
        if (gl_moves_backup[i])
            free(gl_moves_backup[i]);
    free(gl_moves_backup);

    return best;
}



std::string calc_move(void) {
    // if args.mode == 'random':
    // ValuePlusMoves* legals = genlegals(IM_WHITE ? WHITE : BLACK);
    // if (legals->moves == legals->movesend) {
    //     tee("out of moves");
    //     pprint();
    //     return NULL;
    // }

    // num i = rand() % (((num)legals->movesend - (num)legals->moves)/sizeof(num*));
    // while ( !(legals->moves[i]) ) {
    //     i = rand() % (((num)legals->movesend - (num)legals->moves)/sizeof(num*));
    // }

    // Move mv = {0};
    // memcpy(&mv, legals->moves[i], sizeof(Move));
    // LASTMOVE = mv;
    // make_move(legals->moves[i]);

    // while (legals->moves != legals->movesend) {
    //     free(*legals->moves);
    //     legals->moves++;
    // }
    // free(legals);


    // quiescence
    // TODO: what happens if no moves (stalemate/checkmate)? Null?
    tee("Starting quiescence with depth 5\n");
    ValuePlusMove bestmv = quiescence(IM_WHITE ? WHITE : BLACK, -inf+1, inf-1, 41, 0, true);
    Move mv = bestmv.move;
    tee("BEST ");
    tee_move(mv);
    tee("\n");
    make_move(mv);
    tee("SEARCHED %d NODES", NODES);

    return move_to_str(mv);
}



void init_data(void) {
    new_board();

    PIECEDIRS = (Pair**)calloc(65, sizeof(Pair*));
    PIECERANGE = (num*)calloc(65, sizeof(num));
    PIECEVALS = (num*)calloc(65, sizeof(num));

    PIECEDIRS[ROOK] = (Pair*)malloc(5 * sizeof(Pair));
    PIECEDIRS[ROOK][0] = {0, 1};
    PIECEDIRS[ROOK][1] = {1, 0};
    PIECEDIRS[ROOK][2] = {0, -1};
    PIECEDIRS[ROOK][3] = {-1, 0};
    PIECEDIRS[ROOK][4] = {0, 0};

    PIECEDIRS[BISHOP] = (Pair*)malloc(5 * sizeof(Pair));
    PIECEDIRS[BISHOP][0] = {1, 1};
    PIECEDIRS[BISHOP][1] = {1, -1};
    PIECEDIRS[BISHOP][2] = {-1, 1};
    PIECEDIRS[BISHOP][3] = {-1, -1};
    PIECEDIRS[BISHOP][4] = {0, 0};

    PIECEDIRS[KNIGHT] = (Pair*)malloc(9 * sizeof(Pair));
    PIECEDIRS[KNIGHT][0] = {1, 2};
    PIECEDIRS[KNIGHT][1] = {2, 1};
    PIECEDIRS[KNIGHT][2] = {-1, 2};
    PIECEDIRS[KNIGHT][3] = {2, -1};
    PIECEDIRS[KNIGHT][4] = {-1, -2};
    PIECEDIRS[KNIGHT][5] = {-2, -1};
    PIECEDIRS[KNIGHT][6] = {1, -2};
    PIECEDIRS[KNIGHT][7] = {-2, 1};
    PIECEDIRS[KNIGHT][8] = {0, 0};

    PIECEDIRS[QUEEN] = (Pair*)malloc(9 * sizeof(Pair));
    PIECEDIRS[QUEEN][0] = {0, 1};
    PIECEDIRS[QUEEN][1] = {1, 0};
    PIECEDIRS[QUEEN][2] = {0, -1};
    PIECEDIRS[QUEEN][3] = {-1, 0};
    PIECEDIRS[QUEEN][4] = {1, 1};
    PIECEDIRS[QUEEN][5] = {1, -1};
    PIECEDIRS[QUEEN][6] = {-1, 1};
    PIECEDIRS[QUEEN][7] = {-1, -1};
    PIECEDIRS[QUEEN][8] = {0, 0};

    PIECEDIRS[KING] = PIECEDIRS[QUEEN];

    PIECERANGE[KNIGHT] = 1;
    PIECERANGE[BISHOP] = 7;
    PIECERANGE[ROOK] = 7;
    PIECERANGE[QUEEN] = 7;
    PIECERANGE[KING] = 1;

    PIECEVALS[0] = 0;
    PIECEVALS[PAWN] = 100;
    PIECEVALS[KNIGHT] = 300;
    PIECEVALS[BISHOP] = 300;
    PIECEVALS[ROOK] = 500;
    PIECEVALS[QUEEN] = 900;
    PIECEVALS[KING] = 32000;
}



void test() {
    tee("EMPTY BOARD EVAL: %d\n", turochamp(0));

    make_move_str("e2e4");
    tee("1. e4 EVAL: %d\n", turochamp(0));

    tee("LIST OF MOVES IN RESPONSE TO 1. e4\n");
    quiescence(BLACK, -inf+1, inf-1, 41, 0, true);
}

void test_botez() {
    tee("EMPTY BOARD EVAL: %d\n", turochamp(0));

    IM_WHITE = true;
    make_move_str("h2h4");
    make_move_str("b7b6");
    make_move_str("g1f3");
    make_move_str("c8a6");
    make_move_str("d2d4");
    make_move_str("e7e6");
    make_move_str("b1d2");
    make_move_str("d8e7");
    make_move_str("c2c4");
    make_move_str("a6b7");
    make_move_str("d1b3");
    make_move_str("e7f6");
    make_move_str("b3b5");
    make_move_str("b7c6");
    make_move_str("b5h5");
    make_move_str("f8b4");

    tee("LIST OF MOVES IN RESPONSE TO QUEEN\n");
    quiescence(WHITE, -inf+1, inf-1, 41, 0, true);
}


int main(int argc, char const *argv[])
{
    //srand(time(NULL)); // random seed
    f = fopen("jangine.log", "w");
    int num_moves = 0;

    setbuf(stdout, NULL);
    setbuf(f, NULL);
    init_data();

    std::string line_cpp;

    if (argc >= 2 and strcmp(argv[1], "-t") == 0)
        test();
    if (argc >= 2 and strcmp(argv[1], "-b") == 0)
        test_botez();

    while (true) {
        std::getline(std::cin, line_cpp);
        line_cpp.append(1, '\n');
        const char* line = line_cpp.c_str();

        tee("INPUT: %s", line);

        if (strcmp(line, "xboard\n") == 0) {
            tee("feature myname=\"jangine\"\n");
            // tee("feature sigint=0 sigterm=0\n");
            tee("feature ping=1\n");
            tee("feature setboard=1\n");
            tee("feature done=1\n");
        }

        if (strcmp(line, "force\n") == 0 or startswith("result", line)) {
            started = false;
        }

        if (strcmp(line, "white\n") == 0) {
            IM_WHITE = true;
            new_board();
        }

        if (strcmp(line, "new\n") == 0) {
            IM_WHITE = false;
            started = true;  // TODO this line needed to fix xboard for white
            new_board();
            // XXX reset KILLERHEURISTIC
        }

        // TODO: "protover 2" -> reset board
        // engine not started -> make moves anyway
        if (input_is_move(line) and not started) {
            pprint();
            tee("unstarted bs\n");
            pprint();
            make_move_str(line);
            num_moves++;
            pprint();
        }

        if (input_is_move(line) and started) {
            pprint();
            IM_WHITE = (num_moves + 1) % 2;
            make_move_str(line);
            num_moves++;
            pprint();
            std::string mv = calc_move();
            num_moves++;
            pprint();
            tee("move %s\n", mv.c_str());
        }

        if (strcmp(line, "go\n") == 0) {
            IM_WHITE = (num_moves + 1) % 2;
            started = true;
            std::string mv = calc_move();
            num_moves++;
            pprint();
            tee("move %s\n", mv.c_str());
        }

        if (startswith("ping", line)) {
            tee("pong");
        }


        if (strcmp(line, "quit\n") == 0 or strcmp(line, "q\n") == 0) {
            pprint();
            fclose(f);
            return 0;
        }
    }
}
