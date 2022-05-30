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

#define SIMPLE_EVAL  // TODO: just-material eval
#define DEBUG 0
#define NO_QUIES 0
#define QUIES_DEPTH 0  // TODO: limit search of quiescence captures
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
#define store_maybe_promote(a, b, c, d)   \
    { if (i == PROMRANK) {                   \
        store(a, b, c, d, 'q'); /* best */   \
        store(a, b, c, d, 'n'); /* likely */ \
        store(a, b, c, d, 'r');              \
        store(a, b, c, d, 'b');              \
    } else                                   \
        store(a, b, c, d, '\0');             \
    }

num PAWN = 1, KNIGHT = 2, BISHOP = 4, ROOK = 8, QUEEN = 16, KING = 32, WHITE = 64, BLACK = 128;
num KING_EARLYGAME = 8001, KING_ENDGAME = 8002, PAWN_EARLYGAME = 9001, PAWN_ENDGAME = 9002;
num COLORBLIND = ~(WHITE | BLACK);  // use 'piece & COLORBLIND' to remove color from a piece
num inf = 32000;
std::map<num, char> piece_to_letter = {{0, ' '}, {1, ' '}, {2, 'N'}, {4, 'B'}, {8, 'R'}, {16, 'Q'}, {32, 'K'}};
std::map<num, std::string> id_to_unicode = {
        {  0, "."},
        { 65, "♙"}, { 66, "♘"}, { 68, "♗"}, { 72, "♖"}, { 80, "♕"}, { 96, "♔"},
        {129, "♟"}, {130, "♞"}, {132, "♝"}, {136, "♜"}, {144, "♛"}, {160, "♚"},
};
std::map<num, std::array<num, 64>> PIECE_SQUARE_TABLES = {
        {0, std::array<num, 64>{
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
        }},
        {PAWN_EARLYGAME, std::array<num, 64>{
                  0,   0,   0,   0,   0,   0,   0,   0,
                100, 100, 100, 100, 100, 100, 100, 100,
                 20,  20,  40,  60,  60,  40,  60,  60,
                  0,   0,   0,  10,  10,   0,  20,  20,
                  5,   0,  10,  30,  30,   0,   0,   0,
                  5,   0,   0,   0,   0,   0,   0,   0,
                  0,   0, -10, -20, -20,  20,  20,  10,
                  0,   0,   0,   0,   0,   0,   0,   0,
        }},
        {PAWN_ENDGAME, std::array<num, 64>{
                  0,   0,   0,   0,   0,   0,   0,   0,
                100, 100, 100, 100, 100, 100, 100, 100,
                 60,  60,  60,  60,  60,  60,  60,  40,
                 40,  40,  40,  40,  40,  40,  40,  40,
                 20,  20,  20,  20,  20,  20,  20,  20,
                 10,  10,  10,  10,  10,  10,  10,  10,
                  0,   0,   0,   0,   0,   0,   0,   0,
                  0,   0,   0,   0,   0,   0,   0,   0,
        }},
        // TODO: avoid the knight moving too much forward and getting trapped?
        {KNIGHT, std::array<num, 64>{
                -50,-40,-30,-30,-30,-30,-40,-50,
                -40,-20,  0,  0,  0,  0,-20,-40,
                -30,  0, 10, 15, 15, 10,  0,-30,
                -30,  5, 15, 20, 20, 15,  5,-30,
                -30,  0, 15, 20, 20, 15,  0,-30,
                -30,  5, 10, 15, 15, 10,  5,-30,
                -40,-20,  0,  5,  5,  0,-20,-40,
                -50,-40,-30,-30,-30,-30,-40,-50,
        }},
        // TODO: 15/20 to develop to active squares?
        // bishop isn't actually bad on the bad row but prevents castling
        {BISHOP, std::array<num, 64>{
                -20,-10,-10,-10,-10,-10,-10,-20,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -10,  0,  5, 10, 10,  5,  0,-10,
                -10, 20,  5, 10, 10,  5, 20,-10,
                -10,  0, 15, 10, 10, 15,  0,-10,
                -10, 10, 10, 10, 10, 10, 10,-10,
                -10,  5,  0,  0,  0,  0,  5,-10,
                -20,-10,-10,-10,-10,-10,-10,-20,
        }},
        {ROOK, std::array<num, 64>{
                  0,  0,  0,  0,  0,  0,  0,  0,
                  5, 30, 30, 30, 30, 30, 30,  5,
                 -5,  0,  0, 15, 15,  0,  0, -5,
                 -5,  0,  0, 15, 15,  0,  0, -5,
                 -5,  0,  0, 15, 15,  0,  0, -5,
                 -5,  0,  0, 15, 15,  0,  0, -5,
                 -5,  0,  0, 15, 15,  0,  0, -5,
                  0,  0,  0, 15, 15,  0,  0,  0
        }},
        // TODO: avoid capturing b7?
        {QUEEN, std::array<num, 64>{
                -20,-10,-10, -5, -5,-10,-10,-20,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -10,  0,  5,  5,  5,  5,  0,-10,
                 -5,  0,  5,  5,  5,  5,  0, -5,
                  0,  0,  5,  5,  5,  5,  0, -5,
                -10,  5,  5,  5,  5,  5,  0,-10,
                -10,  0,  5,  0,  0,  0,  0,-10,
                -20,-10,-10, -5, -5,-10,-10,-20
        }},
        {KING_EARLYGAME, std::array<num, 64>{
                -20,-20,-20,-20,-20,-20,-20,-20,
                -20,-20,-20,-20,-20,-20,-20,-20,
                -20,-20,-20,-20,-20,-20,-20,-20,
                -20,-20,-20,-20,-20,-20,-20,-20,
                -20,-20,-20,-20,-20,-20,-20,-20,
                -20,-20,-20,-20,-20,-20,-20,-20,
                 40, 40, 20,  0,  0,  0, 40, 40,
                 40, 50, 30,  0, 10,  0, 50, 40
        }},
        {KING_ENDGAME, std::array<num, 64>{
                 50, 50, 50, 50, 50, 50, 50, 50,
                 50, 50, 50, 50, 50, 50, 50, 50,
                 50, 50, 50, 50, 50, 50, 50, 50,
                 20, 30, 40, 50, 50, 40, 30, 20,
                 10, 10, 30, 40, 40, 30, 10, 10,
                  0,  0, 20, 30, 30, 20,  0,  0,
                -30,-30,  0,  0,  0,  0,-30,-30,
                -30,-30,-30,-30,-30,-30,-30,-30
        }}
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
num board_eval = 0;
bool IM_WHITE = false;
bool started = true;
bool MODE_UCI = false;

int64_t NODES_NORMAL = 0;
int64_t NODES_QUIES = 0;

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
    return lenstr >= lenpre and strncmp(pre, str, lenpre) == 0;
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

    PIECE_SQUARE_TABLES[KING] = PIECE_SQUARE_TABLES[KING_EARLYGAME];
    PIECE_SQUARE_TABLES[PAWN] = PIECE_SQUARE_TABLES[PAWN_EARLYGAME];

    set_up_kings(7, 4, 0, 4);
    CASTLINGWHITE = {true, true, true};
    CASTLINGBLACK = {true, true, true};
    board_eval = 0;
}

