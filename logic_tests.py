

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
pprint()
make_move(quiescence(BLACK).moves[0])


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
pprint()
make_move(quiescence(BLACK).moves[0])
# with depth=3 this will be 'wrong' as it thinks pulling back
#   the bishop will result in the loss of the Q



    log_to_board("""INFO:root:>>> BR       BQ BK BB BN BR
        INFO:root:>>> BP BP BP    BP BP BP BP
        INFO:root:>>>
        INFO:root:>>>          WP
        INFO:root:>>>    BN             BB
        INFO:root:>>> WP    WN    WP
        INFO:root:>>>    WP WP WQ       WP WP
        INFO:root:>>> WR    WB    WK WB WN WR """)
    pprint()
    for mv in quiescence(BLACK).moves:  # eg [(4, 1, 6, 2), (6, 3, 6, 2), (0, 3, 3, 3), (7, 2, 6, 3)]
        # WHY DOES IT INSIST ON PUTTING THE Q IN THE CENTER!!??! ARGH
        # found it. faulty transposition table (not compatible with ab-pruning?)
        make_move(mv)
        pprint()
    turochamp(loud=True)





    log_to_board("""\
    INFO:root:>>> BR    BB BQ BK BB BN BR
    INFO:root:>>> BP BP    BP    BP BP BP
    INFO:root:>>>       BP
    INFO:root:>>>
    INFO:root:>>> WB       BP WP
    INFO:root:>>>       WP
    INFO:root:>>> WP WP    WP    WP WP WP
    INFO:root:>>> WR WN WB WQ WK       WR""")

    print(quiescence(BLACK))
    # if Q to (3,6) --> ignores white castling






log_to_board("""INFO:root:>>> BR    BB BQ BK BB BN BR
    INFO:root:>>> BP BP BP BP    BP BP BP
    INFO:root:>>>       BN    BP
    INFO:root:>>>
    INFO:root:>>>          WP WP
    INFO:root:>>>                WN
    INFO:root:>>> WP WP WP       WP WP WP
    INFO:root:>>> WR WN WB WQ WK WB    WR""")

bestmove = quiescence(BLACK).moves[0]
print('bestmove', bestmove)
assert bestmove != (2, 2, 3, 4)



    log_to_board("""INFO:root:>>> BR    BB BQ BK BB    BR
INFO:root:>>> BP    BP    BP BP BP BP
INFO:root:>>> BN BP    BP          BN
INFO:root:>>>
INFO:root:>>>       WB WP WP
INFO:root:>>>                WN
INFO:root:>>> WP WP WP       WP WP WP
INFO:root:>>> WR WN WB WQ       WK WR""")

    # for mv in genlegals(BLACK):
    #     hp = make_move(mv)
    #     for mv2 in genlegals(WHITE):
    #         hp2 = make_move(mv2)
    #         print(mv, mv2, turochamp())
    #         unmake_move(mv2, hp2)
    #     unmake_move(mv, hp)

    bestmove = quiescence(BLACK).moves[0]
    print('bestmove', bestmove)
    assert bestmove != (0, 2, 4, 6)

make_move((0, 2, 4, 6))

bestmove = quiescence(WHITE).moves[0]
print('bestmove', bestmove)
assert bestmove == (4, 2, 2, 0) or bestmove == (7, 2, 2, 7)

# y u no see white's  (4,2,2,0)



make_move((0, 2, 4, 6))
make_move((4, 2, 5, 3))  # XXX
print(turochamp())




log_to_board("""INFO:root:>>> BR    BB BQ BK BB BN BR
    INFO:root:>>> BP BP BP BP BP BP BP BP
    INFO:root:>>>       BN
    INFO:root:>>>          WP
    INFO:root:>>>
    INFO:root:>>>
    INFO:root:>>> WP WP WP    WP WP WP WP
    INFO:root:>>> WR WN WB WQ WK WB WN WR""")

bestmove = quiescence(BLACK).moves[0]
print('bestmove', bestmove)


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




    # XXX didn't consider EP once

    # XXX maybe mobility thru friendly piece (/= pawn) counts half or so
    # XXX



    # nonsensical rook sac
    CASTLINGWHITE = (False, False, False)
    CASTLINGBLACK = (False, False, False)

    log_to_board("""\
    INFO:root:>>>       BB BK BR
    INFO:root:>>>       BP       BQ
    INFO:root:>>>       WQ       WP
    INFO:root:>>>       BP       BR
    INFO:root:>>>    BP WP             WB
    INFO:root:>>> BP WP    WP          WP
    INFO:root:>>> WP    WK             WR
    INFO:root:>>>                   WR""")

    # make_move(())

    print(quiescence(BLACK))




    # it has to take the pawn here
    log_to_board("""\
    INFO:root:>>> BR    BB    BK BB BN
    INFO:root:>>>    BP BP    BP BP BP
    INFO:root:>>>       BN
    INFO:root:>>> BP          BR
    INFO:root:>>>          BQ WN       BP
    INFO:root:>>>          WB    WP
    INFO:root:>>> WP WP WP WB WQ    WP WP
    INFO:root:>>> WR          WK    WN WR""")

    print(quiescence(BLACK))








    # don't throw away rook
    CASTLINGWHITE = (False, False, False)
    CASTLINGBLACK = (False, False, False)

    log_to_board("""\
    INFO:root:>>>       BB BK BR
    INFO:root:>>>       BP       BQ
    INFO:root:>>>       WQ       WP
    INFO:root:>>>       BP       BR
    INFO:root:>>>    BP WP             WB
    INFO:root:>>> BP WP    WP          WP
    INFO:root:>>> WP    WK             WR
    INFO:root:>>>                   WR""")

    print(quiescence(BLACK))




    # actually, this is ok. 4 3 6 1 leads to no advantage after B forks R and Q
    log_to_board("""\
    INFO:root:>>> BR    BB    BK BB BN
    INFO:root:>>>    BP BP    BP BP BP
    INFO:root:>>>       BN
    INFO:root:>>> BP          BR
    INFO:root:>>>          BQ WN       BP
    INFO:root:>>>          WB    WP
    INFO:root:>>> WP WP WP WB WQ    WP WP
    INFO:root:>>> WR          WK    WN WR""")

    print(quiescence(BLACK))



    # e.p. troubles
    log_to_board("""\
    INFO:root:>>> BK
    INFO:root:>>>
    INFO:root:>>>
    INFO:root:>>>
    INFO:root:>>>                   WP BP
    INFO:root:>>>                      WP
    INFO:root:>>>
    INFO:root:>>> WK""")
    LASTMOVE = (6,6,4,6)
    pprint()
    print(genlegals(BLACK))
    pprint()
    print(genlegals(BLACK))
    print(quiescence(BLACK))




    log_to_board("""\
    INFO:root:>>>       BK
    INFO:root:>>>       BP
    INFO:root:>>>
    INFO:root:>>>
    INFO:root:>>>    WB WP             WP
    INFO:root:>>>          WP    WK
    INFO:root:>>>
    INFO:root:>>> BQ""")
    # not throwing the Q



    # WE NEED THREE FOLD REP RULE!!!
    # nope! just ridding of tranpos did the job

    # INFO:root:>>> WK    BK
    # INFO:root:>>>
    # INFO:root:>>>          BQ
    # INFO:root:>>>
    # INFO:root:>>>
    # INFO:root:>>>
    # INFO:root:>>>
    # INFO:root:>>>



    repl for testing:
    while True:
        make_move(quiescence(BLACK).moves[0])
        pprint()
        make_move_str(input("do> "))

