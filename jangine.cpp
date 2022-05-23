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
#include <deque>
#include <vector>
#include <sstream>
#include <algorithm>


// typedef int_fast8_t int;
typedef int_fast16_t num;

#define DEBUG 0
#define SEARCH_DEPTH 5  // how many plies to search
#define QUIESCENCE 41  // how deep to search in quiescence search (combines with SEARCH_DEPTH)
#define is_inside(i, j) (0 <= i and i <= 7 and 0 <= j and j <= 7)
#define gentuples for (num i = 0; i < 8; ++i) for (num j = 0; j < 8; ++j)

bool input_is_move(const char* s) {
    if (strlen(s) < 5 || strlen(s) > 6)  // newline character
        return false;

    return 'a' <= s[0] and s[0] <= 'h' and 'a' <= s[2] && s[2] <= 'h' and
            '1' <= s[1] && s[1] <= '8' and '1' <= s[3] && s[3] <= '8';
}

// TODO TODO TODO replace with vector push_back OR AT LEAST fewer indirect mallocs
#define store(a, b, c, d, e) \
    { *mvsend = (Move*)malloc(sizeof(Move)); \
    Move x = {a, b, c, d, e}; \
    memcpy(*mvsend, &x, sizeof(Move)); \
    mvsend++; }

num PAWN = 1, KNIGHT = 2, BISHOP = 4, ROOK = 8, QUEEN = 16, KING = 32, WHITE = 64, BLACK = 128;
num inf = 32000;
std::map<num, char> piece_to_letter = {{0, ' '}, {1, ' '}, {2, 'N'}, {4, 'B'}, {8, 'R'}, {16, 'Q'}, {32, 'K'}};
std::map<num, std::string> id_to_unicode = {
        {  0, "."},
        { 65, "♙"}, { 66, "♘"}, { 68, "♗"}, { 72, "♖"}, { 80, "♕"}, { 96, "♔"},
        {129, "♟"}, {130, "♞"}, {132, "♝"}, {136, "♜"}, {144, "♛"}, {160, "♚"},
};

typedef struct CASTLINGRIGHTS {
    bool lr;  // left rook has not been moved
    bool k;   // king has not been moved
    bool rr;  // right rook has not been moved
} CASTLINGRIGHTS;

typedef struct Move {
    num f0;  // from
    num f1;
    num t0;  // to
    num t1;
    char prom;  // promote to piece

    // needed for std::map
    bool operator<( const Move & that ) const {
        return 8*this->f0 + this->f1 < 8*that.f0 + that.f1;
    }
    bool operator==( const Move& that ) const {
        return this->f0 == that.f0 and this->f1 == that.f1 and this->t0 == that.t0
            and this->t1 == that.t1 and this->prom == that.prom;
    }
} Move;

// Store captured piece and new castling rights so move can be undone
typedef struct PiecePlusCatling {
    num hit_piece;
    CASTLINGRIGHTS c_rights_w;
    CASTLINGRIGHTS c_rights_b;
} PiecePlusCatling;

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
bool MODE_UCI = false;

int64_t NODES = 0;

