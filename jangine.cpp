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

#define SIMPLE_EVAL  // TODO: just-material eval, piece-square-eval, and turochamp eval
#define NO_QUIES 0
#define QUIES_DEPTH 0  // TODO: limit search of quiescence captures
#define MAX_KILLER_MOVES 1
#define NO_PRINCIPAL_VARIATION_SEARCH 0 // seems to actually be slower since my leaf eval is so cheap

#pragma GCC diagnostic ignored "-Wnarrowing"

num PAWN = 1, KNIGHT = 2, BISHOP = 4, ROOK = 8, QUEEN = 16, KING = 32, WHITE = 64, BLACK = 128;
num KING_EARLYGAME = 8001, KING_ENDGAME = 8002, PAWN_EARLYGAME = 9001, PAWN_ENDGAME = 9002;
num COLORBLIND = ~(WHITE | BLACK);  // use 'piece & COLORBLIND' to remove color from a piece
num inf = 20000;  num inf_king = 26000;
num OOB = 256; num OOV = 0;  // out-of-bounds for "10x12" (12x10) boards and OOB value
std::map<num, char> piece_to_letter = {{0, ' '}, {1, ' '}, {2, 'N'}, {4, 'B'}, {8, 'R'}, {16, 'Q'}, {32, 'K'}};
std::map<num, std::string> id_to_unicode = {
        {  0, "."},
        { 65, "♙"}, { 66, "♘"}, { 68, "♗"}, { 72, "♖"}, { 80, "♕"}, { 96, "♔"},
        {129, "♟"}, {130, "♞"}, {132, "♝"}, {136, "♜"}, {144, "♛"}, {160, "♚"},
};
num PIECE_SQUARE_TABLES[257][120] = {0};  // C array is much faster to query than a C++ map
std::map<num, std::array<num, 120>> PIECE_SQUARE_TABLES_SOURCE = {
        {PAWN_EARLYGAME, std::array<num, 120>{
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV,   0,   0,   0,   0,   0,   0,   0,   0, OOV,
                OOV,  60,  80,  80,  80,  80,  80,  80,  60, OOV,
                OOV,  20,  40,  40,  40,  40,  40,  40,  40, OOV,
                OOV,   0,   0,   0,  10,  10,  10,  10,  10, OOV,
                OOV,   5,   0,  10,  30,  30,   0,   0,   0, OOV,
                OOV,   5,   0,   0,   0,   0,   0,   0,   0, OOV,
                OOV,   0,   0,   0, -20, -20,  30,  30,  15, OOV,
                OOV,   0,   0,   0,   0,   0,   0,   0,   0, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
        }},
        {PAWN_ENDGAME, std::array<num, 120>{
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV,   0,   0,   0,   0,   0,   0,   0,   0, OOV,
                OOV,  80, 100, 100, 100, 100, 100, 100,  80, OOV,
                OOV,  40,  60,  60,  60,  60,  60,  60,  40, OOV,
                OOV,  30,  40,  40,  40,  40,  40,  40,  30, OOV,
                OOV,  10,  20,  20,  20,  20,  20,  20,  10, OOV,
                OOV,   5,  10,  10,  10,  10,  10,  10,   5, OOV,
                OOV,   0,   0,   0,   0,   0,   0,   0,   0, OOV,
                OOV,   0,   0,   0,   0,   0,   0,   0,   0, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
        }},
        {KNIGHT, std::array<num, 120>{
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, -60, -40, -30, -30, -30, -30, -40, -60, OOV,
                OOV, -40, -20,   0,   0,   0,   0, -20, -40, OOV,
                OOV, -30,   0,  10,  15,  15,  10,   0, -30, OOV,
                OOV, -30,   0,  15,  20,  20,  15,   0, -30, OOV,
                OOV, -30,   0,  15,  20,  20,  15,   0, -30, OOV,
                OOV, -30,   5,  15,  15,  15,  15,   5, -30, OOV,
                OOV, -40, -20,   0,  10,  10,   0, -20, -40, OOV,
                OOV, -60, -40, -30, -30, -30, -30, -40, -60, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
        }},
        // bishop isn't actually bad on the bad row but prevents castling
        {BISHOP, std::array<num, 120>{
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, -20, -10, -10, -10, -10, -10, -10, -20, OOV,
                OOV, -10,   0,   0,   0,   0,   0,   0, -10, OOV,
                OOV, -10,   0,   5,  10,  10,   5,   0, -10, OOV,
                OOV, -10,  15,   5,  10,  10,   5,  15, -10, OOV,
                OOV, -10,   0,  15,  10,  10,  15,   0, -10, OOV,
                OOV, -10,  10,  10,  10,  10,  10,  10, -10, OOV,
                OOV, -10,   5,   0,   0,   0,   0,   5, -10, OOV,
                OOV, -20, -10, -10, -10, -10, -20, -10, -20, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
        }},
        {ROOK, std::array<num, 120>{
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV,  -5,   0,   0,   0,   0,   0,   0,  -5, OOV,
                OOV,  -5,  20,  20,  20,  20,  20,  20,  -5, OOV,
                OOV,  -5,   0,   0,  10,  10,   0,   0,  -5, OOV,
                OOV,  -5,   0,   0,  10,  10,   0,   0,  -5, OOV,
                OOV,  -5,   0,   0,  10,  10,   0,   0,  -5, OOV,
                OOV,  -5,   0,   0,  10,  10,   0,   0,  -5, OOV,
                OOV,  -5,   0,   0,  10,  10,   0,   0,  -5, OOV,
                OOV,  -5,   0,   0,  10,  10,   0,   0,  -5, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
        }},
        // TODO: avoid capturing b7?
        {QUEEN, std::array<num, 120>{
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, -20, -10, -10,  -5,  -5, -10, -10, -20, OOV,
                OOV, -10,   0,   0,   0,   0,   0,   0, -10, OOV,
                OOV, -10,   0,   5,   5,   5,   5,   0, -10, OOV,
                OOV,  -5,   0,   5,   5,   5,   5,   0,  -5, OOV,
                OOV,   0,   0,   5,   5,   5,   5,   0,  -5, OOV,
                OOV, -10,   5,   5,   5,   5,   5,   0, -10, OOV,
                OOV, -10,   0,   5,   0,   0,   0,   0, -10, OOV,
                OOV, -20, -10, -10,  -5,  -5, -10, -10, -20, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
        }},
        {KING_EARLYGAME, std::array<num, 120>{
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, -20, -20, -20, -20, -20, -20, -20, -20, OOV,
                OOV, -20, -20, -20, -20, -20, -20, -20, -20, OOV,
                OOV, -20, -20, -20, -20, -20, -20, -20, -20, OOV,
                OOV, -20, -20, -20, -20, -20, -20, -20, -20, OOV,
                OOV, -20, -20, -20, -20, -20, -20, -20, -20, OOV,
                OOV, -20, -20, -20, -20, -20, -20, -20, -20, OOV,
                OOV,  20,  20,  10,   0,   0,   0,  40,  40, OOV,
                OOV,  20,  30,  20,   0,  10,   0,  50,  40, OOV,    // strongly encourage short castling
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
        }},
        {KING_ENDGAME, std::array<num, 120>{
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV,  10,  20,  30,  40,  40,  30,  20,  10, OOV,
                OOV,  20,  30,  40,  40,  40,  40,  30,  20, OOV,
                OOV,  30,  40,  50,  50,  50,  50,  40,  30, OOV,
                OOV,  20,  30,  40,  50,  50,  40,  30,  20, OOV,
                OOV,  10,  10,  30,  40,  40,  30,  10,  10, OOV,
                OOV,   0,   0,  20,  30,  30,  20,   0,   0, OOV,
                OOV, -30, -30,   0,   0,   0,   0, -30, -30, OOV,
                OOV, -30, -30, -30, -30, -30, -30, -30, -30, OOV,   // encourage going to the center
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
                OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV, OOV,
        }}
};

typedef struct CASTLINGRIGHTS {
    bool lc;  // left-castling still possible (long castling)
    bool rc;  // right-castling still possible (short castling)
} CASTLINGRIGHTS;

typedef struct Move {
    int8_t from;
    int8_t to;
    char prom;  // promote to piece, castle, or en-passant

    // needed for std::map
    bool operator<( const Move & that ) const {
        return (this->from << 16) + (this->to << 8) + this->prom <
            (that.from << 16) + (that.to << 8) + that.prom;
    }
    bool operator==( const Move& that ) const {
        return this->from == that.from and this->to == that.to and this->prom == that.prom;
    }
} Move;

// Store captured piece and new castling rights so move can be undone
typedef struct PiecePlusCatling {
    num hit_piece;
    CASTLINGRIGHTS c_rights_w;
    CASTLINGRIGHTS c_rights_b;
} PiecePlusCatling;

int8_t tte_exact = 0; int8_t tte_alpha = 1; int8_t tte_beta = 2;
typedef struct TTEntry {
    int64_t zobrint_hash;
    Move move;  // either the best move found if an exact node, or a node good enough to cause a cutoff
    int16_t value;
    int8_t tte_flag;  // best, alpha, or beta value
    int8_t depth_left;
    // TODO: why not unsigned
} TTEntry;

CASTLINGRIGHTS CASTLINGWHITE = {true, true};
CASTLINGRIGHTS CASTLINGBLACK = {true, true};

Move NULLMOVE = {0};
Move LASTMOVE = {0};
Move LASTMOVE_GAME = {0};

TTEntry TRANSPOS_TABLE[16777216] = {0};  // 4+3+2+1+1 = 11 bits -> 11 * 2**24 bytes = 176 MiB
#define ZOB_MASK 0xffffff

