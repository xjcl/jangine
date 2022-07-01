#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdbool>
#include <cstdint>
#include <cinttypes>
#include <cmath>
#include <ctime>
#include <random>
#include <unistd.h>
#include <map>
#include <set>
#include <deque>
#include <vector>
#include <sstream>
#include <algorithm>


typedef int_fast16_t num;

#define DEBUG 0  // 1: debug PV, 2: all lines, 3: tons of output
#define OUTPUT_TIME 1

#define SIMPLE_EVAL  // TODO: just-material eval, piece-square-eval, and turochamp eval
#define NO_QUIES 0
#define QUIES_DEPTH 0  // TODO: limit search of quiescence captures
#define MAX_KILLER_MOVES 2
#define NO_PRINCIPAL_VARIATION_SEARCH 1 // seems to actually be slower since my leaf eval is so cheap

#define is_inside(i, j) (0 <= i and i <= 7 and 0 <= j and j <= 7)
#define gentuples for (num i = 0; i < 8; ++i) for (num j = 0; j < 8; ++j)
#pragma GCC diagnostic ignored "-Wnarrowing"

bool input_is_move(const char* s) {
    if (strlen(s) < 5 || strlen(s) > 6)  // newline character
        return false;

    return 'a' <= s[0] and s[0] <= 'h' and 'a' <= s[2] && s[2] <= 'h' and
            '1' <= s[1] && s[1] <= '8' and '1' <= s[3] && s[3] <= '8';
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
num PIECE_SQUARE_TABLES[161][64] = {0};  // C array is much faster to query than a C++ map
std::map<num, std::array<num, 64>> PIECE_SQUARE_TABLES_SOURCE = {
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
                 60,  80,  80,  80,  80,  80,  80,  60,
                 20,  40,  40,  40,  40,  40,  40,  40,
                  0,   0,   0,  10,  10,  10,  10,  10,
                  5,   0,  10,  30,  30,   0,   0,   0,
                  5,   0,   0,   0,   0,   0,   0,   0,
                  0,   0,   0, -20, -20,  30,  30,  15,
                  0,   0,   0,   0,   0,   0,   0,   0,
        }},
        {PAWN_ENDGAME, std::array<num, 64>{
                  0,   0,   0,   0,   0,   0,   0,   0,
                 80, 100, 100, 100, 100, 100, 100,  80,
                 40,  60,  60,  60,  60,  60,  60,  40,
                 30,  40,  40,  40,  40,  40,  40,  30,
                 10,  20,  20,  20,  20,  20,  20,  10,
                  5,  10,  10,  10,  10,  10,  10,   5,
                  0,   0,   0,   0,   0,   0,   0,   0,
                  0,   0,   0,   0,   0,   0,   0,   0,
        }},
        {KNIGHT, std::array<num, 64>{
                -60,-40,-30,-30,-30,-30,-40,-60,
                -40,-20,  0,  0,  0,  0,-20,-40,
                -30,  0, 10, 15, 15, 10,  0,-30,
                -30,  0, 15, 20, 20, 15,  0,-30,
                -30,  0, 15, 20, 20, 15,  0,-30,
                -30,  5, 15, 15, 15, 15,  5,-30,
                -40,-20,  0, 10, 10,  0,-20,-40,
                -60,-40,-30,-30,-30,-30,-40,-60,
        }},
        // bishop isn't actually bad on the bad row but prevents castling
        {BISHOP, std::array<num, 64>{
                -20,-10,-10,-10,-10,-10,-10,-20,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -10,  0,  5, 10, 10,  5,  0,-10,
                -10, 15,  5, 10, 10,  5, 15,-10,
                -10,  0, 15, 10, 10, 15,  0,-10,
                -10, 10, 10, 10, 10, 10, 10,-10,
                -10,  5,  0,  0,  0,  0,  5,-10,
                -20,-10,-10,-10,-10,-20,-10,-20,
        }},
        {ROOK, std::array<num, 64>{
                 -5,  0,  0,  0,  0,  0,  0, -5,
                 -5, 20, 20, 20, 20, 20, 20, -5,
                 -5,  0,  0, 10, 10,  0,  0, -5,
                 -5,  0,  0, 10, 10,  0,  0, -5,
                 -5,  0,  0, 10, 10,  0,  0, -5,
                 -5,  0,  0, 10, 10,  0,  0, -5,
                 -5,  0,  0, 10, 10,  0,  0, -5,
                 -5,  0,  0, 10, 10,  0,  0, -5,
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
                 20, 20, 10,  0,  0,  0, 40, 40,
                 20, 30, 20,  0, 10,  0, 50, 40   // strongly encourage short castling
        }},
        {KING_ENDGAME, std::array<num, 64>{
                 10, 20, 30, 40, 40, 30, 20, 10,
                 20, 30, 40, 40, 40, 40, 30, 20,
                 30, 40, 50, 50, 50, 50, 40, 30,
                 20, 30, 40, 50, 50, 40, 30, 20,
                 10, 10, 30, 40, 40, 30, 10, 10,
                  0,  0, 20, 30, 30, 20,  0,  0,
                -30,-30,  0,  0,  0,  0,-30,-30,
                -30,-30,-30,-30,-30,-30,-30,-30   // encourage going to the center
        }}
};

typedef struct CASTLINGRIGHTS {
    bool lc;  // left-castling still possible (long castling)
    bool rc;  // right-castling still possible (short castling)
} CASTLINGRIGHTS;