void board_clear() {  // setting up random positions
    board_initial_position();

    for (int i = 0; i < 64; ++i)
        board[i] = 0;

    CASTLINGWHITE = {false, false, false};
    CASTLINGBLACK = {false, false, false};
}

num SEARCH_DEPTH = 5;  // how many plies to search  // -t: 4 -> 19s,  5 -> 69s
num SEARCH_ADAPTIVE_DEPTH = 21;  // how deep to search in adaptive search (combines with SEARCH_DEPTH)

num HYPERHYPERHYPERBULLET = 1;
num HYPERHYPERBULLET = 2;
num HYPERBULLET = 3;
num BULLET = 4;
num BLITZ = 5;
num RAPID = 6;
num CLASSICAL = 7;
num CLASSICAL_PLUS = 8;
num CLASSICAL_PLUS_PLUS = 9;
num TIME_CONTROL_REQUESTED = BLITZ;
void set_time_control(num TIME_CONTROL) {
    if (TIME_CONTROL == HYPERHYPERHYPERBULLET) {  // -t: 0.1s
        SEARCH_DEPTH = 3;
        SEARCH_ADAPTIVE_DEPTH = 11;
    }
    if (TIME_CONTROL == HYPERHYPERBULLET) {  // -t: 0.4s
        SEARCH_DEPTH = 4;
        SEARCH_ADAPTIVE_DEPTH = 13;
    }
    if (TIME_CONTROL == HYPERBULLET) {  // -t: 1.8s
        SEARCH_DEPTH = 4;
        SEARCH_ADAPTIVE_DEPTH = 21;
    }
    if (TIME_CONTROL == BULLET) {  // -t: 9.3s
        SEARCH_DEPTH = 5;
        SEARCH_ADAPTIVE_DEPTH = 25;
    }
    if (TIME_CONTROL == BLITZ) {  // -t: 25s
        SEARCH_DEPTH = 6;
        SEARCH_ADAPTIVE_DEPTH = 29;
    }
    if (TIME_CONTROL == RAPID) {  // -t: 59s
        SEARCH_DEPTH = 6;
        SEARCH_ADAPTIVE_DEPTH = 33;
    }
    if (TIME_CONTROL == CLASSICAL) {  // -t: 221s
        SEARCH_DEPTH = 7;
        SEARCH_ADAPTIVE_DEPTH = 35;
    }
    if (TIME_CONTROL == CLASSICAL_PLUS) {  // -t: ...s
        SEARCH_DEPTH = 7;
        SEARCH_ADAPTIVE_DEPTH = 39;
    }
    if (TIME_CONTROL >= CLASSICAL_PLUS_PLUS) {
        SEARCH_DEPTH = 8;
        SEARCH_ADAPTIVE_DEPTH = 41;
    }
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

num eval_material(num piece_with_color) {
    num piece = piece_with_color & COLORBLIND;
    return piece_with_color & WHITE ? PIECEVALS[piece] : -PIECEVALS[piece];
}

num eval_piece_on_square(num piece_with_color, num i, num j) {
    num piece = piece_with_color & COLORBLIND;
    return piece_with_color & WHITE ? PIECE_SQUARE_TABLES[piece][8*i+j] : -PIECE_SQUARE_TABLES[piece][8*(7-i)+j];
}

// TODO: test Nb4 incident  https://lichess.org/Y7wbd6Jn04IP
// https://chessprogramming.wikispaces.com/Turochamp#Evaluation%20Features
// https://www.chessprogramming.org/Simplified_Evaluation_Function
// Returns centipawn value to a given board position, from WHITE's perspective
// TODO: breakdown into component numbers for better testability
num initial_eval() {
    num eval = 0;

    // material counting
    gentuples {
        eval += eval_material(board[8*i+j]);
    }

    // https://www.chessprogramming.org/Piece-Square_Tables
    // piece positioning using piece-square-tabkes
    gentuples {
        eval += eval_piece_on_square(board[8*i+j], i, j);
    }

    return eval;
}



PiecePlusCatling make_move(Move mv) {

    if (mv.f0 == 0 and mv.f1 == 0 and mv.t0 == 0 and mv.t1 == 0)
        printf("XXX DANGEROUS! NULL MOVE");

    num piece = board[8*mv.f0+mv.f1];
    num hit_piece = board[8*mv.t0+mv.t1];
    board[8*mv.f0+mv.f1] = 0;
    board[8*mv.t0+mv.t1] = piece;

    num CASTLERANK = piece & WHITE ? 7 : 0;

    board_eval -= eval_material(hit_piece) + eval_piece_on_square(hit_piece, mv.t0, mv.t1);  // remove captured piece. ignores empty squares (0s)  // always empty for en passant
    board_eval += eval_piece_on_square(piece, mv.t0, mv.t1) - eval_piece_on_square(piece, mv.f0, mv.f1);  // adjust position values of moved piece

    if (mv.prom) {
        if (mv.prom == 'e') {  // en passant
            board_eval -= eval_material(board[8*mv.f0+mv.t1]) + eval_piece_on_square(board[8*mv.f0+mv.t1], mv.f0, mv.t1);  // captured pawn did not get removed earlier
            board[8*mv.f0+mv.t1] = 0;
        } else if (mv.prom == 'c') {  // castling
            if (mv.t1 == 2) {  // long
                board[8*CASTLERANK+3] = board[8*CASTLERANK];
                board[8*CASTLERANK] = 0;
                board_eval += eval_piece_on_square(board[8*CASTLERANK+3], CASTLERANK, 3) - eval_piece_on_square(board[8*CASTLERANK+3], CASTLERANK, 0);  // rook positioning
            } else {  // short
                board[8*CASTLERANK+5] = board[8*CASTLERANK+7];
                board[8*CASTLERANK+7] = 0;
                board_eval += eval_piece_on_square(board[8*CASTLERANK+5], CASTLERANK, 5) - eval_piece_on_square(board[8*CASTLERANK+5], CASTLERANK, 7);  // rook positioning
            }
        } else {
            num p;
            switch (mv.prom) {
                case 'q':  p = QUEEN;   break;
                case 'r':  p = ROOK;    break;
                case 'b':  p = BISHOP;  break;
                case 'n':  p = KNIGHT;  break;
            }
            p += (mv.t0 == 0 ? WHITE : BLACK);
            board[8*mv.t0+mv.t1] = p;
            board_eval += eval_material(p) - eval_material(piece);
            board_eval += eval_piece_on_square(p, mv.t0, mv.t1) - eval_piece_on_square(piece, mv.t0, mv.t1);  // t0/t1 to compensate for wrong pieceval earlier
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

    board_eval += eval_material(hit_piece) + eval_piece_on_square(hit_piece, mv.t0, mv.t1);  // add captured piece back in
    board_eval -= eval_piece_on_square(piece, mv.t0, mv.t1) - eval_piece_on_square(piece, mv.f0, mv.f1);  // adjust position values of moved piece

    if (mv.prom)
    {
        if (mv.prom == 'e') {
            board[8*mv.f0+mv.t1] = PAWN + (mv.t0 == 5 ? WHITE : BLACK);
            board_eval += eval_material(board[8*mv.f0+mv.t1]) + eval_piece_on_square(board[8*mv.f0+mv.t1], mv.f0, mv.t1);  // captured pawn did not get added earlier
        } else if (mv.prom == 'c') {  // castling -- undo rook move
            if (mv.t1 == 2) {  // long
                board[8*mv.f0] = board[8*mv.f0+3];
                board[8*mv.f0+3] = 0;
                board_eval += eval_piece_on_square(board[8*mv.f0], mv.f0, 0) - eval_piece_on_square(board[8*mv.f0], mv.f0, 3);  // rook positioning
            }
            else {  // short
                board[8*mv.f0+7] = board[8*mv.f0+5];
                board[8*mv.f0+5] = 0;
                board_eval += eval_piece_on_square(board[8*mv.f0+7], mv.f0, 7) - eval_piece_on_square(board[8*mv.f0+7], mv.f0, 5);  // rook positioning
            }
        } else {
            num old_pawn = PAWN + (mv.t0 == 0 ? WHITE : BLACK);
            board[8*mv.f0+mv.f1] = old_pawn;
            board_eval -= eval_material(piece) - eval_material(old_pawn);
            board_eval -= eval_piece_on_square(piece, mv.f0, mv.f1) - eval_piece_on_square(old_pawn, mv.f0, mv.f1);  // t0/t1 to compensate for wrong pieceval earlier
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



ValuePlusMoves genlegals(num COLOR, bool only_captures = false) {

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
        num bijpiece = bij & COLORBLIND;
        if (not (bij & COLOR))
            continue;
        if (bij & PAWN) {  // pawn moves
            // diagonal captures
            if (j < 7 and board[8*(i+ADD)+j+1] & NCOLOR)
                store_maybe_promote(i, j, i+ADD, j+1);
            if (j > 0 and board[8*(i+ADD)+j-1] & NCOLOR)
                store_maybe_promote(i, j, i+ADD, j-1);

            if (i == EPRANK) {  // en passant capture
                if (LASTMOVE.f0 == i+ADD+ADD and LASTMOVE.f1 == j-1 and LASTMOVE.t0 == i and LASTMOVE.t1 == j-1 and board[8*i+j-1] & PAWN)
                    store(i, j, i+ADD, j-1, 'e');
                if (LASTMOVE.f0 == i+ADD+ADD and LASTMOVE.f1 == j+1 and LASTMOVE.t0 == i and LASTMOVE.t1 == j+1 and board[8*i+j+1] & PAWN)
                    store(i, j, i+ADD, j+1, 'e');
            }

            if (only_captures)
                continue;

            if (not board[8*(i+ADD)+j]) {  // 1 square forward
                store_maybe_promote(i, j, i+ADD, j);
                if (i == STARTRANK and not board[8*(i+ADD+ADD)+j]) {  // 2 squares forward
                    store(i, j, i+ADD+ADD, j, '\0');
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
                        if (not only_captures and not board[8*(i+a*k)+j+b*k]) {  // empty square
                            store(i, j, i+a*k, j+b*k, '\0');
                        } else if (board[8*(i+a*k)+j+b*k] & NCOLOR) {  // square with enemy piece
                            store(i, j, i+a*k, j+b*k, '\0');
                            break;
                        } else if (board[8*(i+a*k)+j+b*k] & COLOR) {  // square with own piece
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


// adaptive quiescence-like value of a move AFTER it is already on the board. Low values indicate importance
num eval_adaptive_depth(num COLOR, Move mv, num hit_piece, bool skip) {
    if (skip)  // skip for quiescence search, simply not needed
        return 0;

    num OTHER_COLOR = (COLOR == WHITE ? BLACK : WHITE);

    if (hit_piece or mv.prom != '\0' and mv.prom != 'c')  // capture or promotion or en passant
        return 0;

    if (king_in_check(OTHER_COLOR) or board[8*mv.t0+mv.t1] & KING)  // check or was king move
        return 0;

    if (COLOR == WHITE ? mv.f0 > mv.t0 : mv.f0 < mv.t0)  // "forward" move -> less quiescent
        return 6;

    return 10;
}

std::string move_to_str(Move mv, bool algebraic = false) {
    char c0 = 'a' + (char) (mv.f1);
    char c1 = '0' + (char) (8 - mv.f0);
    char c2 = 'a' + (char) (mv.t1);
    char c3 = '0' + (char) (8 - mv.t0);
    char c4 = (char)(mv.prom ? mv.prom : ' ');

    if (!algebraic) {
        if (c4 == ' ' or c4 == 'c' or c4 == 'e')
            return std::string{c0, c1, c2, c3};
        return std::string{c0, c1, c2, c3, c4};
    }

    num piece = board[8*mv.f0+mv.f1];
    num hit_piece = board[8*mv.t0+mv.t1];

    char alg0 = piece_to_letter[piece & COLORBLIND];
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

void printf_moves(Move** mvs, num count, std::string header) {
    std::cout << header;
    for (long int i = 0; i < count; ++i)
    {
        if (mvs[i] != NULL) {
            printf("%ld ", i);
            printf_move(*(mvs[i]));
            printf("\n");
        }
        else{
            printf("%ld MISSING\n", i);
        }
    }
}


// https://www.chessprogramming.org/MVV-LVA
// For captures, first consider low-value pieces capturing high-value pieces to produce alpha-beta-cutoffs
// In this specific implementation, all captures are sorted before non-captures.
// For the remaining non-captures, the least valuable pieces move first (good cos queens have so many moves)
int mvv_lva_cmp(const void* a, const void* b) {
    Move **move_a = (Move **)a;
    Move **move_b = (Move **)b;
    if (!*move_b)  return -1;  // -1 = pick a as left value
    if (!*move_a)  return 1;   // +1 = pick b as left value

    Move mv_a = **move_a;
    Move mv_b = **move_b;

    auto vala = 1000 * PIECEVALS[board[8*mv_a.t0+mv_a.t1] & COLORBLIND] - PIECEVALS[board[8*mv_a.f0+mv_a.f1] & COLORBLIND];
    auto valb = 1000 * PIECEVALS[board[8*mv_b.t0+mv_b.t1] & COLORBLIND] - PIECEVALS[board[8*mv_b.f0+mv_b.f1] & COLORBLIND];

    // auto vala = board[8*mv_a.t0+mv_a.t1] == 0 ? PIECEVALS[board[8*mv_a.f0+mv_a.f1] & COLORBLIND] : 1000 * PIECEVALS[board[8*mv_a.t0+mv_a.t1] & COLORBLIND] - PIECEVALS[board[8*mv_a.f0+mv_a.f1] & COLORBLIND];
    // auto valb = board[8*mv_b.t0+mv_b.t1] == 0 ? PIECEVALS[board[8*mv_b.f0+mv_b.f1] & COLORBLIND] : 1000 * PIECEVALS[board[8*mv_b.t0+mv_b.t1] & COLORBLIND] - PIECEVALS[board[8*mv_b.f0+mv_b.f1] & COLORBLIND];

    return valb - vala;  // invert comparison so biggest-valued move is at start of list
}


// Order moves by killer heuristic
int killer_cmp(const void* a, const void* b) {
    Move **move_a = (Move **)a;
    Move **move_b = (Move **)b;
    if (!*move_b)  return -1;  // -1 = pick a as left value
    if (!*move_a)  return 1;   // +1 = pick b as left value

    auto vala = KILLERHEURISTIC.find(**move_a) == KILLERHEURISTIC.end() ? 0 : KILLERHEURISTIC[**move_a];
    auto valb = KILLERHEURISTIC.find(**move_b) == KILLERHEURISTIC.end() ? 0 : KILLERHEURISTIC[**move_b];

    return valb - vala;  // invert comparison so biggest-valued move is at start of list
}


// non-negamax quiescent alpha-beta minimax search
// https://www.chessprogramming.org/Quiescence_Search
ValuePlusMove alphabeta(num COLOR, num alpha, num beta, num adaptive, bool is_quies, num depth, bool lines) {

    NODES_NORMAL += !is_quies;
    NODES_QUIES += is_quies;

    if (is_quies) {
        //if (depth >= SEARCH_DEPTH + 99)
        if (NO_QUIES)
            return {board_eval, {0}, std::deque<Move>()};
    }
    else
        if (adaptive < 0 or depth >= SEARCH_DEPTH)
            return alphabeta(COLOR, alpha, beta, adaptive, true, depth, lines);

    ValuePlusMove best = {COLOR == WHITE ? -inf : inf, {0}, std::deque<Move>()};
    if (is_quies) {
        // "standing pat": to compensate for not considering non-capture moves, at least one move should be
        //    better than doing no move ("null move") -> avoids senseless captures, but vulnerable to zugzwang
        best = {board_eval, {0}, std::deque<Move>()};
        // TODO: why does valgrind throw erros for M1 ??

        if (COLOR == WHITE) {
            if (best.value >= beta) {
                best.value = beta;
                return best;
            }
            if (alpha < best.value)
                alpha = best.value;
        }
        if (COLOR == BLACK) {
            if (best.value <= alpha) {
                best.value = alpha;
                return best;
            }
            if (beta > best.value)
                beta = best.value;
        }
    }

    ValuePlusMoves gl = genlegals(COLOR, is_quies);  // TODO: how to handle checks?
    Move** gl_moves_backup = gl.moves;

    num mvs_len = ((num)gl.movesend - (num)gl.moves) / sizeof(Move*);

    // TODO: if we have no moves this could be no captures
    // TODO: move stalemate/checkmate detection from turochamp to here
    // TODO: depth-adjusted checkmating value missing in quies search
    // qsort apparently cannot handle empty arrays (nmemb=0) so handle them here
    if (!is_quies and !mvs_len) {
        free(gl_moves_backup);

        if (not king_in_check(COLOR))
            return {0, {0}, std::deque<Move>()};  // stalemate
        return {(COLOR == WHITE ? 1 : -1) * (-inf+2000+100*depth), {0}, std::deque<Move>()};  // checkmate
    }

    if (is_quies and !mvs_len) {
        free(gl_moves_backup);
        return best;
    }

    if (DEBUG) printf_moves(gl.moves, mvs_len, "BEFORE QSORT\n");
    if (is_quies) {
        qsort(gl.moves, mvs_len, sizeof(Move *), mvv_lva_cmp);
    } else {
        qsort(gl.moves, mvs_len, sizeof(Move *), mvv_lva_cmp);
        // TODO: Try for a killer heuristic that improves on MVV-LVA
        //qsort(gl.moves, mvs_len, sizeof(Move *), killer_cmp);
    }
    if (DEBUG) printf_moves(gl.moves, mvs_len, "AFTER QSORT\n");

    while (gl.moves != gl.movesend) {

        if (!*(gl.moves)) {
            gl.moves++;
            continue;
        }

        Move mv = **(gl.moves);

        if (depth < 2 and DEBUG) {
            for (int i = 0; i < depth; i++) printf("    ");
            printf_move(mv); printf(" ADAPT %ld \n", adaptive);
        }

        PiecePlusCatling ppc = make_move(mv);

        ValuePlusMove rec = alphabeta(
            COLOR == WHITE ? BLACK : WHITE,
            alpha,
            beta,
            adaptive - eval_adaptive_depth(COLOR, mv, ppc.hit_piece, is_quies),
            is_quies,
            depth + 1,
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

        if (not is_quies and not (lines and depth == 0)) {
            if (COLOR == WHITE and best.value > alpha)  // lo bound
                alpha = best.value;
            if (COLOR == BLACK and best.value < beta)  // hi bound
                beta = best.value;
            if (beta <= alpha) {  // alpha-beta cutoff
                if (KILLERHEURISTIC.find(mv) == KILLERHEURISTIC.end())
                    KILLERHEURISTIC[mv] = 0;
                KILLERHEURISTIC[mv] = depth * depth;
                break;
            }
        }

        // TODO: combine both cases
        // quiescence cutoff
        if (is_quies and COLOR == WHITE) {
            if (best.value >= beta) {
                best.value = beta;

                for (int i = 0; i < 128; ++i)
                    if (gl_moves_backup[i])
                        free(gl_moves_backup[i]);
                free(gl_moves_backup);

                if (best.move.f0 or best.move.f1 or best.move.t0 or best.move.t1)
                    best.variation.push_front(best.move);

                return best;
            }
            if (best.value > alpha)
                alpha = best.value;
        }
        if (is_quies and COLOR == BLACK) {
            if (best.value <= alpha) {
                best.value = alpha;

                for (int i = 0; i < 128; ++i)
                    if (gl_moves_backup[i])
                        free(gl_moves_backup[i]);
                free(gl_moves_backup);

                if (best.move.f0 or best.move.f1 or best.move.t0 or best.move.t1)
                    best.variation.push_front(best.move);

                return best;
            }
            if (best.value < beta)
                beta = best.value;
        }

        gl.moves++;
    }

    for (int i = 0; i < 128; ++i)
        if (gl_moves_backup[i])
            free(gl_moves_backup[i]);
    free(gl_moves_backup);

    if (best.move.f0 or best.move.f1 or best.move.t0 or best.move.t1)
        best.variation.push_front(best.move);

    if (is_quies and COLOR == WHITE)
        best.value = alpha;
    if (is_quies and COLOR == BLACK)
        best.value = beta;

    return best;
}


// find and play the best move in the position
std::string calc_move(bool lines = false) {
    // TODO: mode where a random/suboptimal move can get picked?
    NODES_NORMAL = 0;
    NODES_QUIES = 0;
    KILLERHEURISTIC.clear();

    // Have to re-calculate board info anew each time because GUI/Lichess might reset state
    board_eval = initial_eval();

    num my_color = IM_WHITE ? WHITE : BLACK;
    num queens_on_board = 0;
    num own_pieces_on_board = 0;
    gentuples { queens_on_board += !!(board[8*i+j] & QUEEN); }
    gentuples { own_pieces_on_board += !!((board[8*i+j] & my_color) and (board[8*i+j] & (ROOK | BISHOP | KNIGHT))); }

    if (queens_on_board == 0 and own_pieces_on_board <= 2) {
        printf("Late endgame: Bringing in pawns and king\n");
        PIECE_SQUARE_TABLES[KING] = PIECE_SQUARE_TABLES[KING_ENDGAME];
        PIECE_SQUARE_TABLES[PAWN] = PIECE_SQUARE_TABLES[PAWN_ENDGAME];
        set_time_control(TIME_CONTROL_REQUESTED + 2);  // without queens search is faster, so search more nodes
    } else if (queens_on_board == 0) {
        printf("Early endgame: Queens traded, taking longer thinks\n");
        PIECE_SQUARE_TABLES[KING] = PIECE_SQUARE_TABLES[KING_EARLYGAME];
        PIECE_SQUARE_TABLES[PAWN] = PIECE_SQUARE_TABLES[PAWN_EARLYGAME];
        set_time_control(TIME_CONTROL_REQUESTED + 1);  // without queens search is faster, so search more nodes
    } else {
        printf("Earlygame (opening or middlegame): Queens still on board\n");
        PIECE_SQUARE_TABLES[KING] = PIECE_SQUARE_TABLES[KING_EARLYGAME];
        PIECE_SQUARE_TABLES[PAWN] = PIECE_SQUARE_TABLES[PAWN_EARLYGAME];
        set_time_control(TIME_CONTROL_REQUESTED);
    }

    printf("Starting alphabeta with depth %ld adaptive %ld\n", SEARCH_DEPTH, SEARCH_ADAPTIVE_DEPTH);

    ValuePlusMove bestmv = alphabeta(my_color, -inf+1, inf-1, SEARCH_ADAPTIVE_DEPTH, false, 0, lines);
    Move mv = bestmv.move;

    printf("--> BEST ");
    printf_move(mv);
    printf("\n");
    printf("Number of nodes searched: %ld normal %ld quiescent\n", NODES_NORMAL, NODES_QUIES);

    make_move(mv);

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

    std::cout << "Tests Stalemate" << std::endl;

    board_clear();
    set_up_kings(0, 2, 0, 0);
    board[2*8+1] = WHITE + PAWN;
    pprint();
    std::cout << calc_move(true) << std::endl;

    std::cout << "Tests Promotion" << std::endl;

    board_clear();
    set_up_kings(0, 2, 0, 0);
    board[1*8+6] = WHITE + PAWN;
    pprint();
    std::cout << calc_move(true) << std::endl;

    std::cout << "Tests En-Passant" << std::endl;

    board_initial_position();
    make_move_str("e2e4"); make_move_str("a7a6");
    make_move_str("e4e5"); make_move_str("d7d5");
    pprint();
    std::cout << calc_move(true) << std::endl;

    std::cout << "Tests Castling" << std::endl;

    board_initial_position();
    board[7*8+5] = 0;
    board[7*8+6] = 0;
    board[6*8+4] = 0;
    pprint();
    std::cout << calc_move(true) << std::endl;

    std::cout << "Tests Lasker Trap in Albin Countergambit" << std::endl;

    board_initial_position();
    IM_WHITE = false;
    make_move_str("d2d4"); make_move_str("d7d5");
    make_move_str("c2c4"); make_move_str("e7e5");
    make_move_str("d4e5"); make_move_str("d5d4");
    make_move_str("e2e3"); make_move_str("f8b4");
    make_move_str("c1d2"); make_move_str("d4e3");
    make_move_str("d2b4"); make_move_str("e3f2");
    make_move_str("e1e2");
    pprint();
    std::cout << calc_move(true) << std::endl;

    std::cout << "Tests missed fork from a game" << std::endl;

    board_initial_position();
    IM_WHITE = false;
    make_move_str("d2d4"); make_move_str("b8c6");
    make_move_str("c2c4"); make_move_str("g8f6");
    make_move_str("d4d5"); make_move_str("c6e5");
    make_move_str("d1c2"); make_move_str("e5g6");
    make_move_str("b1c3"); make_move_str("e7e5");
    make_move_str("e2e4"); make_move_str("f8d6");
    make_move_str("g1f3"); make_move_str("c7c6");
    make_move_str("c1g5"); make_move_str("c6d5");
    make_move_str("e4d5"); make_move_str("g6f4");
    make_move_str("g2g3"); make_move_str("f4g6");
    make_move_str("f1g2"); make_move_str("e8g8");
    make_move_str("c3e4"); make_move_str("d6b4");
    make_move_str("e4c3"); make_move_str("d7d6");
    make_move_str("e1g1"); make_move_str("b4c5");
    make_move_str("a2a3"); make_move_str("h7h6");
    make_move_str("g5f6"); make_move_str("d8f6");
    make_move_str("c3e4"); make_move_str("f6f5");
    make_move_str("a1c1"); make_move_str("c8d7");
    make_move_str("e4c5"); make_move_str("d6c5");
    make_move_str("f1e1"); make_move_str("f5c2");
    make_move_str("c1c2"); make_move_str("d7f5");
    make_move_str("c2c3"); make_move_str("a8e8");
    make_move_str("b2b4"); make_move_str("b7b6");
    make_move_str("f3d2"); make_move_str("c5b4");
    make_move_str("a3b4"); make_move_str("e8d8");
    make_move_str("d2e4"); make_move_str("f8e8");
    make_move_str("c3e3");
    pprint();
    std::cout << calc_move(true) << std::endl;

    board_initial_position();
    IM_WHITE = true;
    printf("INITIAL BOARD EVAL: %ld\n", board_eval);

    make_move_str("e2e4");
    printf("1. e4 EVAL: %ld\n", board_eval);

    printf("LIST OF MOVES IN RESPONSE TO 1. e4\n");
    pprint();
    alphabeta(BLACK, -inf+1, inf-1, SEARCH_ADAPTIVE_DEPTH, false, 0, true);

    IM_WHITE = true;
    board_initial_position();
    make_move_str("h2h4"); make_move_str("b7b6");
    make_move_str("g1f3"); make_move_str("c8a6");
    make_move_str("d2d4"); make_move_str("e7e6");
    make_move_str("b1d2"); make_move_str("d8e7");
    make_move_str("c2c4"); make_move_str("a6b7");
    make_move_str("d1b3"); make_move_str("e7f6");
    make_move_str("b3b5"); make_move_str("b7c6");
    make_move_str("b5h5"); make_move_str("f8b4");
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
        if (strcmp(line, "xboard\n") == 0) {
            printf("feature myname=\"jangine\"\n");
            // printf("feature sigint=0 sigterm=0\n");
            printf("feature ping=1\n");
            printf("feature setboard=1\n");
            printf("feature done=1\n");
            MODE_UCI = false;
        }
        if (startswith("time", line)) {
            int time_left = std::stoi(line_cpp.substr(5));
            TIME_CONTROL_REQUESTED = HYPERHYPERHYPERBULLET;                    // just move when under 2 seconds
            if (time_left >=   200) TIME_CONTROL_REQUESTED = HYPERHYPERBULLET; // play hyperhyperbullet-like when 2 seconds left
            if (time_left >=   500) TIME_CONTROL_REQUESTED = HYPERBULLET;      // play hyperbullet-like when 5 seconds left
            if (time_left >=  1500) TIME_CONTROL_REQUESTED = BULLET;           // play bullet-like when 15 seconds left
            if (time_left >=  4500) TIME_CONTROL_REQUESTED = BLITZ;            // play blitz-like when 45 seconds left
            if (time_left >= 18000) TIME_CONTROL_REQUESTED = RAPID;            // play rapid-like when 3+ minutes left
            if (time_left >= 48000) TIME_CONTROL_REQUESTED = CLASSICAL;        // play classical-like when 8+ mins left
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
            num_moves = 0;

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