num PIECEDIRS[65][9] = {0};
bool PIECESLIDE[65] = {0};
num PIECEVALS[257] = {0};

num NODE_DEPTH = 0;
Move KILLER_TABLE[20][MAX_KILLER_MOVES] = {0};  // table of killer moves for each search depth

num board[120] = {0};
num board_eval = 0;  // takes about 10% of compute time
int num_moves = 0;
bool IM_WHITE = false;
bool started = true;
num MODE_UCI = 1; num MODE_XBOARD = 2; num MODE_XJ = 3; num MODE = MODE_UCI;
num VERBOSITY = 1;  // 0: bestmove only, 1: uci INFO, 2: print boards/state, 3: all lines, 4: tons of output

int64_t NODES_NORMAL = 0;
int64_t NODES_QUIES = 0;
std::clock_t SEARCH_START_CLOCK;

void pprint() {
    if (VERBOSITY < 2) return;

    for (num i = 0; i < 8; ++i) {
        printf("  %ld ", (8 - i));
        for (num j = 0; j < 8; ++j) {
            std::string s = id_to_unicode[board[10*i+20+j+1]];
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

bool input_is_move(const char* s) {
    if (strlen(s) < 5 || strlen(s) > 6)  // newline character
        return false;

    return 'a' <= s[0] and s[0] <= 'h' and 'a' <= s[2] and s[2] <= 'h' and
           '1' <= s[1] and s[1] <= '8' and '1' <= s[3] and s[3] <= '8';
}

num default_board[120] = {  // https://www.chessprogramming.org/10x12_Board
        OOB, OOB, OOB, OOB, OOB, OOB, OOB, OOB, OOB, OOB,
        OOB, OOB, OOB, OOB, OOB, OOB, OOB, OOB, OOB, OOB,
        OOB, BLACK + ROOK, BLACK + KNIGHT, BLACK + BISHOP, BLACK + QUEEN,
            BLACK + KING, BLACK + BISHOP, BLACK + KNIGHT, BLACK + ROOK, OOB,
        OOB, BLACK + PAWN, BLACK + PAWN, BLACK + PAWN, BLACK + PAWN,
            BLACK + PAWN, BLACK + PAWN, BLACK + PAWN, BLACK + PAWN, OOB,
        OOB, 0, 0, 0, 0, 0, 0, 0, 0, OOB,
        OOB, 0, 0, 0, 0, 0, 0, 0, 0, OOB,
        OOB, 0, 0, 0, 0, 0, 0, 0, 0, OOB,
        OOB, 0, 0, 0, 0, 0, 0, 0, 0, OOB,
        OOB, WHITE + PAWN, WHITE + PAWN, WHITE + PAWN, WHITE + PAWN,
            WHITE + PAWN, WHITE + PAWN, WHITE + PAWN, WHITE + PAWN,  OOB,
        OOB, WHITE + ROOK, WHITE + KNIGHT, WHITE + BISHOP, WHITE + QUEEN,
            WHITE + KING, WHITE + BISHOP, WHITE + KNIGHT, WHITE + ROOK,  OOB,
        OOB, OOB, OOB, OOB, OOB, OOB, OOB, OOB, OOB, OOB,
        OOB, OOB, OOB, OOB, OOB, OOB, OOB, OOB, OOB, OOB
};

num KINGPOS_WHITE = 95;
num KINGPOS_BLACK = 25;

int64_t zobrint_random_table[257][120] = {0};
// TODO: handle as TTEntry flag instead?
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

    // note that "0" entries have no effect on the final result (for example in zrt[0] for empty squares)
    for (const num& piece : pieces)
        for (int i = 0; i < 64; ++i)
            // for backwards compatability of hashes, assign hashes to their old values for now
            //zobrint_random_table[piece][i] = dist(e2);
            zobrint_random_table[piece][10*(i/8 + 2) + (i % 8) + 1] = dist(e2);

    zobrint_random_table[1][0] = zobrint_random_table[1][21];  // for side-to-move
}

int64_t board_to_zobrint_hash() {
    int64_t zobrint_hash_ = 0;

    for (int i = 0; i < 120; ++i)
        zobrint_hash_ ^= zobrint_random_table[board[i]][i];

    if (not IM_WHITE)
        zobrint_hash_ ^= zobrint_random_table[1][0];

    // TODO: Add en passant ranks and castling rights
    // Okay to implement this without castling rights and en passant at first
    //  because this is only used for repetition detection and PV store, not for actual transpositions
    // We just want to avoid voluntarily going into repeated positions in otherwise winning endgames

    return zobrint_hash_;
}

void set_piece_square_table(bool is_endgame = false) {
    for (num j = 0; j < 120; j++) {
        PIECE_SQUARE_TABLES[0][j] = 0;
        PIECE_SQUARE_TABLES[OOB][j] = 0;

        PIECE_SQUARE_TABLES[KNIGHT][j] = PIECE_SQUARE_TABLES_SOURCE[KNIGHT][j];
        PIECE_SQUARE_TABLES[BISHOP][j] = PIECE_SQUARE_TABLES_SOURCE[BISHOP][j];
        PIECE_SQUARE_TABLES[  ROOK][j] = PIECE_SQUARE_TABLES_SOURCE[  ROOK][j];
        PIECE_SQUARE_TABLES[ QUEEN][j] = PIECE_SQUARE_TABLES_SOURCE[ QUEEN][j];

        PIECE_SQUARE_TABLES[KING][j] = PIECE_SQUARE_TABLES_SOURCE[is_endgame ? KING_ENDGAME : KING_EARLYGAME][j];
        PIECE_SQUARE_TABLES[PAWN][j] = PIECE_SQUARE_TABLES_SOURCE[is_endgame ? PAWN_ENDGAME : PAWN_EARLYGAME][j];

        for (num piece = PAWN; piece <= KING; piece <<= 1) {
            num j_black = 10 * (11 - (j / 10)) + (j % 10);
            PIECE_SQUARE_TABLES[piece + WHITE][j      ] =  PIECE_SQUARE_TABLES[piece][j];
            PIECE_SQUARE_TABLES[piece + BLACK][j_black] = -PIECE_SQUARE_TABLES[piece][j];
        }
    }
}

void board_initial_position() {  // setting up a game
    for (int i = 0; i < 120; ++i)
        board[i] = default_board[i];

    set_piece_square_table();

    KINGPOS_WHITE = 95;
    KINGPOS_BLACK = 25;
    CASTLINGWHITE = {true, true};
    CASTLINGBLACK = {true, true};
    board_eval = 0;

    IM_WHITE = true;
    zobrint_hash = board_to_zobrint_hash();
    board_positions_seen.clear();
    board_positions_seen.insert(zobrint_hash);
}

void board_clear() {  // setting up random positions
    board_initial_position();
    zobrint_hash = 0;

    for (int i = 2; i < 10; ++i)
        for (int j = 1; j < 9; ++j)
            board[10*i+j] = 0;

    CASTLINGWHITE = {false, false};
    CASTLINGBLACK = {false, false};
}

void board_from_fen(const char* fen) {  // setting up a game
    const char* c = fen;
    board_clear();
    num i = 2;
    num j = 1;

    // board
    while (*c != ' ') {
        if (*c == '/') { i++; j = 0; }
        if (*c == '1')  j += 0;
        if (*c == '2')  j += 1;
        if (*c == '3')  j += 2;
        if (*c == '4')  j += 3;
        if (*c == '5')  j += 4;
        if (*c == '6')  j += 5;
        if (*c == '7')  j += 6;
        if (*c == '8')  j += 7;
        if (*c == 'P')  board[10*i+j] = WHITE + PAWN;
        if (*c == 'p')  board[10*i+j] = BLACK + PAWN;
        if (*c == 'N')  board[10*i+j] = WHITE + KNIGHT;
        if (*c == 'n')  board[10*i+j] = BLACK + KNIGHT;
        if (*c == 'B')  board[10*i+j] = WHITE + BISHOP;
        if (*c == 'b')  board[10*i+j] = BLACK + BISHOP;
        if (*c == 'R')  board[10*i+j] = WHITE + ROOK;
        if (*c == 'r')  board[10*i+j] = BLACK + ROOK;
        if (*c == 'Q')  board[10*i+j] = WHITE + QUEEN;
        if (*c == 'q')  board[10*i+j] = BLACK + QUEEN;
        if (*c == 'K') {
            board[10*i+j] = WHITE + KING;
            KINGPOS_WHITE = 10*i+j;
        }
        if (*c == 'k') {
            board[10*i+j] = BLACK + KING;
            KINGPOS_BLACK = 10*i+j;
        }
        j++;
        c++;
    }

    // side to move
    // if you are a real C pro, replace this block with  (*((++c)++) == 'w')  ;)
    c++;
    IM_WHITE = (*c == 'w');
    num_moves += 30 + (num_moves % 2 != IM_WHITE);
    c++;

    // castling => board_clear() sets it all to false
    c++;
    while (*c != ' ') {
        if (*c == 'Q')  CASTLINGWHITE.lc = true;
        if (*c == 'K')  CASTLINGWHITE.rc = true;
        if (*c == 'q')  CASTLINGBLACK.lc = true;
        if (*c == 'k')  CASTLINGBLACK.rc = true;
        c++;
    }

    // en passant target square
    c++;
    if (*c != '-') {
        num j = (*c - 'a') + 1;
        c++;
        num i = ('8' - *c) + 2;
        bool was_white = !!(board[10*(i-1)+j] & WHITE);
        Move mv = {was_white ? 10*(i+1)+j : 10*(i-1)+j, was_white ? 10*(i-1)+j : 10*(i+1)+j};
        LASTMOVE_GAME = mv;
    } else {
        LASTMOVE_GAME = {0};
    }

    // TODO ignore halfmove/fullmove clocks for now since no global move variable yet
    zobrint_hash = board_to_zobrint_hash();
    board_positions_seen.insert(zobrint_hash);
}

num SEARCH_ADAPTIVE_DEPTH = 6;  // how many plies to search

num MAX_SEARCH_DEPTH = 99;  // has to be somewhat low because depth_left is stored as 8-bit number
uint64_t OWN_CLOCK_REMAINING = 180000;  // 180 seconds remaining

typedef struct GenMoves {
    Move* captures;
    Move* captures_end;
    Move* quiets;
    Move* quiets_end;
} GenMoves;

typedef struct ValuePlusMove {
    num value;
    Move move;
} ValuePlusMove;


std::string move_to_str(Move mv, bool algebraic = false) {
    char c0 = 'a' + (char) (mv.from % 10 - 1);
    char c1 = '0' + (char) (10 - (mv.from / 10));  // 2..9 are valid rows
    char c2 = 'a' + (char) (mv.to % 10 - 1);
    char c3 = '0' + (char) (10 - (mv.to / 10));
    char c4 = (char)(mv.prom ? mv.prom : ' ');

    if (!algebraic) {
        if (c4 == ' ' or c4 == 'c' or c4 == 'e')
            return std::string{c0, c1, c2, c3};
        return std::string{c0, c1, c2, c3, c4};
    }

    num piece = board[mv.from];
    num hit_piece = board[mv.to];

    char alg0 = piece_to_letter[piece & COLORBLIND];
    char alg1 = (hit_piece or mv.prom == 'e') ? 'x' : ' ';

    std::string ret{alg0, alg1, c2, c3, ' ', ' ', '(', c0, c1, c2, c3, c4, ')'};
    return ret;
}


inline num eval_material(num piece_with_color) {
    return PIECEVALS[piece_with_color];
}

inline num eval_piece_on_square(num piece_with_color, num i) {
    return PIECE_SQUARE_TABLES[piece_with_color][i];
}

// https://www.chessprogramming.org/Simplified_Evaluation_Function
// Returns centipawn value to a given board position, from WHITE's perspective
// TODO: breakdown into component numbers for better testability
num initial_eval() {
    num eval = 0;

    // material counting
    for (num i = 0; i < 120; ++i)
        eval += eval_material(board[i]);

    // https://www.chessprogramming.org/Piece-Square_Tables
    // piece positioning using piece-square-tables
    for (num i = 0; i < 120; ++i)
        eval += eval_piece_on_square(board[i], i);

    return eval;
}


inline PiecePlusCatling make_move(Move mv)  // apparently 7% of time spent in make and unmake
{
    if (mv == NULLMOVE)
        printf("XXX DANGEROUS! NULL MOVE\n");

    num piece = board[mv.from];
    num hit_piece = board[mv.to];
    board[mv.from] = 0;
    board[mv.to] = piece;

    board_eval -= eval_material(hit_piece) + eval_piece_on_square(hit_piece, mv.to);  // remove captured piece. ignores empty squares (0s)  // always empty for en passant
    board_eval += eval_piece_on_square(piece, mv.to) - eval_piece_on_square(piece, mv.from);  // adjust position values of moved piece

    zobrint_hash ^= zobrint_random_table[piece][mv.from];  // xor OUT piece at   initial   square
    zobrint_hash ^= zobrint_random_table[piece][mv.to];  // xor  IN piece at destination square
    zobrint_hash ^= zobrint_random_table[hit_piece][mv.to];  // xor OUT captured piece (0 if square empty)

    if (mv.prom) {
        if (mv.prom == 'e') {  // en passant
            num ep_square = 10 * (mv.from / 10) + (mv.to % 10);  // square of the CAPTURED/en-passanted pawn  // faster than ternary ? :
            board_eval -= eval_material(board[ep_square]) + eval_piece_on_square(board[ep_square], ep_square);  // captured pawn did not get removed earlier
            zobrint_hash ^= zobrint_random_table[board[ep_square]][ep_square];  // xor OUT captured pawn
            board[ep_square] = 0;
        } else if (mv.prom == 'c') {  // castling -- rook part
            bool is_short_castling = mv.to > mv.from;  // otherwise long castling
            num rookpos_new = is_short_castling ? (mv.from + 1) : (mv.from - 1);  // tests faster because short-castle gets branch-predicted => store castle direction in prom?
            num rookpos_old = is_short_castling ? (mv.from + 3) : (mv.from - 4);

            board[rookpos_new] = board[rookpos_old];
            board[rookpos_old] = 0;
            board_eval += eval_piece_on_square(board[rookpos_new], rookpos_new) - eval_piece_on_square(board[rookpos_new], rookpos_old);  // rook positioning
            zobrint_hash ^= zobrint_random_table[board[rookpos_new]][rookpos_old];  // xor OUT old rook position
            zobrint_hash ^= zobrint_random_table[board[rookpos_new]][rookpos_new];  // xor  IN new rook position
        } else {  // pawn promotion
            num p = (mv.to < 30 ? WHITE : BLACK);
            switch (mv.prom) {
                case 'q':  p += QUEEN;   break;
                case 'r':  p += ROOK;    break;
                case 'b':  p += BISHOP;  break;
                case 'n':  p += KNIGHT;  break;
            }
            board[mv.to] = p;
            board_eval += eval_material(p) - eval_material(piece);
            board_eval += eval_piece_on_square(p, mv.to) - eval_piece_on_square(piece, mv.to);  // t0/t1 to compensate for wrong pieceval earlier
            zobrint_hash ^= zobrint_random_table[piece][mv.to];  // xor OUT promoting pawn
            zobrint_hash ^= zobrint_random_table[    p][mv.to];  // xor  IN promoted piece
        }
    }

    CASTLINGRIGHTS old_cr_w = CASTLINGWHITE;
    CASTLINGRIGHTS old_cr_b = CASTLINGBLACK;

    // lose right to castle if king or rook moves
    CASTLINGBLACK = {CASTLINGBLACK.lc and mv.from != 21 and mv.from != 25, CASTLINGBLACK.rc and mv.from != 25 and mv.from != 28};
    CASTLINGWHITE = {CASTLINGWHITE.lc and mv.from != 91 and mv.from != 95, CASTLINGWHITE.rc and mv.from != 95 and mv.from != 98};

    if (piece == WHITE + KING)      KINGPOS_WHITE = mv.to;
    else if (piece == BLACK + KING) KINGPOS_BLACK = mv.to;

    zobrint_hash ^= zobrint_random_table[1][0];  // switch side to move

    return {hit_piece, old_cr_w, old_cr_b};
}

// TODO XXX TODO: move LASTMOVE setting to make_move/unmake_move, and maybe return it instead of setting global
inline PiecePlusCatling make_null_move()
{
    // TODO: cannot null move if in check
    zobrint_hash ^= zobrint_random_table[1][0];  // switch side to move

    return {0, CASTLINGWHITE, CASTLINGBLACK};
}

inline void unmake_move(Move mv, num hit_piece, CASTLINGRIGHTS c_rights_w, CASTLINGRIGHTS c_rights_b)
{
    num piece = board[mv.to];
    board[mv.to] = hit_piece;
    board[mv.from] = piece;

    board_eval += eval_material(hit_piece) + eval_piece_on_square(hit_piece, mv.to);  // add captured piece back in
    board_eval -= eval_piece_on_square(piece, mv.to) - eval_piece_on_square(piece, mv.from);  // adjust position values of moved piece

    zobrint_hash ^= zobrint_random_table[piece][mv.from];
    zobrint_hash ^= zobrint_random_table[piece][mv.to];
    zobrint_hash ^= zobrint_random_table[hit_piece][mv.to];

    if (mv.prom)
    {
        if (mv.prom == 'e') {  // en passant
            num ep_square = 10 * (mv.from / 10) + (mv.to % 10);  // square of the CAPTURED/en-passanted pawn
            board[ep_square] = PAWN + (ep_square < 60 ? BLACK : WHITE);
            board_eval += eval_material(board[ep_square]) + eval_piece_on_square(board[ep_square], ep_square);  // captured pawn did not get removed earlier
            zobrint_hash ^= zobrint_random_table[board[ep_square]][ep_square];
        } else if (mv.prom == 'c') {  // castling -- undo rook move
            bool is_short_castling = mv.to > mv.from;  // otherwise long castling
            num rookpos_new = is_short_castling ? (mv.from + 1) : (mv.from - 1);
            num rookpos_old = is_short_castling ? (mv.from + 3) : (mv.from - 4);

            board[rookpos_old] = board[rookpos_new];
            board[rookpos_new] = 0;
            board_eval += eval_piece_on_square(board[rookpos_old], rookpos_old) - eval_piece_on_square(board[rookpos_old], rookpos_new);  // rook positioning
            zobrint_hash ^= zobrint_random_table[board[rookpos_old]][rookpos_old];
            zobrint_hash ^= zobrint_random_table[board[rookpos_old]][rookpos_new];
        } else {  // promotion
            num old_pawn = PAWN + (mv.to < 30 ? WHITE : BLACK);
            board[mv.from] = old_pawn;
            board_eval -= eval_material(piece) - eval_material(old_pawn);
            board_eval -= eval_piece_on_square(piece, mv.from) - eval_piece_on_square(old_pawn, mv.from);  // to compensate for wrong pieceval earlier
            zobrint_hash ^= zobrint_random_table[old_pawn][mv.from];
            zobrint_hash ^= zobrint_random_table[   piece][mv.from];
        }
    }

    CASTLINGWHITE = c_rights_w;
    CASTLINGBLACK = c_rights_b;

    if (piece == WHITE + KING)      KINGPOS_WHITE = mv.from;
    else if (piece == BLACK + KING) KINGPOS_BLACK = mv.from;

    zobrint_hash ^= zobrint_random_table[1][0];  // switch side to move
}

inline void unmake_null_move(CASTLINGRIGHTS c_rights_w, CASTLINGRIGHTS c_rights_b)
{
    CASTLINGWHITE = c_rights_w;
    CASTLINGBLACK = c_rights_b;

    zobrint_hash ^= zobrint_random_table[1][0];  // switch side to move
}

void make_move_str(const char* mv)
{
    num a = 8 - (mv[1] - '0');
    num b = (num)(mv[0] - 'a');
    num c = 8 - (mv[3] - '0');
    num d = (num)(mv[2] - 'a');
    char e = mv[4];

    if (abs(d - b) > 1 and board[10*a+20+b+1] & KING)  // castling
        e = 'c';
    if (b != d and board[10*a+20+b+1] & PAWN and not board[10*c+20+d+1])  // en passant (diagonal pawn move to empty square)
        e = 'e';
    if (e == '\n')
        e = '\0';

    Move to_mv = {10*a + 20 + b + 1, 10*c + 20 + d + 1, e};
    make_move(to_mv);
    LASTMOVE_GAME = to_mv;  // copies the struct

    board_positions_seen.insert(zobrint_hash);
}

void printf_move_eval(num depth, ValuePlusMove rec, bool accurate)  // print eval of move (before call to make_move)
{
    if (VERBOSITY < 1) return;

    // TODO support xboard
    if (MODE == MODE_UCI)
        printf("info depth %ld score cp %ld nodes %ld pv", depth, rec.value, NODES_NORMAL + NODES_QUIES);
    else if (MODE == MODE_XJ) {
        double time_expired = 1.0 * (std::clock() - SEARCH_START_CLOCK) / CLOCKS_PER_SEC;
        std::string move_str = move_to_str(rec.move, true);
        const char* cstr = move_str.c_str();
        printf("MOVE %15s |", cstr);

        printf(" EVAL %7.2f %c | D%2ld | NN%9ld | NQ%9ld | t%7.3f | VAR  ... ",
            (float)(rec.value) / 100, accurate ? ' ' : '?', SEARCH_ADAPTIVE_DEPTH, NODES_NORMAL, NODES_QUIES, time_expired);
    }

    // reveal PV from hash table, this saves work of returning a std::deque
    std::deque<Move> moves;
    std::deque<PiecePlusCatling> ppcs;

    num i = 0;  // prevent infinite repetition

    // TODO: revealing PV from hash table is faster but cuts off parts of it
    while ((TRANSPOS_TABLE[zobrint_hash & ZOB_MASK].zobrint_hash == zobrint_hash) && (i < 12)) {
        Move mv = TRANSPOS_TABLE[zobrint_hash & ZOB_MASK].move;

        // if (i == 0 && !(rec.move == mv)) std::exit(2);

        if (MODE == MODE_UCI)
            std::cout << " " << move_to_str(mv, false);
        else if (MODE == MODE_XJ) {
            if (i >= 1)
                std::cout << move_to_str(mv, true) << " ";
        }

        PiecePlusCatling ppc = make_move(mv);  // updates zobrist_hash
        moves.push_back(mv);
        ppcs.push_back(ppc);
        ++i;
    }

    // TODO: check pesudo-legality -> re-use pv_in_hash_table as function
    // TODO: check this after every mode + only print this in MODE_XJ
//    if ((TRANSPOS_TABLE[zobrint_hash & ZOB_MASK].zobrint_hash != zobrint_hash) and (TRANSPOS_TABLE[zobrint_hash & ZOB_MASK].zobrint_hash)
//            and not (TRANSPOS_TABLE[zobrint_hash & ZOB_MASK].move == NULLMOVE))
//        std::cout << "[HASH COLLISION] ";

    for (i = moves.size() - 1; i >= 0; i--) {
        PiecePlusCatling ppc = ppcs[i];
        unmake_move(moves[i], ppc.hit_piece, ppc.c_rights_w, ppc.c_rights_b);
    }

    std::cout << std::endl;
}

bool square_in_check(num COLOR, num i)
{
    num OPPONENT_COLOR = COLOR == WHITE ? BLACK : WHITE;
    num UP = COLOR == WHITE ? -10 : 10;

    // enemy pawns
    if (board[i+UP+1] == PAWN + OPPONENT_COLOR)
        return true;
    if (board[i+UP-1] == PAWN + OPPONENT_COLOR)
        return true;

    for (num piece = KNIGHT; piece <= KING; piece <<= 1)  // enemy pieces incl king
        for (num l = 0; PIECEDIRS[piece][l] != 0; ++l)
            for (num sq = i;;)
            {
                sq += PIECEDIRS[piece][l];
                if (board[sq] == OOB)  // axis goes off board
                    break;
                if (board[sq] == OPPONENT_COLOR + piece)  // opponent piece of right type on axis
                    return true;
                if (board[sq])  // irrelevant piece along axis
                    break;
                if (not PIECESLIDE[piece])
                    break;
            }

    return false;
}

bool king_in_check(num COLOR)  // 36% of time spent in this function
{
    // keeping track of king position enables a HUGE speedup!
    num KINGPOS = COLOR == WHITE ? KINGPOS_WHITE : KINGPOS_BLACK;
    return square_in_check(COLOR, KINGPOS);
}


// TODO TODO TODO replace with vector push_back OR AT LEAST fewer indirect mallocs
inline Move* move_store(Move* mvsend, num a, num b, num c)
{
    mvsend->from = a; mvsend->to = b; mvsend->prom = c;
    return ++mvsend;
}

inline Move* move_store_maybe_promote(Move* mvsend, bool is_promrank, num a, num b)
{
    if (is_promrank) {
        mvsend = move_store(mvsend, a, b, 'q'); /* best */
        mvsend = move_store(mvsend, a, b, 'n'); /* also likely */
        mvsend = move_store(mvsend, a, b, 'r');
        return   move_store(mvsend, a, b, 'b');
    } else
        return   move_store(mvsend, a, b, '\0');
}


inline bool is_pseudo_legal(num COLOR, Move mv)
{
    if (!(board[mv.from] & COLOR))  // moving empty/enemy piece (impossible)
        return false;
    if (board[mv.to] & COLOR)  // self-capture (impossible)
        return false;

    num NCOLOR = COLOR == WHITE ? BLACK : WHITE;
    num bijpiece = board[mv.from] & COLORBLIND;

    if (mv.prom) {
        if (mv.prom == 'c') {  // castling
            if (bijpiece != KING)
                return false;

            // castling (normal king movement is covered by PIECEDIRS)
            CASTLINGRIGHTS castlingrights = COLOR == WHITE ? CASTLINGWHITE : CASTLINGBLACK;
            num CASTLERANK = COLOR == WHITE ? 90 : 20;
            if (mv.from + 2 == mv.to)  // short castle O-O
                return (castlingrights.rc && !board[CASTLERANK+6] && !board[CASTLERANK+7] && board[CASTLERANK+8] == COLOR + ROOK);
            if (mv.from - 2 == mv.to)  // long castle O-O-O
                return (castlingrights.lc && !board[CASTLERANK+2] && !board[CASTLERANK+3] && !board[CASTLERANK+4] && board[CASTLERANK+1] == COLOR + ROOK);
            // this line should be unreachable
        }
        else {
            if (bijpiece != PAWN)
                return false;

            if (mv.prom == 'e') {  // en passant -- capture opponent pawn (this check replaces regular diagonal capture)
                num ADD = COLOR == WHITE ? -10 : 10;  // "up"
                return (LASTMOVE.from-ADD-ADD == LASTMOVE.to) && (board[LASTMOVE.to] == NCOLOR + PAWN) &&
                    (LASTMOVE.to == mv.from-1 || LASTMOVE.to == mv.from+1) && (mv.to == LASTMOVE.from-ADD);
            }
            // intentional fall-through, we still have to check if the requested forward/diagonal movement is possible for promotions
        }
    }

    if (bijpiece == PAWN) {
        num ADD = COLOR == WHITE ? -10 : 10;  // "up"
        num PROMRANK = COLOR == WHITE ? 30 : 80;
        if ((PROMRANK < mv.from && mv.from < PROMRANK + 10) ^ (mv.prom != '\0'))  // (on 7th rank) iff (has promotion piece set)
            return false;  // importantly, mv.prom == 'e' was handled earlier
        if (mv.from + ADD == mv.to)  // 1 square forward (needs empty square)
            return !board[mv.to];
        if ((mv.from + ADD + 1 == mv.to) || (mv.from + ADD - 1 == mv.to))  // if not en passant: can only move diagonally if occupied
            return !!board[mv.to];
        num STARTRANK = COLOR == WHITE ? 80 : 30;
        if (mv.from + ADD + ADD == mv.to)  // 2 squares forward (needs empty squares + startrank)
            return !board[mv.from + ADD] && !board[mv.from + ADD + ADD] && STARTRANK < mv.from && mv.from < STARTRANK + 10;
    } else
        for (num l = 0; PIECEDIRS[bijpiece][l] != 0; ++l) {
            for (num sq = mv.from;;)
            {
                sq += PIECEDIRS[bijpiece][l];
                if (sq == mv.to)  // move is at least theoretically possible ("pseudo-legal")
                    return true;
                if (board[sq])
                    break;
                if (not PIECESLIDE[bijpiece])
                    break;
            }
        }

    return false;
}


// generates all captures if captures=true, generates all quiet moves if captures=false
inline GenMoves gen_moves_maybe_legal(num COLOR, bool do_captures, bool do_quiets)  // 30% of time spent here
{
    // TODO: move these to global variables? and just overwrite?
    // TODO: generally avoid double indirection here
    Move* captures = do_captures ? ((Move*)malloc(125 * sizeof(Move))) : NULL;  // malloc appears faster than global array
    Move* captures_end = captures;
    Move* quiets = do_quiets ? ((Move*)malloc(125 * sizeof(Move))) : NULL;  // weird 10% slowdown only if BOTH calloc and >=126
    Move* quiets_end = quiets;

    num NCOLOR = WHITE;  // opponent color
    num ADD = 10;  // "up"
    num STARTRANK = 30;
    num PROMRANK = 80;
    num EPRANK = 60;
    num CASTLERANK = 20;
    CASTLINGRIGHTS castlingrights = CASTLINGBLACK;

    if (COLOR == WHITE) {  // maybe make 2 structs with these constants?
        NCOLOR = BLACK;
        ADD = -10;
        STARTRANK = 80;
        PROMRANK = 30;
        EPRANK = 50;
        CASTLERANK = 90;
        castlingrights = CASTLINGWHITE;
    }

    if (do_quiets) {  // castling moves
        //    TODO: FIX THIS!!! TRIES TO CASTLE WHEN BISHOP ON C8
        //    INPUT: position startpos moves d2d4 b8c6 e2e4 d7d5 e4d5 d8d5 g1f3 d5e4 f1e2
        //    MOVE   B f5  (c8f5 ) | KH        4 | EVAL    0.45   | VAR  ...   c4  (c2c4 ) Kxc8  (e8c8c)
        if (castlingrights.rc and board[CASTLERANK+5] == COLOR + KING and board[CASTLERANK+8] == COLOR + ROOK)  // short castle O-O
            if (not board[CASTLERANK+6] and not board[CASTLERANK+7])
                if (not square_in_check(COLOR, CASTLERANK+5) and not square_in_check(COLOR, CASTLERANK+6) and not square_in_check(COLOR, CASTLERANK+7))
                    quiets_end = move_store(quiets_end, CASTLERANK+5, CASTLERANK+7, 'c');
        if (castlingrights.lc and board[CASTLERANK+5] == COLOR + KING and board[CASTLERANK+1] == COLOR + ROOK) {  // long castle O-O-O
            if (not board[CASTLERANK+2] and not board[CASTLERANK+3] and not board[CASTLERANK+4])
                if (not square_in_check(COLOR, CASTLERANK+3) and not square_in_check(COLOR, CASTLERANK+4) and not square_in_check(COLOR, CASTLERANK+5))
                    quiets_end = move_store(quiets_end, CASTLERANK+5, CASTLERANK+3, 'c');
        }
    }

    // I made the mistake of using mailbox addressing here in 2015, so looping over all 64 squares (most empty) will always be slow
    // I ended up switching this engine to 12x10 boards for efficiency, but for bitboards I would probably do a new engine
    for (num i = 0; i < 120; ++i) {  // i = 21; i < 99 is SLOWER for some reason! :(
        if (not (board[i] & COLOR))  // also covers board[i] == OOB
            continue;
        if (board[i] & PAWN) {  // pawn moves
            if (do_captures) {
                // diagonal captures
                if (board[i+ADD+1] & NCOLOR)
                    captures_end = move_store_maybe_promote(captures_end, PROMRANK < i and i < PROMRANK + 10, i, i+ADD+1);
                if (board[i+ADD-1] & NCOLOR)
                    captures_end = move_store_maybe_promote(captures_end, PROMRANK < i and i < PROMRANK + 10, i, i+ADD-1);
                if (EPRANK < i and i < EPRANK + 10) {  // en passant capture
                    if (LASTMOVE.from == i+ADD+ADD-1 and LASTMOVE.to == i-1 and board[i-1] & PAWN)
                        captures_end = move_store(captures_end, i, i+ADD-1, 'e');
                    if (LASTMOVE.from == i+ADD+ADD+1 and LASTMOVE.to == i+1 and board[i+1] & PAWN)
                        captures_end = move_store(captures_end, i, i+ADD+1, 'e');
                }
            }
            if (do_quiets)
                if (not board[i+ADD]) {  // 1 square forward
                    quiets_end = move_store_maybe_promote(quiets_end, PROMRANK < i and i < PROMRANK + 10, i, i+ADD);
                    if (STARTRANK < i and i < STARTRANK + 10 and not board[i+ADD+ADD])  // 2 squares forward
                        quiets_end = move_store(quiets_end, i, i+ADD+ADD, '\0');
                }
        } else {  // piece moves
            num bijpiece = board[i] & COLORBLIND;
            for (num l = 0; PIECEDIRS[bijpiece][l] != 0; ++l) {
                for (num sq = i;;)
                {
                    sq += PIECEDIRS[bijpiece][l];
                    if (board[sq] == OOB)  // out of bounds
                        break;
                    if (board[sq] & COLOR)  // move to square with own piece (illegal)
                        break;
                    if (board[sq] & NCOLOR) {  // move to square with enemy piece (capture)
                        if (do_captures)
                            captures_end = move_store(captures_end, i, sq, '\0');
                        break;
                    }
                    if (do_quiets)  // move to empty square  // board[sq] HAS TO BE 0 at this point
                        quiets_end = move_store(quiets_end, i, sq, '\0');
                    if (not PIECESLIDE[bijpiece])
                        break;
                }
            }
        }
    }

    return {captures, captures_end, quiets, quiets_end};
}

inline void free_GenMoves(GenMoves gl)
{
    free(gl.captures);
    free(gl.quiets);
}


int move_order_mvv_lva(const void* a, const void* b)
{
    Move mv_a = *((Move *)a);
    Move mv_b = *((Move *)b);
    return 1000 * PIECEVALS[board[mv_b.to] & COLORBLIND] - PIECEVALS[board[mv_b.from] & COLORBLIND]
           - (1000 * PIECEVALS[board[mv_a.to] & COLORBLIND] - PIECEVALS[board[mv_a.from] & COLORBLIND]);
}


inline void store_hash_entry(Move move, num value, int8_t tte_flag, num depth, num depth_left, bool is_quies)
{
    // https://crypto.stackexchange.com/questions/27370/formula-for-the-number-of-expected-collisions
    // TODO: calculate expected number of collisions
    // TODO: why nullmove??
    if (!(move == NULLMOVE) && !is_quies)
        TRANSPOS_TABLE[zobrint_hash & ZOB_MASK] = {zobrint_hash, move, value, tte_flag, depth_left};
}

// non-negamax quiescent alpha-beta minimax search
// https://www.chessprogramming.org/Quiescence_Search
ValuePlusMove negamax(num COLOR, num alpha, num beta, num adaptive, bool is_quies, bool can_null_move, num depth, bool lines, bool lines_accurate)  // 22% of time spent here
{
    NODES_NORMAL += !is_quies;
    NODES_QUIES += is_quies;

    // treat all repeated positions as an instant draw to avoid repetitions when winning and encourage when losing
    if ((depth >= 1) and board_positions_seen.count(zobrint_hash))
        return {0, {0}};

    if (is_quies) {
        // TODO: limit quiescence search depth somehow
        if (NO_QUIES)
            return {COLOR == WHITE ? board_eval : -board_eval, {0}};
    }
    else if (depth >= 1) {
        TTEntry tte = TRANSPOS_TABLE[zobrint_hash & ZOB_MASK];
        if (tte.zobrint_hash == zobrint_hash)
            if (tte.depth_left >= adaptive) {
                if (tte.tte_flag == tte_exact)
                    return {tte.value, tte.move};
                if (tte.tte_flag == tte_alpha and tte.value <= alpha)
                    return {alpha, tte.move};
                if (tte.tte_flag == tte_beta and tte.value >= beta)
                    return {beta, tte.move};
            }

        // Main search is done, do quiescence search at the leaf nodes
        if (adaptive <= 0)
            return negamax(COLOR, alpha, beta, adaptive, true, false, depth, lines, lines_accurate);
    }

    ValuePlusMove best = {-inf, {0}};
    if (is_quies) {
        // "standing pat": to compensate for not considering non-capture moves, at least one move should be
        //    better than doing no move ("null move") -> avoids senseless captures, but vulnerable to zugzwang
        best = {COLOR == WHITE ? board_eval : -board_eval, {0}};

        if (best.value >= beta)
            return {beta, {0}};
        if (alpha < best.value)
            alpha = best.value;
    }

    Move LASTMOVE_BAK = LASTMOVE;

    // TODO: when switching to bitboards, also check if a piece other than a pawn is still on the board
    // TODO: Only one null move in entire search or just disallow 2 in a row?
    // Null Move Pruning
    // maybe okay to do without king_in_check-call as no duplicate moves possible and king will be captured instead?
    if (can_null_move && depth >= 1 && adaptive >= 3 && !king_in_check(COLOR))
    {
        PiecePlusCatling ppc = make_null_move();
        LASTMOVE = {0};

        ValuePlusMove rec = negamax(COLOR == WHITE ? BLACK : WHITE, -beta, -beta + 1, adaptive - 3, is_quies, false, depth + 1, lines, lines_accurate);

        unmake_null_move(ppc.c_rights_w, ppc.c_rights_b);
        LASTMOVE = LASTMOVE_BAK;

        if (-rec.value >= beta)
            return {beta, {0}};
    }

    num alpha_orig = alpha;
    Move mv = {0};
    PiecePlusCatling ppc;
    GenMoves gl = {0};
    Move* cur_mv = NULL;

    num adaptive_new = 0;
    num legal_moves_found = 0;

    num try_killer_move = -1;
    num alpha_raised_n_times = 0;
    Move pv_mv = TRANSPOS_TABLE[zobrint_hash & ZOB_MASK].move;
    bool pv_in_hash_table = (not is_quies) and (TRANSPOS_TABLE[zobrint_hash & ZOB_MASK].zobrint_hash == zobrint_hash) and is_pseudo_legal(COLOR, pv_mv);

    // per-move variables
    bool do_extend = false;

    // [1/3] try best move (hash/pv move) from previous search iterative deepening search first
    //  -> skip move generation if alpha-beta cutoff (3.6% fewer calls to genmoves, overall 1.4% speedup Sadge)
    if (pv_in_hash_table)
    {
        mv = pv_mv;
        ppc = make_move(mv);
        goto jump_into_loop_with_hash_move;
    } else
        pv_mv = NULLMOVE;

    while (true)
    {
        if ((gl.captures == NULL) and (gl.quiets == NULL))  // [2/3] after hash move, try captures
        {
            LASTMOVE = LASTMOVE_BAK;  // needs to be set for en passant captures in gen_moves_maybe_legal
            gl = gen_moves_maybe_legal(COLOR, true, false);
            cur_mv = gl.captures;

            // TODO: selection-type sort instead ? Or custom qsort?
            num caps_len = gl.captures_end - gl.captures;  // can only be 0 in illegal positions -> add assert statement
            if (caps_len > 1)
                qsort(gl.captures, caps_len, sizeof(Move), move_order_mvv_lva);  // TODO: use std::sort  // 7% of runtime
        }

        if (cur_mv == gl.captures_end)  // [3/3] after captures, try quiet moves
        {
            if (is_quies)
                break;

            free_GenMoves(gl);
            gl = gen_moves_maybe_legal(COLOR, false, true);
            cur_mv = gl.quiets;

            num quiets_len = gl.quiets_end - gl.quiets;
            if (quiets_len > 1)
                try_killer_move = 0;
        }

        if (try_killer_move > -1 and try_killer_move < MAX_KILLER_MOVES)
        {
            // try moves in killer table first
            // use selection sort instead of qsort because only 1 or 2 values in the list matter
            // note: WHEN we sort also plays a role because the KILLER_TABLE will be filled differently

            for (Move* swap = cur_mv; swap < gl.quiets_end; swap++) {
                if (*swap == KILLER_TABLE[depth][try_killer_move]) {  // triangle swap
                    Move tmp = *cur_mv;
                    *cur_mv = *swap;
                    *swap = tmp;
                    break;
                }
            }

            try_killer_move++;
        }

        if (cur_mv == gl.quiets_end)  // end of move list
            break;

        mv = *(cur_mv);

        // skip hash move that we already did before gen_moves_maybe_legal
        if (mv == pv_mv)
            goto continue_proper;  // already checked hash move

        // Delta pruning -- do not make_move for captures that can never raise alpha
        if (is_quies) {
            num eval_max_improve = (COLOR == WHITE ? 1 : -1) * board_eval + (PIECEVALS[board[mv.to] & COLORBLIND] + 200);

            if (eval_max_improve < alpha and not mv.prom)  // TODO: also check != 'c' and != 'e'
                goto continue_proper;
        }

        ppc = make_move(mv);

        if (king_in_check(COLOR)) {  // illegal move  // king_in_check takes 11s of the 30s program time!!
            unmake_move(mv, ppc.hit_piece, ppc.c_rights_w, ppc.c_rights_b);
            goto continue_proper;
        }

        jump_into_loop_with_hash_move:

        legal_moves_found++;

        LASTMOVE = mv;  // only needed for gen_moves_maybe_legal so okay to only set here

        ValuePlusMove rec;

        if (!is_quies) {  // "search extensions", i.e. searching checks or promoting pawns more deeply
            num OTHER_COLOR = COLOR == WHITE ? BLACK : WHITE;
            bool ext_check = king_in_check(OTHER_COLOR);  // https://www.chessprogramming.org/Check_Extensions
            bool ext_seventh_rank_pawn = ((30 < mv.to and mv.to < 39) or (80 < mv.to and mv.to < 89)) and (board[mv.to] & PAWN);
            do_extend = ext_check || ext_seventh_rank_pawn;
        }

        adaptive_new = adaptive - (1 - do_extend);

        // "Since there is not much to be gained in the last two plies of the normal search, one might disable PVS there"
        if (NO_PRINCIPAL_VARIATION_SEARCH or (depth > 5) or not (pv_in_hash_table and (alpha_raised_n_times >= 1)))
            // Normal alpha-beta search
            rec = negamax(
                COLOR == WHITE ? BLACK : WHITE,
                -beta, -alpha,
                adaptive_new, is_quies, can_null_move, depth + 1, lines, lines_accurate
            );
        else {
            // https://www.chessprogramming.org/Principal_Variation_Search
            // Idea of PV: assume hash move will already be best move, so use null window to aggressively prune other moves
            // Should be active when (alpha_raised_n_times == 1) (PV move raised alpha and no move beat PV move yet)
            // Why does ">= 1" also work (and even produce better results)?
            rec = negamax(  // try a null-window search that saves time if everything is below alpha
                COLOR == WHITE ? BLACK : WHITE,
                -(alpha+1), -alpha,
                adaptive_new, is_quies, can_null_move, depth + 1, lines, lines_accurate
            );
            if ((-rec.value) > alpha)  // costly re-search if above search fails
                rec = negamax(
                    COLOR == WHITE ? BLACK : WHITE,
                    -beta, -alpha,
                    adaptive_new, is_quies, can_null_move, depth + 1, lines, lines_accurate
                );
        }

        rec.value *= -1;

        if (rec.value > best.value)
            best = {rec.value, mv};

        unmake_move(mv, ppc.hit_piece, ppc.c_rights_w, ppc.c_rights_b);

        if ((depth == 0 and lines) or (VERBOSITY >= 4)) {
            for (int i = 0; i < depth; i++)
                printf("    ");
            // due to alpha-beta pruning, this isn't the "true" eval for suboptimal moves, unless lines_accurate=true
            bool accurate = (best.move == mv) or lines_accurate;
            ValuePlusMove mv_printable = {rec.value, mv};
            printf_move_eval(depth, mv_printable, accurate);
        }

        if (not is_quies and not (lines_accurate and depth == 0)) {
            if (best.value >= beta) {  // alpha-beta cutoff
                if (not ppc.hit_piece and mv.prom != 'e') {  // store non-captures producing cutoffs as killer moves
                    for (num i = 0; i < MAX_KILLER_MOVES; i++)
                        if (KILLER_TABLE[depth][i] == mv)
                            goto do_not_store_in_killer_table;

                    // shift old entries over by 1
                    for (num i = MAX_KILLER_MOVES - 1; i >= 1; i--)
                        KILLER_TABLE[depth][i] = KILLER_TABLE[depth][i-1];

                    KILLER_TABLE[depth][0] = mv;
                }

                do_not_store_in_killer_table:

                // TODO: okay to give back exact higher value here? should only happen in case of checkmate right?
                store_hash_entry(best.move, beta, tte_beta, depth, adaptive, is_quies);  // true value is >= beta
                free_GenMoves(gl);
                return best;
            }
            if (best.value > alpha) {  // alpha-raising (new low bound)
                alpha = best.value;
                alpha_raised_n_times++;
            }
        }

        // TODO: combine both cases
        // quiescence cutoff
        if (is_quies) {
            if (best.value >= beta) {
                best.value = beta;
                free_GenMoves(gl);
                return best;
            }
            if (best.value > alpha)
                alpha = best.value;
        }

        continue_proper:
        if (cur_mv != NULL)
            cur_mv++;
    }

    free_GenMoves(gl);

    // TODO: stalemate/checkmate detection here or in evaluation? adjust to depth
    if (!is_quies and !legal_moves_found) {  // "no legal moves" during normal search
        if (not king_in_check(COLOR))
            return {0, {0}};  // COLOR is stalemated
        return {-inf, {0}};  // COLOR is checkmated
    }

    if (is_quies and !legal_moves_found)  // "no captures" in quiescence search
        return best;

    if (is_quies)
        best.value = alpha;

    // for (exact) mate scores, we encode the distance, so we add 100cp per depth to the already-encoded distance
    if (best.value > inf/2)
        best.value = best.value - 100;
    if (best.value < -inf/2)
        best.value = best.value + 100;

    if (alpha > alpha_orig)
        store_hash_entry(best.move, best.value, tte_exact, depth, adaptive, is_quies);  // exact value
    else
        store_hash_entry(best.move, alpha, tte_alpha, depth, adaptive, is_quies);  // true value is <= alpha

    return best;
}


// find and play the best move in the position
// TODO: lines
std::string calc_move(bool lines = false)
{
    SEARCH_START_CLOCK = std::clock();

    NODES_NORMAL = 0;
    NODES_QUIES = 0;
    // re-use killer moves from previous move (2 plies ago) -> very small speed improvement
    for (num i = 0; i < 18; i++)
        for (num j = 0; j < MAX_KILLER_MOVES; j++)
            KILLER_TABLE[i][j] = KILLER_TABLE[i+2][j];
    // Since game positions are correlated and the TABLE_ZOB is checked we actually go faster if we do not clear this
    //memset(TRANSPOS_TABLE, 0, sizeof TRANSPOS_TABLE);

    num game_phase = 0;   // game phase based on pieces traded. initial pos: 24, rook endgame: 8
    for (num i = 0; i < 120; ++i) {
        num p = board[i];
        game_phase += 1 * !!(p & KNIGHT) + 1 * !!(p & BISHOP) + 2 * !!(p & ROOK) + 4 * !!(p & QUEEN);

        if (p == WHITE + KING)  KINGPOS_WHITE = i;
        if (p == BLACK + KING)  KINGPOS_BLACK = i;
    }

    if (VERBOSITY >= 2)
        printf((game_phase <= 8) ? "Setting piece-square tables to endgame\n" : "Setting piece-square tables to middlegame\n");
    set_piece_square_table(game_phase <= 8);

    // Have to re-calculate board info anew each time because GUI/Lichess might reset state
    board_eval = initial_eval();
    zobrint_hash = board_to_zobrint_hash();
    num my_color = IM_WHITE ? WHITE : BLACK;
    uint64_t OWN_TIME_TO_USE_MAX = (OWN_CLOCK_REMAINING / 20);  // assume 25-ish moves left
    Move mv = {0};

    if (VERBOSITY >= 2)
        printf("Starting iterative deepening alphabeta search at ZOB %ld eval %ld\n", zobrint_hash, board_eval);
    for (num search_depth = 1; search_depth <= MAX_SEARCH_DEPTH; search_depth++)
    {
        SEARCH_ADAPTIVE_DEPTH = search_depth;
        LASTMOVE = LASTMOVE_GAME;

        ValuePlusMove best_at_depth = negamax(my_color, -inf-1, inf+1, SEARCH_ADAPTIVE_DEPTH, false, true, 0, (VERBOSITY >= 3), (VERBOSITY >= 3));

        printf_move_eval(search_depth, best_at_depth, true);
        mv = best_at_depth.move;

        if ((best_at_depth.value <= -inf/2) or (best_at_depth.value >= inf/2)) {
            if (VERBOSITY >= 2)
                printf("Found mate score already so stopping iterative deepening early\n");
            break;
        }

        double time_expired = 1000.0 * (std::clock() - SEARCH_START_CLOCK) / CLOCKS_PER_SEC;  // millis
        if (time_expired * 5 > OWN_TIME_TO_USE_MAX) {  // assume next depth takes 5 times as long
            if (VERBOSITY >= 2)
                printf("Used up time budget of %7ldcs allocated to finding this move\n", OWN_TIME_TO_USE_MAX);
            break;
        }
    }
    if (VERBOSITY >= 2)
        printf("|End|ing iterative deepening alphabeta search at ZOB %ld eval %ld\n", zobrint_hash, board_eval);

    make_move(mv);
    board_positions_seen.insert(zobrint_hash);

    return move_to_str(mv);
}


void init_data(void) {
    // TODO use std::array or map with vector inside or something to avoid leaks
    board_initial_position();
    init_zobrint();

    PIECEDIRS[KNIGHT][0] = 12;
    PIECEDIRS[KNIGHT][1] = 21;
    PIECEDIRS[KNIGHT][2] = -8;
    PIECEDIRS[KNIGHT][3] = 19;
    PIECEDIRS[KNIGHT][4] = -12;
    PIECEDIRS[KNIGHT][5] = -21;
    PIECEDIRS[KNIGHT][6] = 8;
    PIECEDIRS[KNIGHT][7] = -19;
    PIECEDIRS[KNIGHT][8] = 0;

    PIECEDIRS[BISHOP][0] = 11;
    PIECEDIRS[BISHOP][1] = 9;
    PIECEDIRS[BISHOP][2] = -9;
    PIECEDIRS[BISHOP][3] = -11;
    PIECEDIRS[BISHOP][4] = 0;

    PIECEDIRS[ROOK][0] = 1;
    PIECEDIRS[ROOK][1] = 10;
    PIECEDIRS[ROOK][2] = -1;
    PIECEDIRS[ROOK][3] = -10;
    PIECEDIRS[ROOK][4] = 0;

    for (num piece = QUEEN; piece <= KING; piece += KING - QUEEN) {  // what
        PIECEDIRS[piece][0] = 1;
        PIECEDIRS[piece][1] = 10;
        PIECEDIRS[piece][2] = -1;
        PIECEDIRS[piece][3] = -10;
        PIECEDIRS[piece][4] = 11;
        PIECEDIRS[piece][5] = 9;
        PIECEDIRS[piece][6] = -9;
        PIECEDIRS[piece][7] = -11;
        PIECEDIRS[piece][8] = 0;
    }

    PIECESLIDE[KNIGHT] = false;
    PIECESLIDE[BISHOP] = true;
    PIECESLIDE[ROOK] = true;
    PIECESLIDE[QUEEN] = true;
    PIECESLIDE[KING] = false;

    // https://lichess.org/@/ubdip/blog/finding-the-value-of-pieces/PByOBlNB
    PIECEVALS[0] = 0;
    PIECEVALS[OOB] = 0;
    PIECEVALS[PAWN] = 100;
    PIECEVALS[KNIGHT] = 305;
    PIECEVALS[BISHOP] = 325;  // Should not trade bishop for a knight, unless it wins a pawn or king pawn structure becomes damaged
    PIECEVALS[ROOK] = 470;  // Engine keeps trading its knight+bishop for a rook+pawn, thinking it is a good trade, which it is not
    PIECEVALS[QUEEN] = 950;  // Engine also trades into having 2 rooks for a queen, this is usually also not worth it
    PIECEVALS[KING] = inf_king;

    for (num piece = PAWN; piece <= KING; piece <<= 1) {
        PIECEVALS[piece + WHITE] =  PIECEVALS[piece];
        PIECEVALS[piece + BLACK] = -PIECEVALS[piece];
    }
}


void test_fen(const char* desc, const char* fen)
{
    std::cout << desc << std::endl;
    board_from_fen(fen);
    pprint();
    std::cout << calc_move(true) << std::endl;
}

void test()
{
    MODE = MODE_XJ;
    VERBOSITY = 2;
    MAX_SEARCH_DEPTH = 7;
    OWN_CLOCK_REMAINING = 9999999;

    // Should find Ra1 being mate (eval=299) and Rh7 being stalemate (eval=0)
    test_fen("Tests Mate-in-1 (and stalemate)", "k1K5/8/8/8/8/8/8/7R w - - 0 1");
    test_fen("\nTests Mate-in-2", "1k5r/ppp5/8/8/8/3Q4/8/2KR4 w - - 0 1");
    test_fen("\nTests Mate-in-3", "2k4r/ppp5/8/8/8/3Q4/8/2KR4 w - - 0 1");
    test_fen("\nTests Mate-in-4 (and promotion)", "k1K5/8/1P6/8/8/8/8/8 w - - 0 1");
    test_fen("\nTests 2 queens mate-in-3 (should stop early)", "4Q3/8/8/5Q2/8/2K5/8/6k1 w - - 0 1");
    test_fen("\nTests Promotion", "k1K5/6P1/8/8/8/8/8/8 w - - 0 1");
    test_fen("\nTests Stalemate tactics from https://lichess.org/VdE6aggQ/black#74", "4Q3/6pk/6Np/7P/3P4/8/PPq2PK1/6R1 b - - 4 36");  // Qxg3+ leads to forced stalemate at depth 3

    std::cout << "\nTests En-Passant" << std::endl;
    board_initial_position();
    make_move_str("e2e4"); make_move_str("a7a6");
    make_move_str("e4e5"); make_move_str("d7d5");
    pprint();
    std::cout << calc_move(true) << std::endl;

    std::cout << "\nTests Castling" << std::endl;
    board_initial_position();
    board[96] = 0;
    board[97] = 0;
    board[85] = 0;
    pprint();
    IM_WHITE = true;
    std::cout << calc_move(true) << std::endl;

    std::cout << "\nTests detecting/avoiding repetitions" << std::endl;
    board_initial_position();
    IM_WHITE = true;
    zobrint_hash ^= zobrint_random_table[board[24]][24];
    board[24] = 0;  // up a queen so shouldn't draw
    board_eval += PIECEVALS[QUEEN] + PIECE_SQUARE_TABLES[QUEEN][24];
    board_positions_seen.insert(board_to_zobrint_hash());
    make_move_str("g1f3"); make_move_str("g8f6");
    make_move_str("f3g1"); make_move_str("f6g8");
    make_move_str("g1f3"); make_move_str("g8f6");
    pprint();
    std::cout << calc_move(true) << std::endl;  // Should see that f3g1 would be 3-fold

    test_fen("\nTests Lasker Trap in Albin Countergambit", "rnbqk1nr/ppp2ppp/8/4P3/1BP5/8/PP2KpPP/RN1Q1BNR b kq - 1 7");
    test_fen("\nTests Lasker Trap in Albin Countergambit", "rnbqk1nr/ppp2ppp/8/4P3/1bPp4/4P3/PP1B1PPP/RN1QKBNR b KQkq - 2 5");
    test_fen("\nTests going into a fork https://lichess.org/A1Jn5Z5s#53", "3rr1k1/p4pp1/1p4np/3Ppb2/1PP1N3/4R1P1/5PBP/4R1K1 b - - 4 27");  // should NOT play Rc8??
    test_fen("\nTests missed en-passant https://lichess.org/4bSSvnGS/white#60", "r5k1/p4p2/2n1b2p/5q2/2p1R3/P1N2PB1/QP4KP/8 w - - 14 31");  // should NOT play 31. b4??
    test_fen("\nTests missed knight move https://lichess.org/jOQqZlqM#42", "r3r1k1/1pb2pp1/2P2p2/1p6/8/P1NR2p1/5PPP/5RK1 w - - 0 22");  // should NOT play 22. hxg3?, rather Nxb5 or Nd5

    board_initial_position();
    printf("\nINITIAL BOARD EVAL: %ld\n", board_eval);
    make_move_str("e2e4");
    printf("1. e4 EVAL: %ld\n", board_eval);
    IM_WHITE = false; std::cout << calc_move(true) << std::endl;
    make_move_str("b1c3");
    IM_WHITE = false; std::cout << calc_move(true) << std::endl;
    make_move_str("g1f3");
    IM_WHITE = false; std::cout << calc_move(true) << std::endl;

    test_fen("\nTests Rc8 blunder caused by wrong continue https://lichess.org/1NIkB4Dg/black#48", "r3q1k1/p2n3p/1pQ2np1/3p4/3P4/1P1RP1P1/P4P1P/2R3K1 b - - 0 24");  // should be ~0.75ish, not -3.90 as in the game
    test_fen("\nTests iliachess game https://www.chess.com/analysis/game/live/50110768933", "r3r1k1/pbq2p1R/1p2nQp1/8/6P1/2PBpPP1/PP4K1/4R3 b - - 0 22");
    test_fen("\nBf5 blunder from https://lichess.org/Cwt4L3df/black#81", "8/6pk/6bp/p7/P7/2R1R1P1/1P6/2KB1q2 b - - 3 41");  // found Bf5 at depth 7 which is a blunder :/
    test_fen("\nTests pawn loss tactics https://lichess.org/5pQ86dA7/white#42", "2k1rb2/1bpp1p2/p6p/1p1N4/2P5/1P6/1P1N1PPP/5RK1 w - - 1 22");
    //test_fen("\nTests pawn loss tactics https://lichess.org/5pQ86dA7/white#42", "2k1r3/1bpp1p2/p6p/1pb5/2P5/1P2N3/1P1N1PPP/5RK1 w - - 3 23");
    test_fen("Qxf3+ sac 34 https://lichess.org/2RWlBUCy#66", "1k2r3/2b3p1/1pp2pPp/r7/P2P4/2R2q2/2QR1PP1/6K1 w - - 0 34");
    test_fen("Qxf3+ sac 33 https://lichess.org/2RWlBUCy#64", "1k2r3/2b3p1/1pp2pPp/r2q4/P2P4/1R3N2/2QR1PP1/6K1 w - - 3 33");

//    MAX_SEARCH_DEPTH = 12;
//    test_fen("TODO", "8/8/p6P/P6k/4p1p1/6K1/8/8 b - - 0 52");  // produces WEIRDNESS on DEPTH 12 -> 64bit hash collision?

    std::exit(0);
}

uint64_t perft(num depth_left, num COLOR)
{
    if (depth_left == 0)
        return 1;

    uint64_t nodes = 0;
    GenMoves gl = gen_moves_maybe_legal(COLOR, true, true);
    Move* cur = gl.quiets;

    while (cur != gl.captures_end) {
        while (cur != gl.quiets_end && cur != gl.captures_end)
        {
            Move mv = *cur;
            PiecePlusCatling ppc = make_move(mv);
            LASTMOVE = mv;

            if (!king_in_check(COLOR))
                nodes += perft(depth_left - 1, COLOR == WHITE ? BLACK : WHITE);

            unmake_move(mv, ppc.hit_piece, ppc.c_rights_w, ppc.c_rights_b);
            cur++;
        }

        if (cur == gl.quiets_end)
            cur = gl.captures;
    }

    free_GenMoves(gl);

    return nodes;
}


// Verify number of legal moves: https://www.chessprogramming.org/Perft_Results
void test_perft()
{
    const char* fens[7] = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",  // initial position
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        "rnbqkbnr/p1p1pppp/1p6/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3",  // own idea: test LASTMOVE
    };
    // expected number of leaf nodes at depth $index
    const uint64_t nodes[7][6] = {
        {1, 20,  400,  8902,  197281,   4865609},
        {1, 48, 2039, 97862, 4085603, 193690690},
        {1, 14,  191,  2812,   43238,    674624},
        {1,  6,  264,  9467,  422333,  15833292},
        {1, 44, 1486, 62379, 2103487,  89941194},
        {1, 46, 2079, 89890, 3894594, 164075551},
        {1, 31,  836, 25874,  689955,  22041575},
    };

    for (num i = 0; i < 7; i++)
    {
        board_from_fen(fens[i]);
        std::cout << "Perft Position " << (i + 1) << std::endl;
        for (num d = 0; d < 6; d++) {
            LASTMOVE = LASTMOVE_GAME;
            uint64_t p = perft(d, IM_WHITE ? WHITE : BLACK);
            std::cout << (p == nodes[i][d] ? "OK" : "-- WRONG -->") << " " << d << " " << p << std::endl;
        }
    }

    std::exit(0);
}


std::vector<std::string> split_string(std::string line_cpp)
{
    std::string buf;
    std::stringstream ss(line_cpp);
    std::vector<std::string> tokens;

    while (ss >> buf)
        tokens.push_back(buf);

    return tokens;
}


int main(int argc, char const *argv[])
{
    setbuf(stdout, NULL);
    init_data();

    if (argc >= 2 and strcmp(argv[1], "-t") == 0)
        test();

    if (argc >= 2 and strcmp(argv[1], "-p") == 0)
        test_perft();

    // TODO: add --verbosity and other command line arguments

    std::string line_cpp;

    while (true) {
        std::getline(std::cin, line_cpp);
        line_cpp.append(1, '\n');
        std::vector<std::string> tokens = split_string(line_cpp);
        const char* line = line_cpp.c_str();

        if (VERBOSITY >= 2)
            printf("INPUT: %s", line);

        if (strcmp(line, "xboard\n") == 0) {
            printf("feature myname=\"Jangine\"\n");
            // printf("feature sigint=0 sigterm=0\n");
            printf("feature ping=1\n");
            printf("feature setboard=1\n");
            printf("feature done=1\n");
            MODE = MODE_XBOARD;
        }
        if (startswith("time", line)) {
            // time left is given as centiseconds (1/100) but we store it as milliseconds (1/1000)
            OWN_CLOCK_REMAINING = 10 * std::stoi(line_cpp.substr(5));
        }

        if (startswith("verbosity", line)) {  // custom jangine extension
            VERBOSITY = std::stoi(tokens[1]);
        }

        /*** UCI ***/
        if (strcmp(line, "uci\n") == 0) {
            printf("id name Jangine\n");
            printf("id author Jan-Frederik Konopka\n");
            printf("uciok\n");
            MODE = MODE_UCI;
        }

        if (strcmp(line, "isready\n") == 0) {
            printf("readyok\n");
        }

        if (strcmp(line, "ucinewgame\n") == 0) {
            IM_WHITE = false;
            board_initial_position();
        }

        // TODO: "position fen 2b2rk1/p1p4p/2p1p1p1/br2N1Q1/1p2q3/8/PB3PPP/3R1RK1 w - - 0 1 moves e5f7"
        if (startswith("position startpos moves", line)) {
            num_moves = 0;
            board_initial_position();
            for (size_t i = 3; i < tokens.size(); ++i) {
                make_move_str(tokens[i].c_str());
                num_moves++;
            }
            IM_WHITE = (num_moves + 1) % 2;
        }
        /*** UCI END ***/

        /*** XBOARD ***/
        if (startswith("setboard", line)) {
            std::string fen = line_cpp.substr(9);
            board_from_fen(fen.c_str());
            pprint();
        }
        /*** XBOARD END ***/

        if (strcmp(line, "force\n") == 0 or startswith("result", line)) {
            started = false;
        }

        if (strcmp(line, "white\n") == 0) {
            IM_WHITE = true;
            num_moves = 0;
        }

        if (strcmp(line, "black\n") == 0) {
            IM_WHITE = false;
            num_moves = 1;
        }

        if (strcmp(line, "new\n") == 0) {
            IM_WHITE = false;
            started = true;  // TODO this line needed to fix xboard for white
            board_initial_position();
        }

        // TODO: "protover 2" -> reset board

        if (input_is_move(line)) {
            make_move_str(line);
            num_moves++;
            if (started) {
                pprint();
                IM_WHITE = (num_moves + 1) % 2;
                std::string mv = calc_move(true);
                num_moves++;
                pprint();
                printf((MODE == MODE_UCI) ? "bestmove %s\n" : "move %s\n", mv.c_str());
            }
        }

        if (startswith("go", line)) {
            IM_WHITE = (num_moves + 1) % 2;

            for (size_t i = 0; i < tokens.size(); ++i)
                if ((IM_WHITE && strcmp(tokens[i].c_str(), "wtime") == 0) || (!IM_WHITE && strcmp(tokens[i].c_str(), "btime") == 0))
                    OWN_CLOCK_REMAINING = std::stoi(tokens[i+1]);  // time left in MILLISECONDS

            started = true;
            std::string mv = calc_move(true);
            num_moves++;
            pprint();
            printf((MODE == MODE_UCI) ? "bestmove %s\n" : "move %s\n", mv.c_str());
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