typedef struct Move {
    int8_t f0;  // from
    int8_t f1;
    int8_t t0;  // to
    int8_t t1;
    char prom;  // promote to piece

    // needed for std::map
    bool operator<( const Move & that ) const {
        return (64 * (8*this->t0 + this->t1) + 8*this->f0 + this->f1) << 8 + this->prom <
            (64 * (8*that.t0 + that.t1) + 8*that.f0 + that.f1) << 8 + that.prom;
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

CASTLINGRIGHTS CASTLINGWHITE = {true, true};
CASTLINGRIGHTS CASTLINGBLACK = {true, true};

Move NULLMOVE = {0};
Move LASTMOVE = {0};
Move LASTMOVE_GAME = {0};
Move TRANSPOS_TABLE[16777216] = {0};  // 24 bits -> (8+4) * 2**24 bytes = 192 MiB
int64_t TRANSPOS_TABLE_ZOB[16777216] = {0};
#define ZOB_MASK 0xffffff

struct Pair {
    num a;
    num b;
};

Pair PIECEDIRS[65][9] = {0};
num PIECERANGE[65] = {0};
num PIECEVALS[65] = {0};

num NODE_DEPTH = 0;
Move KILLER_TABLE[20][MAX_KILLER_MOVES] = {0};  // table of killer moves for each search depth

num board[64] = {0};
num board_eval = 0;  // takes about 10% of compute time
bool IM_WHITE = false;
bool started = true;
bool MODE_UCI = false;

int64_t NODES_NORMAL = 0;
int64_t NODES_QUIES = 0;
std::clock_t SEARCH_START_CLOCK;

void pprint() {
    for (num i = 0; i < 8; ++i) {
        printf("  %ld ", (8 - i));
        for (num j = 0; j < 8; ++j) {
            std::string s = id_to_unicode[board[8 * i + j]];
            printf("%s ", s.c_str());
        }
        printf(" \n");
    }
    printf("    a b c d e f g h\n\n");
}

bool startswith(const char *pre, const char *str) {
    size_t lenpre = strlen(pre), lenstr = strlen(str);
    return lenstr >= lenpre and strncmp(pre, str, lenpre) == 0;
}

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

int64_t zobrint_random_table[161][64] = {0};
std::set<int64_t> board_positions_seen;
int64_t zobrint_hash = 0;

void init_zobrint() {
    std::mt19937_64 e2(5489u);  // fixed default seed
    std::uniform_int_distribution<long long int> dist(0);  // undefined for in64_t :/

    std::array<num, 13> pieces = {
        1,  // keep zobrint_random_table[1][0] for who to move
        WHITE + PAWN, WHITE + KNIGHT, WHITE + BISHOP, WHITE + ROOK, WHITE + QUEEN, WHITE + KING,
        BLACK + PAWN, BLACK + KNIGHT, BLACK + BISHOP, BLACK + ROOK, BLACK + QUEEN, BLACK + KING,
    };

    for (int i = 0; i < 64; ++i)
        zobrint_random_table[0][i] = 0;  // fill level 0 with 0s to ensure empty squares have no effect

    for (const num& piece : pieces)
        for (int i = 0; i < 64; ++i)
            zobrint_random_table[piece][i] = dist(e2);
}

int64_t board_to_zobrint_hash() {
    int64_t zobrint_hash_ = 0;

    for (int i = 0; i < 64; ++i)
        zobrint_hash_ ^= zobrint_random_table[board[i]][i];

    if (not IM_WHITE)
        zobrint_hash_ ^= zobrint_random_table[1][0];

    // TODO: Add en passant ranks and castling rights
    // Okay to implement this without castling rights and en passant at first
    //  because this is only used for repetition detection and PV store, not for actual transpositions
    // We just want to avoid voluntarily going into repeated positions in otherwise winning endgames

    return zobrint_hash_;
}

void set_up_kings(num wi, num wj, num bi, num bj) {
    board[8*wi+wj] = KING + WHITE;
    board[8*bi+bj] = KING + BLACK;
    KINGPOS_WHITE_i = wi;
    KINGPOS_WHITE_j = wj;
    KINGPOS_BLACK_i = bi;
    KINGPOS_BLACK_j = bj;
}

void set_piece_square_table(bool is_endgame = false) {
    for (int j = 0; j < 64; j++) {
        PIECE_SQUARE_TABLES[KNIGHT][j] = PIECE_SQUARE_TABLES_SOURCE[KNIGHT][j];
        PIECE_SQUARE_TABLES[BISHOP][j] = PIECE_SQUARE_TABLES_SOURCE[BISHOP][j];
        PIECE_SQUARE_TABLES[  ROOK][j] = PIECE_SQUARE_TABLES_SOURCE[  ROOK][j];
        PIECE_SQUARE_TABLES[ QUEEN][j] = PIECE_SQUARE_TABLES_SOURCE[ QUEEN][j];

        PIECE_SQUARE_TABLES[KING][j] = PIECE_SQUARE_TABLES_SOURCE[is_endgame ? KING_ENDGAME : KING_EARLYGAME][j];
        PIECE_SQUARE_TABLES[PAWN][j] = PIECE_SQUARE_TABLES_SOURCE[is_endgame ? PAWN_ENDGAME : PAWN_EARLYGAME][j];
    }
}

void board_initial_position() {  // setting up a game
    for (int i = 0; i < 64; ++i)
        board[i] = default_board[i];

    set_piece_square_table();

    set_up_kings(7, 4, 0, 4);
    CASTLINGWHITE = {true, true};
    CASTLINGBLACK = {true, true};
    board_eval = 0;

    zobrint_hash = board_to_zobrint_hash();
    board_positions_seen.clear();
    board_positions_seen.insert(zobrint_hash);
}

void board_clear() {  // setting up random positions
    board_initial_position();
    zobrint_hash = 0;

    for (int i = 0; i < 64; ++i)
        board[i] = 0;

    CASTLINGWHITE = {false, false};
    CASTLINGBLACK = {false, false};
}

void board_from_fen(const char* fen) {  // setting up a game
    const char* c = fen;
    board_clear();
    num i = 0;
    num j = 0;

    while (*c != ' ') {
        if (*c == '/') { i++; j = -1; }
        if (*c == '1')  j += 0;
        if (*c == '2')  j += 1;
        if (*c == '3')  j += 2;
        if (*c == '4')  j += 3;
        if (*c == '5')  j += 4;
        if (*c == '6')  j += 5;
        if (*c == '7')  j += 6;
        if (*c == '8')  j += 7;
        if (*c == 'P')  board[8*i+j] = WHITE + PAWN;
        if (*c == 'p')  board[8*i+j] = BLACK + PAWN;
        if (*c == 'N')  board[8*i+j] = WHITE + KNIGHT;
        if (*c == 'n')  board[8*i+j] = BLACK + KNIGHT;
        if (*c == 'B')  board[8*i+j] = WHITE + BISHOP;
        if (*c == 'b')  board[8*i+j] = BLACK + BISHOP;
        if (*c == 'R')  board[8*i+j] = WHITE + ROOK;
        if (*c == 'r')  board[8*i+j] = BLACK + ROOK;
        if (*c == 'Q')  board[8*i+j] = WHITE + QUEEN;
        if (*c == 'q')  board[8*i+j] = BLACK + QUEEN;
        if (*c == 'K')  board[8*i+j] = WHITE + KING;
        if (*c == 'k')  board[8*i+j] = BLACK + KING;
        j++;
        *c++;
    }

    *c++;
    IM_WHITE = (*c == 'w');

    // ignore castling for now
    // ignore en passant and halfmove/fullmove clocks for now
    zobrint_hash = board_to_zobrint_hash();
}

num SEARCH_ADAPTIVE_DEPTH = 6;  // how many plies to search

num MAX_SEARCH_DEPTH = 99999;
num OWN_CLOCK_REMAINING = 18000;

typedef struct ValuePlusMoves {
    num value;
    Move** moves;
    Move** movesend;
} ValuePlusMoves;

typedef struct ValuePlusMove {
    num value;
    Move move;
} ValuePlusMove;


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


num eval_material(num piece_with_color) {
    num piece = piece_with_color & COLORBLIND;
    return piece_with_color & WHITE ? PIECEVALS[piece] : -PIECEVALS[piece];
}

num eval_piece_on_square(num piece_with_color, num i, num j) {
    num piece = piece_with_color & COLORBLIND;
    return piece_with_color & WHITE ? PIECE_SQUARE_TABLES[piece][8*i+j] : -PIECE_SQUARE_TABLES[piece][8*(7-i)+j];
}

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
    // piece positioning using piece-square-tables
    gentuples {
        eval += eval_piece_on_square(board[8*i+j], i, j);
    }

    return eval;
}


PiecePlusCatling make_move(Move mv)
{
    if (mv == NULLMOVE)
        printf("XXX DANGEROUS! NULL MOVE\n");

    num piece = board[8*mv.f0+mv.f1];
    num hit_piece = board[8*mv.t0+mv.t1];
    board[8*mv.f0+mv.f1] = 0;
    board[8*mv.t0+mv.t1] = piece;

    num CASTLERANK = piece & WHITE ? 7 : 0;

    board_eval -= eval_material(hit_piece) + eval_piece_on_square(hit_piece, mv.t0, mv.t1);  // remove captured piece. ignores empty squares (0s)  // always empty for en passant
    board_eval += eval_piece_on_square(piece, mv.t0, mv.t1) - eval_piece_on_square(piece, mv.f0, mv.f1);  // adjust position values of moved piece

    zobrint_hash ^= zobrint_random_table[piece][8*mv.f0+mv.f1];  // xor OUT piece at   initial   square
    zobrint_hash ^= zobrint_random_table[piece][8*mv.t0+mv.t1];  // xor  IN piece at destination square
    zobrint_hash ^= zobrint_random_table[hit_piece][8*mv.t0+mv.t1];  // xor OUT captured piece (0 if square empty)

    if (mv.prom) {
        if (mv.prom == 'e') {  // en passant
            board_eval -= eval_material(board[8*mv.f0+mv.t1]) + eval_piece_on_square(board[8*mv.f0+mv.t1], mv.f0, mv.t1);  // captured pawn did not get removed earlier
            zobrint_hash ^= zobrint_random_table[board[8*mv.f0+mv.t1]][8*mv.f0+mv.t1];  // xor OUT captured pawn
            board[8*mv.f0+mv.t1] = 0;
        } else if (mv.prom == 'c') {  // castling -- rook part
            if (mv.t1 == 2) {  // long (left)
                board[8*CASTLERANK+3] = board[8*CASTLERANK];
                board[8*CASTLERANK] = 0;
                board_eval += eval_piece_on_square(board[8*CASTLERANK+3], CASTLERANK, 3) - eval_piece_on_square(board[8*CASTLERANK+3], CASTLERANK, 0);  // rook positioning
                zobrint_hash ^= zobrint_random_table[board[8*CASTLERANK+3]][8*CASTLERANK  ];  // xor OUT old rook position
                zobrint_hash ^= zobrint_random_table[board[8*CASTLERANK+3]][8*CASTLERANK+3];  // xor  IN new rook position
            } else {  // short (right)
                board[8*CASTLERANK+5] = board[8*CASTLERANK+7];
                board[8*CASTLERANK+7] = 0;
                board_eval += eval_piece_on_square(board[8*CASTLERANK+5], CASTLERANK, 5) - eval_piece_on_square(board[8*CASTLERANK+5], CASTLERANK, 7);  // rook positioning
                zobrint_hash ^= zobrint_random_table[board[8*CASTLERANK+5]][8*CASTLERANK+7];  // xor OUT old rook position
                zobrint_hash ^= zobrint_random_table[board[8*CASTLERANK+5]][8*CASTLERANK+5];  // xor  IN new rook position
            }
        } else {  // pawn promotion
            num p = (mv.t0 == 0 ? WHITE : BLACK);
            switch (mv.prom) {
                case 'q':  p += QUEEN;   break;
                case 'r':  p += ROOK;    break;
                case 'b':  p += BISHOP;  break;
                case 'n':  p += KNIGHT;  break;
            }
            board[8*mv.t0+mv.t1] = p;
            board_eval += eval_material(p) - eval_material(piece);
            board_eval += eval_piece_on_square(p, mv.t0, mv.t1) - eval_piece_on_square(piece, mv.t0, mv.t1);  // t0/t1 to compensate for wrong pieceval earlier
            zobrint_hash ^= zobrint_random_table[piece][8*mv.t0+mv.t1];  // xor OUT promoting pawn
            zobrint_hash ^= zobrint_random_table[    p][8*mv.t0+mv.t1];  // xor  IN promoted piece
        }
    }

    CASTLINGRIGHTS old_cr_w = CASTLINGWHITE;
    CASTLINGRIGHTS old_cr_b = CASTLINGBLACK;

    CASTLINGRIGHTS cr = (piece & WHITE ? CASTLINGWHITE : CASTLINGBLACK);

    if (mv.f0 == CASTLERANK) {  // lose right to castle if king or rook moves
        cr = {mv.f1 != 0 and mv.f1 != 4 and cr.lc, mv.f1 != 7 and mv.f1 != 4 and cr.rc};
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

    zobrint_hash ^= zobrint_random_table[1][0];  // switch side to move

    return {hit_piece, old_cr_w, old_cr_b};
}

void unmake_move(Move mv, num hit_piece, CASTLINGRIGHTS c_rights_w, CASTLINGRIGHTS c_rights_b)
{
    num piece = board[8*(mv.t0)+mv.t1];
    board[8*(mv.t0)+mv.t1] = hit_piece;
    board[8*(mv.f0)+mv.f1] = piece;

    num CASTLERANK = piece & WHITE ? 7 : 0;

    board_eval += eval_material(hit_piece) + eval_piece_on_square(hit_piece, mv.t0, mv.t1);  // add captured piece back in
    board_eval -= eval_piece_on_square(piece, mv.t0, mv.t1) - eval_piece_on_square(piece, mv.f0, mv.f1);  // adjust position values of moved piece

    zobrint_hash ^= zobrint_random_table[piece][8*mv.f0+mv.f1];
    zobrint_hash ^= zobrint_random_table[piece][8*mv.t0+mv.t1];
    zobrint_hash ^= zobrint_random_table[hit_piece][8*mv.t0+mv.t1];

    if (mv.prom)
    {
        if (mv.prom == 'e') {  // en passant
            board[8*mv.f0+mv.t1] = PAWN + (mv.t0 == 5 ? WHITE : BLACK);
            board_eval += eval_material(board[8*mv.f0+mv.t1]) + eval_piece_on_square(board[8*mv.f0+mv.t1], mv.f0, mv.t1);  // captured pawn did not get added earlier
            zobrint_hash ^= zobrint_random_table[board[8*mv.f0+mv.t1]][8*mv.f0+mv.t1];
        } else if (mv.prom == 'c') {  // castling -- undo rook move
            if (mv.t1 == 2) {  // long (left)
                board[8*mv.f0] = board[8*mv.f0+3];
                board[8*mv.f0+3] = 0;
                board_eval += eval_piece_on_square(board[8*mv.f0], mv.f0, 0) - eval_piece_on_square(board[8*mv.f0], mv.f0, 3);  // rook positioning
                zobrint_hash ^= zobrint_random_table[board[8*CASTLERANK  ]][8*CASTLERANK  ];
                zobrint_hash ^= zobrint_random_table[board[8*CASTLERANK  ]][8*CASTLERANK+3];
            }
            else {  // short (right)
                board[8*mv.f0+7] = board[8*mv.f0+5];
                board[8*mv.f0+5] = 0;
                board_eval += eval_piece_on_square(board[8*mv.f0+7], mv.f0, 7) - eval_piece_on_square(board[8*mv.f0+7], mv.f0, 5);  // rook positioning
                zobrint_hash ^= zobrint_random_table[board[8*CASTLERANK+7]][8*CASTLERANK+7];
                zobrint_hash ^= zobrint_random_table[board[8*CASTLERANK+7]][8*CASTLERANK+5];
            }
        } else {  // promotion
            num old_pawn = PAWN + (mv.t0 == 0 ? WHITE : BLACK);
            board[8*mv.f0+mv.f1] = old_pawn;
            board_eval -= eval_material(piece) - eval_material(old_pawn);
            board_eval -= eval_piece_on_square(piece, mv.f0, mv.f1) - eval_piece_on_square(old_pawn, mv.f0, mv.f1);  // t0/t1 to compensate for wrong pieceval earlier
            zobrint_hash ^= zobrint_random_table[old_pawn][8*mv.f0+mv.f1];
            zobrint_hash ^= zobrint_random_table[   piece][8*mv.f0+mv.f1];
        }
    }

    CASTLINGWHITE = c_rights_w;
    CASTLINGBLACK = c_rights_b;

    if (piece == WHITE + KING) {
        KINGPOS_WHITE_i = mv.f0;
        KINGPOS_WHITE_j = mv.f1;
    }
    else if (piece == BLACK + KING) {
        KINGPOS_BLACK_i = mv.f0;
        KINGPOS_BLACK_j = mv.f1;
    }

    zobrint_hash ^= zobrint_random_table[1][0];  // switch side to move
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
    LASTMOVE_GAME = to_mv;  // copies the struct

    board_positions_seen.insert(board_to_zobrint_hash());
}

void printf_move(Move mv) {
    std::string move_str = move_to_str(mv, true);
    const char* cstr = move_str.c_str();
    printf("MOVE %15s |", cstr);
}

void printf_move_eval(ValuePlusMove rec, bool accurate)  // print eval of move (before call to make_move)
{
    double time_expired = OUTPUT_TIME ? (1.0 * (std::clock() - SEARCH_START_CLOCK) / CLOCKS_PER_SEC) : 0;
    printf(" EVAL %7.2f %c | D%2ld | NN%9ld | NQ%9ld | t%7.3f | VAR  ... ",
        (float)(rec.value) / 100, accurate ? ' ' : '?', SEARCH_ADAPTIVE_DEPTH, NODES_NORMAL, NODES_QUIES, time_expired);

    // reveal PV from hash table, this saves work of returning a std::deque
    std::deque<Move> moves;
    std::deque<PiecePlusCatling> ppcs;

    PiecePlusCatling ppc = make_move(rec.move);
    moves.push_back(rec.move);
    ppcs.push_back(ppc);
    num i = 0;  // prevent infinite repetition

    // TODO: revealing PV from hash table is faster but cuts off parts of it
    while ((TRANSPOS_TABLE_ZOB[zobrint_hash & ZOB_MASK] == zobrint_hash) && (i < 12)) {
        Move mv = TRANSPOS_TABLE[zobrint_hash & ZOB_MASK];
        std::cout << move_to_str(mv, true) << " ";

        ppc = make_move(mv);  // updates zobrist_hash
        moves.push_back(mv);
        ppcs.push_back(ppc);
        ++i;
    }

    if ((TRANSPOS_TABLE_ZOB[zobrint_hash & ZOB_MASK] != zobrint_hash) and (TRANSPOS_TABLE_ZOB[zobrint_hash & ZOB_MASK] != 0)
            and not (TRANSPOS_TABLE[zobrint_hash & ZOB_MASK] == NULLMOVE))
        std::cout << "[HASH COLLISION] ";

    for (int i = moves.size() - 1; i >= 0; i--) {
        ppc = ppcs[i];
        unmake_move(moves[i], ppc.hit_piece, ppc.c_rights_w, ppc.c_rights_b);
    }

    std::cout << std::endl;
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
        else
            printf("%ld MISSING\n", i);
    }
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


// TODO TODO TODO replace with vector push_back OR AT LEAST fewer indirect mallocs
inline Move** move_store(Move** mvsend, num COLOR, num a, num b, num c, num d, num e)
{
    *mvsend = (Move*)malloc(sizeof(Move));
    (*mvsend)->f0 = a; (*mvsend)->f1 = b; (*mvsend)->t0 = c; (*mvsend)->t1 = d; (*mvsend)->prom = e;
    return ++mvsend;
}

inline Move** move_store_maybe_promote(Move** mvsend, num COLOR, bool is_promrank, num a, num b, num c, num d)
{
    if (is_promrank) {
        mvsend = move_store(mvsend, COLOR, a, b, c, d, 'q'); /* best */
        mvsend = move_store(mvsend, COLOR, a, b, c, d, 'n'); /* likely */
        mvsend = move_store(mvsend, COLOR, a, b, c, d, 'r');
        return   move_store(mvsend, COLOR, a, b, c, d, 'b');
    } else
        return   move_store(mvsend, COLOR, a, b, c, d, '\0');
}


ValuePlusMoves gen_moves_maybe_legal(num COLOR, bool only_captures = false)
{
    Move** mvs = (Move**)calloc(128, sizeof(Move*));  // Maximum should be 218 moves  // also inits to NULLptrs
    Move** mvsend = mvs;

    num NCOLOR = COLOR == WHITE ? BLACK : WHITE;
    num ADD = COLOR == WHITE ? -1 : 1;  // "up"
    num STARTRANK = COLOR == WHITE ? 6 : 1;
    num PROMRANK = COLOR == WHITE ? 1 : 6;
    num EPRANK = COLOR == WHITE ? 3 : 4;
    num CASTLERANK = COLOR == WHITE ? 7 : 0;

    if (not only_captures) {  // castling moves
        //    TODO: FIX THIS!!! TRIES TO CASTLE WHEN BISHOP ON C8
        //    INPUT: position startpos moves d2d4 b8c6 e2e4 d7d5 e4d5 d8d5 g1f3 d5e4 f1e2
        //    MOVE   B f5  (c8f5 ) | KH        4 | EVAL    0.45   | VAR  ...   c4  (c2c4 ) Kxc8  (e8c8c)
        CASTLINGRIGHTS castlingrights = COLOR == WHITE ? CASTLINGWHITE : CASTLINGBLACK;
        if (castlingrights.rc and board[8*CASTLERANK+4] == COLOR + KING and board[8*CASTLERANK+7] == COLOR + ROOK) {  // short castle O-O
            if (not board[8*CASTLERANK+5] and not board[8*CASTLERANK+6] and
                    not square_in_check(COLOR, CASTLERANK, 4) and not square_in_check(COLOR, CASTLERANK, 5) and not square_in_check(COLOR, CASTLERANK, 6))
                mvsend = move_store(mvsend, COLOR, CASTLERANK, 4, CASTLERANK, 6, 'c');
        }
        if (castlingrights.lc and board[8*CASTLERANK+4] == COLOR + KING and board[8*CASTLERANK+0] == COLOR + ROOK) {  // long castle O-O-O
            if (not board[8*CASTLERANK+1] and not board[8*CASTLERANK+2] and not board[8*CASTLERANK+3] and
                    not square_in_check(COLOR, CASTLERANK, 2) and not square_in_check(COLOR, CASTLERANK, 3) and not square_in_check(COLOR, CASTLERANK, 4))
                mvsend = move_store(mvsend, COLOR, CASTLERANK, 4, CASTLERANK, 2, 'c');
        }
    }

    gentuples {
        num bij = board[8*i+j];
        num bijpiece = bij & COLORBLIND;
        if (not (bij & COLOR))
            continue;
        if (bij & PAWN) {  // pawn moves
            // diagonal captures
            if (j < 7 and board[8*(i+ADD)+j+1] & NCOLOR)
                mvsend = move_store_maybe_promote(mvsend, COLOR, i == PROMRANK, i, j, i+ADD, j+1);
            if (j > 0 and board[8*(i+ADD)+j-1] & NCOLOR)
                mvsend = move_store_maybe_promote(mvsend, COLOR, i == PROMRANK, i, j, i+ADD, j-1);

            if (i == EPRANK) {  // en passant capture
                if (LASTMOVE.f0 == i+ADD+ADD and LASTMOVE.f1 == j-1 and LASTMOVE.t0 == i and LASTMOVE.t1 == j-1 and board[8*i+j-1] & PAWN)
                    mvsend = move_store(mvsend, COLOR, i, j, i+ADD, j-1, 'e');
                if (LASTMOVE.f0 == i+ADD+ADD and LASTMOVE.f1 == j+1 and LASTMOVE.t0 == i and LASTMOVE.t1 == j+1 and board[8*i+j+1] & PAWN)
                    mvsend = move_store(mvsend, COLOR, i, j, i+ADD, j+1, 'e');
            }

            if (only_captures)
                continue;

            if (not board[8*(i+ADD)+j]) {  // 1 square forward
                mvsend = move_store_maybe_promote(mvsend, COLOR, i == PROMRANK, i, j, i+ADD, j);
                if (i == STARTRANK and not board[8*(i+ADD+ADD)+j]) {  // 2 squares forward
                    mvsend = move_store(mvsend, COLOR, i, j, i+ADD+ADD, j, '\0');
                }
            }
        }
        else {  // non-pawn moves
            for (num l = 0; PIECEDIRS[bijpiece][l].a != 0 or PIECEDIRS[bijpiece][l].b != 0; ++l) {
                num a = PIECEDIRS[bijpiece][l].a;
                num b = PIECEDIRS[bijpiece][l].b;
                for (num k = 1; k <= PIECERANGE[bijpiece]; ++k)
                {
                    if (not is_inside(i+a*k, j+b*k))  // out of bounds
                        break;
                    if (board[8*(i+a*k)+j+b*k] & COLOR)  // move to square with own piece (illegal)
                        break;
                    if (board[8*(i+a*k)+j+b*k] & NCOLOR) {  // move to square with enemy piece (capture)
                        mvsend = move_store(mvsend, COLOR, i, j, i+a*k, j+b*k, '\0');
                        break;
                    }
                    if (not only_captures and not board[8*(i+a*k)+j+b*k])  // move to empty square
                        mvsend = move_store(mvsend, COLOR, i, j, i+a*k, j+b*k, '\0');
                }
            }
        }
    }

    return {0, mvs, mvsend};
}


// adaptive search-extensions-like value of a move AFTER it is already on the board.
num eval_adaptive_depth(num COLOR, Move mv, num hit_piece, bool skip) {
    if (skip)  // skip for quiescence search, simply not needed
        return 0;

    num OTHER_COLOR = (COLOR == WHITE ? BLACK : WHITE);

    if (king_in_check(OTHER_COLOR))  // https://www.chessprogramming.org/Check_Extensions
        return 0;

    if ((mv.t0 == 1 or mv.t0 == 6) and (board[8*mv.t0+mv.t1] & PAWN))  // passed pawn extension
        return 0;

    return 1;
}


// https://www.chessprogramming.org/MVV-LVA
// For captures, first consider low-value pieces capturing high-value pieces to produce alpha-beta-cutoffs
// In this specific implementation, all captures are sorted before non-captures.
// For the remaining non-captures, the least valuable pieces move first (good cos queens have so many moves)
// https://www.chessprogramming.org/Move_Ordering#Typical_move_ordering
int_fast32_t move_order_key(Move mv)
{
//    // try best move (pv move) from previous search (iterative deepening)
//    if ((TRANSPOS_TABLE_ZOB[zobrint_hash & ZOB_MASK] == zobrint_hash) and (TRANSPOS_TABLE[zobrint_hash & ZOB_MASK] == mv))
//        //{printf_move(LASTMOVE); printf_move(mv); printf("\n"); return 90000003;}
//        return 90000020;

    // MVV-LVA (most valuable victim, least valuable attacker)
    if (board[8*mv.t0+mv.t1])
        return 1000 * PIECEVALS[board[8*mv.t0+mv.t1] & COLORBLIND] - PIECEVALS[board[8*mv.f0+mv.f1] & COLORBLIND];  // min: 68_000

    // try "killer moves"
    for (num i = 0; i < MAX_KILLER_MOVES; i++)
        if (KILLER_TABLE[NODE_DEPTH][i] == mv)
            return 49000 - i;

    if (mv.prom)
        return 1;

    return 0;
}


int move_order_cmp(const void* a, const void* b)
{
    Move **move_a = (Move **)a;
    Move **move_b = (Move **)b;
    return move_order_key(**move_b) - move_order_key(**move_a);  // invert comparison so biggest-valued move is at start of list
}


void select_front_move(Move** front, Move** back)
{
    Move** cur = front;
    int_fast32_t front_val = move_order_key(**front);

    while (++cur != back) {
        int_fast32_t cur_val = move_order_key(**cur);

        if (cur_val > front_val) {
            Move* tmp = *front;
            *front = *cur;
            *cur = tmp;
            front_val = cur_val;
        }
    }
}


// non-negamax quiescent alpha-beta minimax search
// https://www.chessprogramming.org/Quiescence_Search
ValuePlusMove alphabeta(num COLOR, num alpha, num beta, num adaptive, bool is_quies, num depth, bool lines, bool lines_accurate)
{
    NODES_NORMAL += !is_quies;
    NODES_QUIES += is_quies;

    // treat all repeated positions as an instant draw to avoid repetitions when winning and encourage when losing
    if ((depth == 1 or depth == 2) and board_positions_seen.count(zobrint_hash))
        return {0, {0}};

    if (is_quies) {
        // TODO: limit quiescence search depth somehow
        if (NO_QUIES)
            return {board_eval, {0}};
    }
    else
        // Main search is done, do quiescence search at the leaf nodes
        if (adaptive <= 0)
            return alphabeta(COLOR, alpha, beta, adaptive, true, depth, lines, lines_accurate);

    ValuePlusMove best = {COLOR == WHITE ? -inf : inf, {0}};
    if (is_quies) {
        // "standing pat": to compensate for not considering non-capture moves, at least one move should be
        //    better than doing no move ("null move") -> avoids senseless captures, but vulnerable to zugzwang
        best = {board_eval, {0}};

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

    Move mv = {0};
    PiecePlusCatling ppc;
    ValuePlusMoves gl = {0};
    Move** cur_mv = NULL;

    num legal_moves_found = 0;
    Move LASTMOVE_BAK = LASTMOVE;

    num alpha_raised_n_times = 0;
    bool pv_in_hash_table = (not is_quies) and (TRANSPOS_TABLE_ZOB[zobrint_hash & ZOB_MASK] == zobrint_hash);
    Move pv_mv = TRANSPOS_TABLE[zobrint_hash & ZOB_MASK];

    // try best move (hash/pv move) from previous search iterative deepening search first
    //  -> skip move generation if alpha-beta cutoff (3.6% fewer calls to genmoves, overall 1.4% speedup Sadge)
    if (pv_in_hash_table)
    {
        mv = TRANSPOS_TABLE[zobrint_hash & ZOB_MASK];
        ppc = make_move(mv);
        pv_in_hash_table = true;
        goto jump_into_loop_with_hash_move;
    }

    while (true)
    {
        if (gl.moves == NULL)
        {
            LASTMOVE = LASTMOVE_BAK;

            gl = gen_moves_maybe_legal(COLOR, is_quies);
            gl_moves_backup = gl.moves;
            num mvs_len = gl.movesend - gl.moves;  // can only be 0 in illegal positions -> add assert statement

            NODE_DEPTH = depth;

            if (DEBUG >= 3) printf_moves(gl.moves, mvs_len, "BEFORE QSORT\n");
            // TODO: Different sorting function for quies vs non-quies?
            // TODO: selection-type sort instead ?
            qsort(gl.moves, mvs_len, sizeof(Move *), move_order_cmp);  // >=25% of runtime spent in qsort
            if (DEBUG >= 3) printf_moves(gl.moves, mvs_len, "AFTER QSORT\n");
        }

        if (cur_mv == gl.movesend)  // end of move list
            break;

        //select_front_move(cur_mv, gl.movesend);

        mv = **(cur_mv);

        // skip hash move that we already did before gen_moves_maybe_legal
        if (pv_in_hash_table and (pv_mv == mv)) {
            cur_mv++;
            continue;  // already checked hash move
        }

        ppc = make_move(mv);

        if (king_in_check(COLOR)) {  // illegal move  // king_in_check takes 11s of the 30s program time!!
            unmake_move(mv, ppc.hit_piece, ppc.c_rights_w, ppc.c_rights_b);
            cur_mv++;
            continue;
        }

        jump_into_loop_with_hash_move: 0;

        legal_moves_found++;

        if ((depth < 2) and (DEBUG >= 3)) {
            for (int i = 0; i < depth; i++) printf("    ");
            printf_move(mv); printf(" ADAPT %ld \n", adaptive);
        }

        LASTMOVE = mv;  // only needed for gen_moves_maybe_legal so okay to only set here

        ValuePlusMove rec;
        num adaptive_new = adaptive - eval_adaptive_depth(COLOR, mv, ppc.hit_piece, is_quies);

        // "Since there is not much to be gained in the last two plies of the normal search, one might disable PVS there"
        // No sense in searching beyond depth 5 anyway because we only record depth <= 5 in transpos table
        if (NO_PRINCIPAL_VARIATION_SEARCH or (depth > 5) or not (pv_in_hash_table and (alpha_raised_n_times == 1)))
            // Normal alpha-beta search
            rec = alphabeta(
                COLOR == WHITE ? BLACK : WHITE,
                alpha,
                beta,
                adaptive_new, is_quies, depth + 1, lines, lines_accurate
            );
        else {
            // https://www.chessprogramming.org/Principal_Variation_Search
            // Idea of PV: assume hash move will already be best move, so use null window to aggressively prune other moves
            // Should be active when (alpha_raised_n_times == 1) (PV move raised alpha and no move beat PV move yet)
            rec = alphabeta(  // try a null-window search that saves time if everything is below alpha
                COLOR == WHITE ? BLACK : WHITE,
                COLOR == WHITE ? alpha : (beta - 1),
                COLOR == WHITE ? (alpha + 1) : beta,
                adaptive_new, is_quies, depth + 1, lines, lines_accurate
            );

            if (COLOR == WHITE ? rec.value > alpha : rec.value < beta)  // costly re-search if above search fails
                rec = alphabeta(
                    COLOR == WHITE ? BLACK : WHITE,
                    alpha,
                    beta,
                    adaptive_new, is_quies, depth + 1, lines, lines_accurate
                );
        }

        if (COLOR == WHITE ? rec.value > best.value : rec.value < best.value)
            best = {rec.value, mv};

        unmake_move(mv, ppc.hit_piece, ppc.c_rights_w, ppc.c_rights_b);

        if ((depth == 0 and lines) or (DEBUG >= 3)) {
            for (int i = 0; i < depth; i++)
                printf("    ");
            printf_move(mv);
            // due to alpha-beta pruning, this isn't the "true" eval for suboptimal moves, unless lines_accurate=true
            bool accurate = (best.move == mv) or lines_accurate;
            ValuePlusMove mv_printable = {rec.value, mv};
            printf_move_eval(mv_printable, accurate);
        }

        if (not is_quies and not (lines_accurate and depth == 0)) {  // alpha-raising and alpha-beta-cutoffs
            if (COLOR == WHITE and best.value > alpha) {  // lo bound
                alpha = best.value;
                alpha_raised_n_times++;
            }
            if (COLOR == BLACK and best.value < beta) {  // hi bound
                beta = best.value;
                alpha_raised_n_times++;
            }
            if (beta <= alpha) {  // alpha-beta cutoff
                if (not ppc.hit_piece) {  // store non-captures producing cutoffs as killer moves
                    for (num i = 0; i < MAX_KILLER_MOVES; i++)
                        if (KILLER_TABLE[depth][i] == mv)
                            goto do_not_store_in_killer_table;

                    // shift old entries over by 1
                    for (num i = MAX_KILLER_MOVES - 1; i >= 1; i--)
                        KILLER_TABLE[depth][i] = KILLER_TABLE[depth][i-1];

                    KILLER_TABLE[depth][0] = mv;
                }

                do_not_store_in_killer_table: break;
            }
        }

        // TODO: combine both cases
        // quiescence cutoff
        if (is_quies and COLOR == WHITE) {
            if (best.value >= beta) {
                best.value = beta;

                for (int i = 0; i < 128; ++i) {
                    if (not gl.moves[i])
                        break;
                    free(gl.moves[i]);
                }
                free(gl.moves);

                return best;
            }
            if (best.value > alpha)
                alpha = best.value;
        }
        if (is_quies and COLOR == BLACK) {
            if (best.value <= alpha) {
                best.value = alpha;

                for (int i = 0; i < 128; ++i) {
                    if (not gl.moves[i])
                        break;
                    free(gl.moves[i]);
                }
                free(gl.moves);

                return best;
            }
            if (best.value < beta)
                beta = best.value;
        }

        if (cur_mv != NULL)
            cur_mv++;
    }

    if (gl.moves) {
        for (int i = 0; i < 128; ++i) {
            if (not gl.moves[i])
                break;
            free(gl.moves[i]);
        }
        free(gl.moves);
    }

    // TODO: stalemate/checkmate detection here or in evaluation? adjust to depth
    if (!is_quies and !legal_moves_found) {  // "no legal moves" during normal search
        if (not king_in_check(COLOR))
            return {0, {0}};  // stalemate
        return {(COLOR == WHITE ? 1 : -1) * (-inf+2000+100*depth), {0}};  // checkmate
    }

    if (is_quies and !legal_moves_found)  // "no captures" in quiescence search
        return best;

    // https://crypto.stackexchange.com/questions/27370/formula-for-the-number-of-expected-collisions
    // TODO: calculate expected number of collisions
    //if ((depth <= 5) and not (best.move == NULLMOVE)) {
    if (not (best.move == NULLMOVE) and (((depth <= 9) and (not is_quies)) or DEBUG)) {
        TRANSPOS_TABLE[zobrint_hash & ZOB_MASK] = best.move;
        TRANSPOS_TABLE_ZOB[zobrint_hash & ZOB_MASK] = zobrint_hash;  // verify we got the right one
    }

    if (is_quies and COLOR == WHITE)
        best.value = alpha;
    if (is_quies and COLOR == BLACK)
        best.value = beta;

    return best;
}


// find and play the best move in the position
// TODO: lines
std::string calc_move(bool lines = false)
{
    SEARCH_START_CLOCK = std::clock();

    // TODO: mode where a random/suboptimal move can get picked?
    NODES_NORMAL = 0;
    NODES_QUIES = 0;
    for (num i = 0; i < 20; i++)
        for (num j = 0; j < MAX_KILLER_MOVES; j++)
            KILLER_TABLE[i][j] = {0};
    // Since game positions are correlated and the TABLE_ZOB is checked we actually go faster if we do not clear this
    //memset(TRANSPOS_TABLE, 0, sizeof TRANSPOS_TABLE);
    //memset(TRANSPOS_TABLE_ZOB, 0, sizeof TRANSPOS_TABLE_ZOB);

    // Have to re-calculate board info anew each time because GUI/Lichess might reset state
    board_eval = initial_eval();
    zobrint_hash = board_to_zobrint_hash();
    num my_color = IM_WHITE ? WHITE : BLACK;
    num OWN_TIME_TO_USE_MAX = (OWN_CLOCK_REMAINING / 20);  // assume 25-ish moves left
    Move mv = {0};

    num game_phase = 0;   // game phase based on pieces traded. initial pos: 24, rook endgame: 8
    gentuples {
        num p = board[8*i+j];
        game_phase += 1 * !!(p & KNIGHT) + 1 * !!(p & BISHOP) + 2 * !!(p & ROOK) + 4 * !!(p & QUEEN);
    }

    printf((game_phase <= 8) ? "Setting piece-square tables to endgame\n" : "Setting piece-square tables to middlegame\n");
    set_piece_square_table(game_phase <= 8);

    printf("Starting iterative deepening alphabeta search at ZOB %ld\n", zobrint_hash);
    for (num search_depth = 1; search_depth <= MAX_SEARCH_DEPTH; search_depth++)
    {
        SEARCH_ADAPTIVE_DEPTH = search_depth;
        LASTMOVE = LASTMOVE_GAME;

        ValuePlusMove best_at_depth = alphabeta(my_color, -inf/2, inf/2, SEARCH_ADAPTIVE_DEPTH, false, 0, (DEBUG >= 2), (DEBUG >= 2));

        printf_move(best_at_depth.move);
        printf_move_eval(best_at_depth, true);
        mv = best_at_depth.move;

        if ((best_at_depth.value <= -inf/2) or (best_at_depth.value >= inf/2)) {
            printf("Found mate score already so stopping iterative deepening early\n");
            break;
        }

        double time_expired = 100.0 * (std::clock() - SEARCH_START_CLOCK) / CLOCKS_PER_SEC;
        if (time_expired * 5 > OWN_TIME_TO_USE_MAX) {  // assume next depth takes 5 times as long
            printf("Used up time budget of %7ldcs allocated to finding this move\n", OWN_TIME_TO_USE_MAX);
            break;
        }
    }
    printf("|End|ing iterative deepening alphabeta search at ZOB %ld\n", zobrint_hash);

    make_move(mv);

    return move_to_str(mv);
}


void init_data(void) {
    // TODO use std::array or map with vector inside or something to avoid leaks
    board_initial_position();
    init_zobrint();

    PIECEDIRS[KNIGHT][0] = {1, 2};
    PIECEDIRS[KNIGHT][1] = {2, 1};
    PIECEDIRS[KNIGHT][2] = {-1, 2};
    PIECEDIRS[KNIGHT][3] = {2, -1};
    PIECEDIRS[KNIGHT][4] = {-1, -2};
    PIECEDIRS[KNIGHT][5] = {-2, -1};
    PIECEDIRS[KNIGHT][6] = {1, -2};
    PIECEDIRS[KNIGHT][7] = {-2, 1};
    PIECEDIRS[KNIGHT][8] = {0, 0};

    PIECEDIRS[BISHOP][0] = {1, 1};
    PIECEDIRS[BISHOP][1] = {1, -1};
    PIECEDIRS[BISHOP][2] = {-1, 1};
    PIECEDIRS[BISHOP][3] = {-1, -1};
    PIECEDIRS[BISHOP][4] = {0, 0};

    PIECEDIRS[ROOK][0] = {0, 1};
    PIECEDIRS[ROOK][1] = {1, 0};
    PIECEDIRS[ROOK][2] = {0, -1};
    PIECEDIRS[ROOK][3] = {-1, 0};
    PIECEDIRS[ROOK][4] = {0, 0};

    for (num piece = QUEEN; piece <= KING; piece += KING - QUEEN) {  // what
        PIECEDIRS[piece][0] = {0, 1};
        PIECEDIRS[piece][1] = {1, 0};
        PIECEDIRS[piece][2] = {0, -1};
        PIECEDIRS[piece][3] = {-1, 0};
        PIECEDIRS[piece][4] = {1, 1};
        PIECEDIRS[piece][5] = {1, -1};
        PIECEDIRS[piece][6] = {-1, 1};
        PIECEDIRS[piece][7] = {-1, -1};
        PIECEDIRS[piece][8] = {0, 0};
    }

    PIECERANGE[KNIGHT] = 1;
    PIECERANGE[BISHOP] = 7;
    PIECERANGE[ROOK] = 7;
    PIECERANGE[QUEEN] = 7;
    PIECERANGE[KING] = 1;

    // https://lichess.org/@/ubdip/blog/finding-the-value-of-pieces/PByOBlNB
    PIECEVALS[0] = 0;
    PIECEVALS[PAWN] = 100;
    PIECEVALS[KNIGHT] = 305;
    PIECEVALS[BISHOP] = 325;  // Should not trade bishop for a knight, unless it wins a pawn or king pawn structure becomes damaged
    PIECEVALS[ROOK] = 470;  // Engine keeps trading its knight+bishop for a rook+pawn, thinking it is a good trade, which it is not
    PIECEVALS[QUEEN] = 950;  // Engine also trades into having 2 rooks for a queen, this is usually also not worth it
    PIECEVALS[KING] = inf;
}



void test() {
    MAX_SEARCH_DEPTH = 7;
    OWN_CLOCK_REMAINING = 9999999;

    std::cout << "TODO" << std::endl;
    board_from_fen("8/8/p6P/P6k/4p1p1/6K1/8/8 b - - 0 52");
    pprint();
    std::cout << calc_move(true) << std::endl;

    std::cout << "Qxf3+ sac 34 https://lichess.org/2RWlBUCy#66" << std::endl;
    board_from_fen("1k2r3/2b3p1/1pp2pPp/r7/P2P4/2R2q2/2QR1PP1/6K1 w - - 0 34");
    pprint();
    std::cout << calc_move(true) << std::endl;

    std::cout << "Qxf3+ sac 33 https://lichess.org/2RWlBUCy#64" << std::endl;
    board_from_fen("1k2r3/2b3p1/1pp2pPp/r2q4/P2P4/1R3N2/2QR1PP1/6K1 w - - 3 33");
    pprint();
    std::cout << calc_move(true) << std::endl;

    std::cout << "Tests Mate-in-1 (and stalemate)" << std::endl;
    board_from_fen("k1K5/8/8/8/8/8/8/7R w - - 0 1");
    pprint();
    std::cout << calc_move(true) << std::endl;  // Should find Ra1 being mate (eval=299) and Rh7 being stalemate (eval=0)

    std::cout << "\nTests Mate-in-2" << std::endl;
    board_from_fen("1k5r/ppp5/8/8/8/3Q4/8/2KR4 w - - 0 1");
    pprint();
    std::cout << calc_move(true) << std::endl;  // Should find mate-in-2 (eval=297) if depth >= 4

    std::cout << "\nTests Mate-in-3" << std::endl;
    board_from_fen("2k4r/ppp5/8/8/8/3Q4/8/2KR4 w - - 0 1");
    pprint();
    std::cout << calc_move(true) << std::endl;  // Should find mate-in-3 (eval=295) if depth >= 6

    std::cout << "\nTests Mate-in-4 (and promotion)" << std::endl;
    board_from_fen("k1K5/8/1P6/8/8/8/8/8 w - - 0 1");
    pprint();
    std::cout << calc_move(true) << std::endl;  // Kc7 is stalemate, b7+ promotes

    std::cout << "\nTests 2 queens mate-in-3 (should stop early)" << std::endl;
    board_clear();
    set_up_kings(7, 6, 5, 2);
    board[0*8+4] = WHITE + QUEEN;
    board[3*8+5] = WHITE + QUEEN;
    pprint();
    std::cout << calc_move(true) << std::endl;  // should not be slow

    std::cout << "\nTests Promotion" << std::endl;
    board_from_fen("k1K5/6P1/8/8/8/8/8/8 w - - 0 1");
    pprint();
    std::cout << calc_move(true) << std::endl;

    std::cout << "\nTests Stalemate tactics from https://lichess.org/VdE6aggQ/black#74" << std::endl;
    board_from_fen("4Q3/6pk/6Np/7P/3P4/8/PPq2PK1/6R1 b - - 4 36");
    pprint();
    std::cout << calc_move(true) << std::endl;  // Qxg3+ leads to forced stalemate at depth 3

    std::cout << "\nTests En-Passant" << std::endl;
    board_initial_position();
    make_move_str("e2e4"); make_move_str("a7a6");
    make_move_str("e4e5"); make_move_str("d7d5");
    pprint();
    std::cout << calc_move(true) << std::endl;

    std::cout << "\nTests Castling" << std::endl;
    board_initial_position();
    board[7*8+5] = 0;
    board[7*8+6] = 0;
    board[6*8+4] = 0;
    pprint();
    std::cout << calc_move(true) << std::endl;

    std::cout << "\nTests detecting/avoiding repetitions" << std::endl;
    board_initial_position();
    IM_WHITE = true;
    zobrint_hash ^= zobrint_random_table[board[3]][3];
    board[3] = 0;  // up a queen so shouldn't draw
    board_positions_seen.insert(board_to_zobrint_hash());
    make_move_str("g1f3"); make_move_str("g8f6");
    make_move_str("f3g1"); make_move_str("f6g8");
    make_move_str("g1f3"); make_move_str("g8f6");
    pprint();
    std::cout << calc_move(true) << std::endl;  // Should see that f3g1 would be 3-fold

    std::cout << "\nTests Lasker Trap in Albin Countergambit" << std::endl;
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
    std::cout << calc_move(true) << std::endl;  // Should find knight promotion as best

    std::cout << "\nTests going into a fork https://lichess.org/A1Jn5Z5s#53" << std::endl;
    board_from_fen("3rr1k1/p4pp1/1p4np/3Ppb2/1PP1N3/4R1P1/5PBP/4R1K1 b - - 4 27");
    pprint();
    std::cout << calc_move(true) << std::endl;  // should NOT play Rc8??
    // ^ Reason for this bug was searching forward moves LESS than backwards moves (intended the other way)

    std::cout << "\nTests missed en-passant https://lichess.org/4bSSvnGS/white#60" << std::endl;
    board_from_fen("r5k1/p4p2/2n1b2p/5q2/2p1R3/P1N2PB1/QP4KP/8 w - - 14 31");
    pprint();
    std::cout << calc_move(true) << std::endl; // should NOT play 31. b4??

    std::cout << "\nTests missed knight move https://lichess.org/jOQqZlqM#42" << std::endl;
    board_from_fen("r3r1k1/1pb2pp1/2P2p2/1p6/8/P1NR2p1/5PPP/5RK1 w - - 0 22");
    pprint();
    std::cout << calc_move(true) << std::endl; // should NOT play 22. hxg3?, rather Nxb5 or Nd5

    board_initial_position();
    printf("\nINITIAL BOARD EVAL: %ld\n", board_eval);
    make_move_str("e2e4");
    printf("1. e4 EVAL: %ld\n", board_eval);
    IM_WHITE = false; std::cout << calc_move(true) << std::endl;
    make_move_str("b1c3");
    IM_WHITE = false; std::cout << calc_move(true) << std::endl;
    make_move_str("g1f3");
    IM_WHITE = false; std::cout << calc_move(true) << std::endl;

    printf("\nLIST OF MOVES IN RESPONSE TO QUEEN\n");
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
    std::cout << calc_move(true) << std::endl;

    std::cout << "\nTests Rc8 blunder caused by wrong continue https://lichess.org/1NIkB4Dg/black#48" << std::endl;
    board_from_fen("r3q1k1/p2n3p/1pQ2np1/3p4/3P4/1P1RP1P1/P4P1P/2R3K1 b - - 0 24");
    pprint();
    std::cout << calc_move(true) << std::endl;  // should be ~0.75ish, not -3.90 as in the game

    std::cout << "\nTests iliachess game https://www.chess.com/analysis/game/live/50110768933" << std::endl;
    board_from_fen("r3r1k1/pbq2p1R/1p2nQp1/8/6P1/2PBpPP1/PP4K1/4R3 b - - 0 22");
    pprint();
    std::cout << calc_move(true) << std::endl;

    std::cout << "Bf5 blunder from https://lichess.org/Cwt4L3df/black#81" << std::endl;
    board_from_fen("8/6pk/6bp/p7/P7/2R1R1P1/1P6/2KB1q2 b - - 3 41");
//    make_move_str("g6f5"); make_move_str("e3f3");
//    make_move_str("f1h3"); make_move_str("f3f5");
//    make_move_str("h3f5");
//    IM_WHITE = true;
    pprint();
    std::cout << calc_move(true) << std::endl;  // finds Bf5 at depth 7 which is a blunder :/

    std::cout << "\nTests pawn loss tactics https://lichess.org/5pQ86dA7/white#42" << std::endl;
    board_from_fen("2k1rb2/1bpp1p2/p6p/1p1N4/2P5/1P6/1P1N1PPP/5RK1 w - - 1 22");
    board_from_fen("2k1r3/1bpp1p2/p6p/1pb5/2P5/1P2N3/1P1N1PPP/5RK1 w - - 3 23");
    pprint();
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
            OWN_CLOCK_REMAINING = time_left;  // time left in centiseconds
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
            std::string mv = calc_move(true);
            num_moves++;
            pprint();
            printf(MODE_UCI ? "bestmove %s\n" : "move %s\n", mv.c_str());
        }

        if (startswith("go", line)) {
            IM_WHITE = (num_moves + 1) % 2;
            started = true;
            std::string mv = calc_move(true);
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
