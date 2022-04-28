# jangine -- Jan's Basic C++ chess engine

Chess engine I wrote in 2015 that can beat a competent beginner.
- Algorithms used:
    - Search: alpha-beta search, quiescence search, killer heuristic
    - Evaluation: turochamp
- Strength should be around **1300 Lichess**. It's good at taking free material and giving checkmate, but bad at avoiding pins and endgames.
- This was my first C++ project which is why code quality is suboptimal and C-like.
- No adjustment of search depth to time controls. 30+ minute matches recommended.

Challenge me on lichess! https://lichess.org/@/jangine

![](./jangine_lichess_scr.png)


## Usage

Build

    g++ jangine.cpp -o jangine

Visual tests such as mate-in-n-moves tasks

    ./jangine -t

Run as UCI engine

    ./jangine

    ./jangine | tee -a jangine.log  # logging to file

To run with lichess, clone this project https://github.com/ShailChoksi/lichess-bot and place the engine binary in the "engine" folder.
