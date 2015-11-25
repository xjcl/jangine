

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


# log_to_board("""INFO:root:>>> BR    BB BQ BK BB BN BR
#     INFO:root:>>> BP    BP BP BP BP BP BP
#     INFO:root:>>> BP
#     INFO:root:>>>
#     INFO:root:>>>             WP
#     INFO:root:>>>                WN
#     INFO:root:>>> WP WP WP WP    WP WP WP
#     INFO:root:>>> WR WN WB WQ WK       WR""")

# bestmove = quiescence(BLACK).moves[0]
# print('bestmove', bestmove)


exit()

