# jangine -- Jan's C++ chess engine

Chess engine I wrote in 2022 (some parts in 2015)
- Algorithms used:
    - **Evaluation** is very simple. It is based entirely on (material counting) and piece-square tables,
        meaning each piece type gets a value based on the square it is on and the game phase (middlegame/endgame).
        The values were obtained from looking at other engines and using my intuition and testing.

    - **Search**: Traditional alpha-beta search with quiescence search for correctness. Iterative deepening
        is used so that the best move from depth-n-search can be stored as hash move in the hash table and tried
        first during depth-n+1-search. Zobrint hashes are used for this purpose (and for repetition detection).
        In order to generate as many alpha-beta cutoffs as possible, moves are tried in the following order:
        1. Hash move
        2. Captures in MVV-LVA order
        3. Killer moves (killer heuristic)
        4. Other quiet promotions
        5. Other quiet moves

    - No principal variation search (did not speed program up), no null move pruning, 
        no late move reductions (risky!), no futility pruning (check cannot be detected efficiently),
        no static exchange evaluation

    - **Opening/endgame optimizations**: No opening or endgame database is built-in, and should be provided.
        No specialized strategies either, so mating up one queen is difficult. 
 
- Strength (git version 33d1b644):
    - Was able to beat my NM friend 6-0 in Blitz (3+0) and 37½-2½ (35W 3D 1L) in Bullet (1+0)
    - Not officially assessed by [CCRL](http://ccrl.chessdom.com/ccrl/404/), but it scored
      17-83 in a match against Honzovy Šachy 2.0, indicating strength of around 1800

- Time management:
  - Iterative deepening, keep deepening while trying to use under 5% of the remaining time 


This was my first C++ project back in 2015 which is why there are code quality issues.
The project also got switched from C to C++ which is why C style is used.
Since many features (e.g. using a std::map vs a static C array) of C++ are too slow for
my use case I am thinking of switching the engine back to C entirely.


Challenge me on lichess! https://lichess.org/@/jangine

![](./jangine_lichess_scr2.png)

Test suite running in console:

![](./jangine_console_scr.png)


## Usage

Build:

    g++ -O2 jangine.cpp -o jangine

Run tests for rules (checkmate, stalemate, castling, promotion, en passant, etc.) and benchmark speed:

    ./jangine -t

Run as interactive (stdin/stdout) UCI engine:

    ./jangine

    ./jangine | tee -a jangine.log  # logging to file

To host bot for lichess, clone the lichess-bot project https://github.com/ShailChoksi/lichess-bot and place the engine binary in the "engines/" folder.
