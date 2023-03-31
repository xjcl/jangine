# Jangine -- Jan's C++ chess engine

**Jangine** is a chess engine (computer program that plays chess) I wrote in 2022 (parts in 2015).

<img height="200px" src="jangine_logo_dall-e.png" width="200px"/>

_Logo of Jangine, created by DALL-E_

- **Board representation** is done with [10x12 boards](https://www.chessprogramming.org/10x12_Board).
  - This is similar to 8x8 boards but only needs a single
      index to address each square, instead relying on sentinel values for efficient out-of-bounds detection.
      This leads to a simple programming model.
  - Speed could be improved with the more advanced technique of bitboards which I haven't had time to research yet.

- **Evaluation** is very simple. It is based entirely on material counting and
  [piece-square tables](https://www.chessprogramming.org/Piece-Square_Tables).
  - This means each piece type gets a value based on the square it is on and the game phase (middlegame/endgame).
  - The square values were obtained from looking at other engines, my chess player intuition, and testing.
  - The piece values were chosen to stop the engine from giving up N+B for R+P or Q for R+R.

- **Search**: Traditional [alpha-beta search](https://www.chessprogramming.org/Alpha-Beta) with
  [quiescence search](https://www.chessprogramming.org/Quiescence_Search) for correctness.
  - Transposition table of constant size 176 MiB for storing information about searched positions.
  - Iterative deepening is used so that the best move from depth-n-search can be stored as hash move in the hash table
    and tried first during depth-(n+1)-search.
  - Null move pruning, delta pruning
  - Depth extensions for
      1. checks
      2. pawn reaching seventh rank
  - In order to generate as many alpha-beta cutoffs as possible, moves are tried in the following order:
      1. Hash move
      2. Captures in MVV-LVA order
      3. Killer moves (killer heuristic)
      4. Other quiet moves (incl. promotions) in no particular order

- For now: No principal variation search (did not speed program up),
    no late move reductions (risky!), no futility pruning (I cannot detect checks efficiently),
    no static exchange evaluation.

- **Opening/endgame optimizations**:
  - No opening or endgame database is built-in, and should be provided externally.
  - No specialized strategies either, so even simple endgame plans might not be found.

- **Time management**:
  - Iterative deepening, keep deepening while trying to use under 5% of the remaining time.

- **Language choice**:
  - C (later C++) was chosen in 2015 due to superior speed over most languages.
  - Because I had no C/C++ experience in 2015 (and still don't have very much), there are code quality issues.
  - Since many features (e.g. using a std::map vs a static C array) of C++ are too slow for
      my use case I am thinking of switching the engine back to C entirely.

- **UCI/XBoard support**: Partial, both being worked on.

- **GUI**: I use the Arena Chess GUI for testing.

- **Strength**: On a (Ryzen 3600X single-core):
    - [33d1b644 2022-06-20] Was able to beat my NM friend 6-0 in Blitz (3+0) and 37½-2½ (35W 3D 1L) in Bullet (1+0)

**Play me on Lichess!** https://lichess.org/@/jangine

<img src="./jangine_lichess_scr2.png" width="45%"/> <img src="./jangine_console_scr.png" width="45%"/>


## Usage

Build:

    g++ -Ofast jangine.cpp -o jangine

Build on Linux for Windows:

    sudo apt install mingw-w64
    x86_64-w64-mingw32-g++ -Wall -c -g -Ofast jangine.cpp -o jangine.o
    x86_64-w64-mingw32-g++ -static -static-libgcc -static-libstdc++ -o jangine.exe jangine.o

Run the included test suites:

    # Run set of demo positions (checkmate, stalemate, promotion, en passant, etc.)
    ./jangine -t

    # Solve tactics exercises from the book "Win At Chess"
    python test_win_at_chess.py  # Python >= 3.7

Run as an interactive (stdin/stdout) UCI engine:

    ./jangine

To host bot for lichess, clone the lichess-bot project https://github.com/ShailChoksi/lichess-bot and place the
engine binary in the `engines/` folder.


## Versions

| Version    | G1920 2+1         | est. CCRL | G2315 2+1         | est. CCRL | CCRL      |
|------------|-------------------|-----------|-------------------|-----------|-----------|
| 2022-12-01 | -                 | -         | -                 | -         | -         |
| 2022-12-27 | -                 | -         | -                 | -         | -         |
| 2022-12-29 |  82.0/110 (74.5%) | 2106      |  18.5/100 (18.5%) | 2057      | **1938*** |
| 2023-01-15 |  83.5/ 99 (84.3%) | 2213      | 115.0/500 (23.0%) | 2105      | -         |
| 2023-03-31 | -                 | -         | 146.0/500 (29.2%) | 2161      | -         |


*: Dubious rating as print statements were leading to large time losses on buggy testing setup

G2315 is a 10-man gauntlet against the following opponents rated an average of 2315:
- (2374) Jazz 721
- (2361) Jackychess-0.13.4.jar
- (2346) Plisk-0.2.7d
- (2343) Chess4j-5.1-linux.jar
- (2324) Mediocre_v0.5.jar
- (2319) AICE Linux 0.99.2
- (2310) Sungorus64-1.4
- (2271) Paladin-0.1
- (2264) Phalanx-scid-3.61
- (2242) SpaceDog-0.97.7-Linux

G1920 is an 11-man gauntlet against the following opponents rated an average of 1920:
- (1995) Gully-2.16pl1
- (1965) Weasel-1.0.2-Beta
- (1965) Sissa-64-2.0
- (1962) ALChess_184_x64
- (1954) Tinman-v0.4.0
- (1927) Heracles 0.6.16
- (1912) Matmoi-7.15.0-cct
- (1876) Rustic-alpha-3.0.0-linux-64-bit-popcnt
- (1863) Deepov-0.4
- (1856) Fatalii-v0.3.1-x86_64-unknown-linux-musl
- (1842) Sayuri-2018.05.23-linux-64bit

CCRL rating estimated by assuming opponents in a group are rated close to the average:

    1 / (1 + 10^((opponentAverage - myRating)/400))  =  scorePercentage

Rearranges to:

    myRating  =  opponentAverage - 400 * math.log10(1/scorePercentage - 1)