void pprint() {
    for (num i = 0; i < 8; ++i) {
        printf("%ld ", (8 - i));
        for (num j = 0; j < 8; ++j) {
            std::string s = id_to_unicode[board[8 * i + j]];
            printf("%s ", s.c_str());
        }
        printf(" \n");
    }
    printf("  a b c d e f g h\n\n");
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

num KINGPOS_WHITE_i = 7;
num KINGPOS_WHITE_j = 4;
num KINGPOS_BLACK_i = 0;
num KINGPOS_BLACK_j = 4;

void board_clear() {  // setting up random positions
    for (int i = 0; i < 64; ++i)
        board[i] = 0;

    CASTLINGWHITE = {false, false, false};
    CASTLINGBLACK = {false, false, false};
}

void set_up_kings(num wi, num wj, num bi, num bj) {
    board[8*wi+wj] = KING + WHITE;
    board[8*bi+bj] = KING + BLACK;
    KINGPOS_WHITE_i = wi;
    KINGPOS_WHITE_j = wj;
    KINGPOS_BLACK_i = bi;
    KINGPOS_BLACK_j = bj;
}

void board_initial_position() {  // setting up a game
    for (int i = 0; i < 64; ++i)
        board[i] = default_board[i];

    set_up_kings(7, 4, 0, 4);
    CASTLINGWHITE = {true, true, true};
    CASTLINGBLACK = {true, true, true};
}

typedef struct ValuePlusMoves {
    num value;
    Move** moves;
    Move** movesend;
} ValuePlusMoves;

typedef struct ValuePlusMove {
    num value;
    Move move;
    std::deque<Move> variation;
} ValuePlusMove;


PiecePlusCatling make_move(Move mv) {

    if (mv.f0 == 0 and mv.f1 == 0 and mv.t0 == 0 and mv.t1 == 0)
        printf("XXX DANGEROUS! NULL MOVE");

    num piece = board[8*mv.f0+mv.f1];
    num hit_piece = board[8*mv.t0+mv.t1];
    board[8*mv.f0+mv.f1] = 0;
    board[8*mv.t0+mv.t1] = piece;

    num CASTLERANK = piece & WHITE ? 7 : 0;

    if (mv.prom) {
        if (mv.prom == 'e') {  // en passant
            board[8*mv.f0+mv.t1] = 0;
        } else if (mv.prom == 'c') {  // castling
            if (mv.t1 == 2) {  // long
                board[8*CASTLERANK+3] = board[8*CASTLERANK];
                board[8*CASTLERANK] = 0;
            } else {  // short
                board[8*CASTLERANK+5] = board[8*CASTLERANK+7];
                board[8*CASTLERANK+7] = 0;
            }
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

    CASTLINGRIGHTS old_cr_w = CASTLINGWHITE;
    CASTLINGRIGHTS old_cr_b = CASTLINGBLACK;

    CASTLINGRIGHTS cr = (piece & WHITE ? CASTLINGWHITE : CASTLINGBLACK);

    if (mv.f0 == CASTLERANK) {
        cr = {mv.f1 != 0 and cr.lr, mv.f1 != 4 and cr.k,  mv.f1 != 7 and cr.rr};
        if (piece & WHITE)  CASTLINGWHITE = cr;
        else                CASTLINGBLACK = cr;
    }

    if (piece == WHITE + KING) {
        KINGPOS_WHITE_i = mv.t0;
        KINGPOS_WHITE_j = mv.t1;
    }
    else if (piece == BLACK + KING) {
        KINGPOS_BLACK_i = mv.t0;
        KINGPOS_BLACK_j = mv.t1;
    }

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
        } else if (mv.prom == 'c') {  // castling -- undo rook move
            if (mv.t1 == 2) {  // long
                board[8*mv.f0] = board[8*mv.f0+3];
                board[8*mv.f0+3] = 0;
            }
            else {  // short
                board[8*mv.f0+7] = board[8*mv.f0+5];
                board[8*mv.f0+5] = 0;
            }
        } else {
            board[8*mv.f0+mv.f1] = PAWN + (mv.t0 == 0 ? WHITE : BLACK);
        }
    }

    CASTLINGWHITE = c_rights_w;
    CASTLINGBLACK = c_rights_b;
    LASTMOVE = {0};

    if (piece == WHITE + KING) {
        KINGPOS_WHITE_i = mv.f0;
        KINGPOS_WHITE_j = mv.f1;
    }
    else if (piece == BLACK + KING) {
        KINGPOS_BLACK_i = mv.f0;
        KINGPOS_BLACK_j = mv.f1;
    }
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



bool square_in_check(num COLOR, num i, num j) {
    num OPPONENT_COLOR = COLOR == WHITE ? BLACK : WHITE;
    num UP = COLOR == WHITE ? -1 : 1;

    // enemy pawns
    if (is_inside(i+UP, j+1) and board[8*(i+UP)+j+1] == PAWN + OPPONENT_COLOR)
        return true;
    if (is_inside(i+UP, j-1) and board[8*(i+UP)+j-1] == PAWN + OPPONENT_COLOR)
        return true;

    for (int n = 1; n < 6; ++n) {  // enemy pieces incl king
        num piece = 1 << n;

        for (num l = 0; PIECEDIRS[piece][l].a != 0 or PIECEDIRS[piece][l].b != 0; ++l) {
            num a = PIECEDIRS[piece][l].a;
            num b = PIECEDIRS[piece][l].b;
            for (num k = 1; k <= PIECERANGE[piece]; ++k)
            {
                if (not is_inside(i+a*k, j+b*k))  // axis goes off board
                    break;
                if (board[8*(i+a*k)+j+b*k] == OPPONENT_COLOR + piece)  // opponent piece of right type on axis
                    return true;
                if (board[8*(i+a*k)+j+b*k])  // irrelevant piece along axis
                    break;
            }
        }
    }

    return false;
}


bool king_in_check(num COLOR, bool allow_illegal_position = true) {  // 90^ of time spent in this function
    // keeping track of king position enables a HUGE speedup!
    num i_likely = COLOR == WHITE ? KINGPOS_WHITE_i : KINGPOS_BLACK_i;
    num j_likely = COLOR == WHITE ? KINGPOS_WHITE_j : KINGPOS_BLACK_j;

    if (board[8 * i_likely + j_likely] == KING + COLOR)
        return square_in_check(COLOR, i_likely, j_likely);

    gentuples {
        if (board[8 * i + j] == KING + COLOR)
            return square_in_check(COLOR, i, j);
    }

    if (allow_illegal_position)
        return true;  // no king on board -> used for illegal move detection

    // TODO: Find out why this happens anyways in unexpected moments :/
    std::cout << "no king on board, should never happen" << std::endl;
    pprint();
    std::exit(0);
}



ValuePlusMoves genlegals(num COLOR) {

    Move** mvs = (Move**)calloc(128, sizeof(Move*));  // Maximum should be 218 moves
        // also inits to NULLptrs
    Move** mvsend = mvs;

    num NCOLOR = COLOR == WHITE ? BLACK : WHITE;
    num ADD = COLOR == WHITE ? -1 : 1;  // "up"
    num STARTRANK = COLOR == WHITE ? 6 : 1;
    num PROMRANK = COLOR == WHITE ? 1 : 6;
    num EPRANK = COLOR == WHITE ? 3 : 4;
    num CASTLERANK = COLOR == WHITE ? 7 : 0;

    gentuples {
        num bij = board[8*i+j];
        num bijpiece = bij & ~WHITE & ~BLACK;
        if (not (bij & COLOR))
            continue;
        if (bij & PAWN) {  // pawn moves
            Move** badmvs = mvsend;

            if (not board[8*(i+ADD)+j]) {  // promotion
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

            if (i == EPRANK) {  // en passant
                if (LASTMOVE.f0 == i+ADD+ADD and LASTMOVE.f1 == j-1 and LASTMOVE.t0 == i and LASTMOVE.t1 == j-1) {
                    store(i, j, i+ADD, j-1, 'e');
                }
                if (LASTMOVE.f0 == i+ADD+ADD and LASTMOVE.f1 == j+1 and LASTMOVE.t0 == i and LASTMOVE.t1 == j+1) {
                    store(i, j, i+ADD, j+1, 'e');
                }
            }
        }
        else {  // non-pawn moves
            for (num l = 0; PIECEDIRS[bijpiece][l].a != 0 or PIECEDIRS[bijpiece][l].b != 0; ++l) {
                num a = PIECEDIRS[bijpiece][l].a;
                num b = PIECEDIRS[bijpiece][l].b;
                for (num k = 1; k <= PIECERANGE[bijpiece]; ++k)
                {
                    if (is_inside(i+a*k, j+b*k)) {
                        if (not board[8*(i+a*k)+j+b*k]) {
                            // printf("%ld%ld%ld%ld %ld%ld\n", i, j, i+a*k, j+b*k, a, b);
                            store(i, j, i+a*k, j+b*k, '\0');
                        } else if (board[8*(i+a*k)+j+b*k] & NCOLOR) {
                            // printf("%ld%ld%ld%ld %ld%ld\n", i, j, i+a*k, j+b*k, a, b);
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

    CASTLINGRIGHTS castlingrights = COLOR == WHITE ? CASTLINGWHITE : CASTLINGBLACK;
    if (castlingrights.k and castlingrights.rr and board[8*CASTLERANK+4] == COLOR + KING and board[8*CASTLERANK+7] == COLOR + ROOK) {  // short castle O-O
        if (not board[8*CASTLERANK+5] and not board[8*CASTLERANK+6] and
                not square_in_check(COLOR, CASTLERANK, 4) and not square_in_check(COLOR, CASTLERANK, 5) and not square_in_check(COLOR, CASTLERANK, 6))
            store(CASTLERANK, 4, CASTLERANK, 6, 'c');
    }
    if (castlingrights.k and castlingrights.lr and board[8*CASTLERANK+4] == COLOR + KING and board[8*CASTLERANK] == COLOR + ROOK) {  // long castle O-O-O
        if (not board[8*CASTLERANK+1] and not board[8*CASTLERANK+2] and not board[8*CASTLERANK+3] and
                not square_in_check(COLOR, CASTLERANK, 2) and not square_in_check(COLOR, CASTLERANK, 3) and not square_in_check(COLOR, CASTLERANK, 4))
            store(CASTLERANK, 4, CASTLERANK, 2, 'c');
    }


    Move** mvscpy = mvs;
    while (mvscpy != mvsend) {
        PiecePlusCatling ppc = make_move(**mvscpy);
        bool illegal = king_in_check(COLOR);
        unmake_move(**mvscpy, ppc.hit_piece, ppc.c_rights_w, ppc.c_rights_b);

        if (illegal) {
            free(*mvscpy);
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


// quiescence value of a move AFTER it is already on the board. Low values indicate importance
num eval_quies(num COLOR, Move mv, num hit_piece) {
    num OTHER_COLOR = (COLOR == WHITE ? BLACK : WHITE);

    if (hit_piece or mv.prom != '\0' and mv.prom != 'c')  // capture or promotion or en passant
        return 0;

    if (king_in_check(OTHER_COLOR) or board[8*mv.t0+mv.t1] & KING)  // check or was king move
        return 0;

    num quies = 20;

    if (COLOR == WHITE ? mv.f0 > mv.t0 : mv.f0 < mv.t0)  // "retreating" move -> quiet + should be searched less
        quies *= 1.2;
    if (COLOR == WHITE ? mv.f0 < mv.t0 : mv.f0 > mv.t0)  // "forward" move -> less quiescent
        quies *= 0.8;

    return quies;
}

std::string move_to_str(Move mv, bool algebraic = false) {
    char c0 = 'a' + (char) (mv.f1);
    char c1 = '0' + (char) (8 - mv.f0);
    char c2 = 'a' + (char) (mv.t1);
    char c3 = '0' + (char) (8 - mv.t0);
    char c4 = (char)(mv.prom ? mv.prom : ' ');

    if (!algebraic) {
        if (c4 == ' ' or c4 == 'c')
            return std::string{c0, c1, c2, c3};
        return std::string{c0, c1, c2, c3, c4};
    }

    num piece = board[8*mv.f0+mv.f1];
    num hit_piece = board[8*mv.t0+mv.t1];

    char alg0 = piece_to_letter[piece & ~WHITE & ~BLACK];
    char alg1 = (hit_piece or mv.prom == 'e') ? 'x' : ' ';

    std::string ret{alg0, alg1, c2, c3, ' ', ' ', '(', c0, c1, c2, c3, c4, ')'};
    return ret;
}

void printf_move(Move mv) {
    auto it = KILLERHEURISTIC.find(mv);
    num kh = it == KILLERHEURISTIC.end() ? 0 : it->second;

    std::string move_str = move_to_str(mv, true);
    const char* cstr = move_str.c_str();
    printf("MOVE %15s | KH %8ld |", cstr, kh);
}

void printf_moves(Move** mvs, num count) {
    for (long int i = 0; i < count; ++i)
    {
        if (mvs[i] != NULL) {
            printf("%ld ", i);
            printf_move(*(mvs[i]));
            printf("\n");
        }
        else{
            printf("MISSING\n");
        }
    }
}

// https://chessprogramming.wikispaces.com/Turochamp#Evaluation%20Features
// Returns centipawn value to a given board position, from WHITE's perspective
// TODO: breakdown into component numbers for better testability
num turochamp(num depth) {

    num eval = 0;

    gentuples {
        // material counting
        num bijp = board[8*i+j] & ~WHITE & ~BLACK;
        num bij = board[8*i+j];
        num MUL = bij & WHITE ? 1 : -1;

        eval += MUL * PIECEVALS[bijp];

        // king safety: how many moves would the king have if it was a queen
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

    // mobility: for each piece, add the square root of the number of moves the piece can make
    for (num MUL = -1; MUL < 2; MUL += 2)  // 2 iterations -- one for WHITE/1 one for BLACK/-1
    {
        num COLOR = (MUL == 1 ? WHITE : BLACK);

        ValuePlusMoves gl = genlegals(COLOR);

        num mvs_len = ((num)gl.movesend - (num)gl.moves) / sizeof(Move*);

        if (!mvs_len) {
            if (not king_in_check(COLOR))
                return 0;  // stalemate
            return MUL * (-inf+2000+100*depth);  // checkmate-in-DEPTH-plies
        }

        for (num j = 0; j < mvs_len; ++j)
        {
            if (gl.moves[j] == NULL)
                continue;
            // printf("%p ", gl.moves[j]);
            Move ref_mv = *(gl.moves[j]);
            num i = 0;
            if (not (board[8*ref_mv.f0+ref_mv.f1] & KING)) {
                while (j < mvs_len) {
                    if (gl.moves[j] == NULL) {
                        ++j;
                        continue;
                    }
                    if ((*(gl.moves[j])).f0 != ref_mv.f0 or (*(gl.moves[j])).f1 != ref_mv.f1)
                        break;  // next piece
                    ++i;
                    if (board[8* (*(gl.moves[j])).t0 + (*(gl.moves[j])).t1])  // capture counts as 2
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
    Move **move_a = (Move **)a;
    Move **move_b = (Move **)b;
    if (!*move_b)
        return -1;
    if (!*move_a)
        return 1;

    auto vala = KILLERHEURISTIC.find(**move_a) == KILLERHEURISTIC.end() ? 0 : KILLERHEURISTIC[**move_a];
    auto valb = KILLERHEURISTIC.find(**move_b) == KILLERHEURISTIC.end() ? 0 : KILLERHEURISTIC[**move_b];
    return valb - vala;
}


ValuePlusMove quiescence(num COLOR, num alpha, num beta, num quies, num depth, num noise, bool lines) {

    NODES += 1;

    if (quies <= 0 or depth > SEARCH_DEPTH)
        return {turochamp(depth), {0}, std::deque<Move>()};

    ValuePlusMove best = {COLOR == WHITE ? -inf : inf, {0}, std::deque<Move>()};

    ValuePlusMoves gl = genlegals(COLOR);
    Move** gl_moves_backup = gl.moves;

    num mvs_len = ((num)gl.movesend - (num)gl.moves) / sizeof(Move*);

    if (!mvs_len) {
        free(gl_moves_backup);

        if (not king_in_check(COLOR))
            return {0, {0}, std::deque<Move>()};  // stalemate
        return {(COLOR == WHITE ? 1 : -1) * (-inf+2000+100*depth), {0}, std::deque<Move>()};  // checkmate
    }

    // printf("BEFORE ALL\n");
    // printf(" mvs_len: %ld\n", mvs_len);
    // printf_moves(gl.moves, mvs_len);
    qsort(gl.moves, mvs_len, sizeof(Move*), killer_cmp);
    // printf_moves(gl.moves, mvs_len);
    // printf("AFTER QSORT\n");

    while (gl.moves != gl.movesend) {

        if (!*(gl.moves)) {
            gl.moves++;
            continue;
        }

        Move mv = **(gl.moves);

        if (depth < 2 and DEBUG) {
            for (int i = 0; i < depth; i++) printf("    ");
            printf_move(mv); printf(" QUIES %ld \n", quies);
        }

        PiecePlusCatling ppc = make_move(mv);

        ValuePlusMove rec = quiescence(
            COLOR == WHITE ? BLACK : WHITE,
            alpha,
            beta,
            quies - eval_quies(COLOR, mv, ppc.hit_piece),
            depth + 1,
            false,
            lines
        );

        unmake_move(mv, ppc.hit_piece, ppc.c_rights_w, ppc.c_rights_b);

        if (COLOR == WHITE ? rec.value > best.value : rec.value < best.value)
            best = {rec.value, mv, rec.variation};

        if (depth == 0 or DEBUG) {
            for (int i = 0; i < depth; i++)
                printf("    ");
            printf_move(mv);
            // due to alpha-beta pruning, this isn't the "true" eval for suboptimal moves, unless lines=true
            bool accurate = (best.move == mv) or lines;
            printf(" EVAL %7.2f %c | VAR  ... ", (float)(rec.value) / 100, accurate ? ' ' : '?');

            // TODO: not entirely accurate cos move_to_str reads current board state
            for (int i = 0; i < rec.variation.size(); i++)
                std::cout << move_to_str(rec.variation[i], true) << " ";
            std::cout << std::endl;
        }

        if (not (lines and depth == 0)) {
            if (COLOR == WHITE and best.value > alpha)
                alpha = best.value;
            if (COLOR == BLACK and best.value < beta)
                beta = best.value;
            if (beta <= alpha) {
                if (KILLERHEURISTIC.find(mv) == KILLERHEURISTIC.end()) {
                    KILLERHEURISTIC[mv] = 0;
                }
                KILLERHEURISTIC[mv] += depth * depth;
                break;
            }
        }

        gl.moves++;
    }

    for (int i = 0; i < 128; ++i)
        if (gl_moves_backup[i])
            free(gl_moves_backup[i]);
    free(gl_moves_backup);

    best.variation.push_front(best.move);
    return best;
}


std::string calc_move(bool lines = false) {
    // TODO: mode where a random move gets picked?
    // TODO: what happens if no moves (stalemate/checkmate)? Null?
    printf("Starting quiescence with depth %d\n", SEARCH_DEPTH);
    NODES = 0;
    KILLERHEURISTIC.clear();
    ValuePlusMove bestmv = quiescence(IM_WHITE ? WHITE : BLACK, -inf+1, inf-1, QUIESCENCE, 0, false, lines);
    Move mv = bestmv.move;
    printf("--> BEST ");
    printf_move(mv);
    printf("\n");
    make_move(mv);
    printf("SEARCHED %ld NODES\n", NODES);

    return move_to_str(mv);
}


void init_data(void) {
    // TODO use map with vector inside or something to avoid leaks
    board_initial_position();

    PIECEDIRS = (Pair**)calloc(65, sizeof(Pair*));
    PIECERANGE = (num*)calloc(65, sizeof(num));
    PIECEVALS = (num*)calloc(65, sizeof(num));

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

    PIECEDIRS[BISHOP] = (Pair*)malloc(5 * sizeof(Pair));
    PIECEDIRS[BISHOP][0] = {1, 1};
    PIECEDIRS[BISHOP][1] = {1, -1};
    PIECEDIRS[BISHOP][2] = {-1, 1};
    PIECEDIRS[BISHOP][3] = {-1, -1};
    PIECEDIRS[BISHOP][4] = {0, 0};

    PIECEDIRS[ROOK] = (Pair*)malloc(5 * sizeof(Pair));
    PIECEDIRS[ROOK][0] = {0, 1};
    PIECEDIRS[ROOK][1] = {1, 0};
    PIECEDIRS[ROOK][2] = {0, -1};
    PIECEDIRS[ROOK][3] = {-1, 0};
    PIECEDIRS[ROOK][4] = {0, 0};

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
    PIECEVALS[BISHOP] = 325;  // Fischer
    PIECEVALS[ROOK] = 500;
    PIECEVALS[QUEEN] = 900;
    PIECEVALS[KING] = inf;
}



void test() {
    std::cout << "Tests Mate-in-1 #1 (and stalemate)" << std::endl;

    IM_WHITE = true;
    board_clear();
    set_up_kings(0, 2, 0, 0);
    board[63] = WHITE + ROOK;
    pprint();
    std::cout << calc_move(true) << std::endl;

    std::cout << "Tests Mate-in-2 #1" << std::endl;

    board_clear();
    set_up_kings(7, 2, 0, 1);
    board[7] = BLACK + ROOK;
    board[8+0] = BLACK + PAWN;
    board[8+1] = BLACK + PAWN;
    board[8+2] = BLACK + PAWN;
    board[5*8+3] = WHITE + QUEEN;
    board[7*8+3] = WHITE + ROOK;
    pprint();
    std::cout << calc_move(true) << std::endl;

    std::cout << "Tests Mate-in-3 #1" << std::endl;

    board_clear();
    set_up_kings(7, 2, 0, 2);
    board[7] = BLACK + ROOK;
    board[8+0] = BLACK + PAWN;
    board[8+1] = BLACK + PAWN;
    board[8+2] = BLACK + PAWN;
    board[5*8+3] = WHITE + QUEEN;
    board[7*8+3] = WHITE + ROOK;
    pprint();
    std::cout << calc_move(true) << std::endl;

    std::cout << "Tests Castling" << std::endl;

    board_initial_position();
    board[7*8+5] = 0;
    board[7*8+6] = 0;
    board[6*8+4] = 0;
    pprint();
    std::cout << calc_move(true) << std::endl;

    std::cout << "Tests En-Passant" << std::endl;

    board_initial_position();
    make_move_str("e2e4");
    make_move_str("a7a6");
    make_move_str("e4e5");
    make_move_str("d7d5");
    pprint();
    std::cout << calc_move(true) << std::endl;

    std::cout << "Tests Promotion" << std::endl;

    board_clear();
    set_up_kings(0, 2, 0, 0);
    board[1*8+6] = WHITE + PAWN;
    pprint();
    std::cout << calc_move(true) << std::endl;

    std::cout << "Tests Lasker Trap in Albin Countergambit" << std::endl;

    board_initial_position();
    IM_WHITE = false;
    make_move_str("d2d4");
    make_move_str("d7d5");
    make_move_str("c2c4");
    make_move_str("e7e5");
    make_move_str("d4e5");
    make_move_str("d5d4");
    make_move_str("e2e3");
    make_move_str("f8b4");
    make_move_str("c1d2");
    make_move_str("d4e3");
    make_move_str("d2b4");
    make_move_str("e3f2");
    make_move_str("e1e2");
    pprint();
    std::cout << calc_move(true) << std::endl;

    board_initial_position();
    IM_WHITE = true;
    printf("INITIAL BOARD EVAL: %ld\n", turochamp(0));

    make_move_str("e2e4");
    printf("1. e4 EVAL: %ld\n", turochamp(0));

    printf("LIST OF MOVES IN RESPONSE TO 1. e4\n");
    pprint();
    quiescence(BLACK, -inf+1, inf-1, 41, 0, true, true);

    IM_WHITE = true;
    board_initial_position();
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
    pprint();

    printf("LIST OF MOVES IN RESPONSE TO QUEEN\n");
    std::cout << calc_move(true) << std::endl;
    std::exit(0);
}


int main(int argc, char const *argv[])
{
    setbuf(stdout, NULL);
    init_data();

    if (argc >= 2 and strcmp(argv[1], "-t") == 0)
        test();

    std::string line_cpp;
    int num_moves = 0;

    while (true) {
        std::getline(std::cin, line_cpp);
        line_cpp.append(1, '\n');
        const char* line = line_cpp.c_str();

        printf("INPUT: %s", line);

        if (strcmp(line, "xboard\n") == 0) {
            printf("feature myname=\"jangine\"\n");
            // printf("feature sigint=0 sigterm=0\n");
            printf("feature ping=1\n");
            printf("feature setboard=1\n");
            printf("feature done=1\n");
            MODE_UCI = false;
        }

        /*** UCI ***/
        if (strcmp(line, "uci\n") == 0) {
            printf("id name jangine\n");
            printf("uciok\n");
            MODE_UCI = true;
        }

        if (strcmp(line, "isready\n") == 0) {
            printf("readyok\n");
        }

        if (strcmp(line, "ucinewgame\n") == 0) {
            IM_WHITE = false;
            board_initial_position();
        }

        if (startswith("position startpos moves", line)) {
            std::string buf;
            std::stringstream ss(line_cpp);
            std::vector<std::string> tokens;

            while (ss >> buf)
                tokens.push_back(buf);

            board_initial_position();
            for (int i = 3; i < tokens.size(); ++i) {
                make_move_str(tokens[i].c_str());
                num_moves++;
            }
            IM_WHITE = (num_moves + 1) % 2;
        }
        /*** UCI END ***/

        if (strcmp(line, "force\n") == 0 or startswith("result", line)) {
            started = false;
        }

        if (strcmp(line, "white\n") == 0) {
            IM_WHITE = true;
            board_initial_position();
        }

        if (strcmp(line, "new\n") == 0) {
            IM_WHITE = false;
            started = true;  // TODO this line needed to fix xboard for white
            board_initial_position();
        }

        // TODO: "protover 2" -> reset board
        // engine not started -> make moves anyway
        if (input_is_move(line) and not started) {
            pprint();
            printf("unstarted bs\n");
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
            printf(MODE_UCI ? "bestmove %s\n" : "move %s\n", mv.c_str());
        }

        if (startswith("go", line)) {
            IM_WHITE = (num_moves + 1) % 2;
            started = true;
            std::string mv = calc_move();
            num_moves++;
            pprint();
            printf(MODE_UCI ? "bestmove %s\n" : "move %s\n", mv.c_str());
        }

        if (startswith("ping", line)) {
            std::replace(line_cpp.begin(), line_cpp.end(), 'i', 'o');
            std::cout << line_cpp << std::endl;
        }

        if (strcmp(line, "quit\n") == 0 or strcmp(line, "q\n") == 0) {
            pprint();
            return 0;
        }
    }
}
