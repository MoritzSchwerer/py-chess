### Py-Chess

This is a well performing C++ chess environment for RL. It should work almost identical to [Pettingzoo's Chess Env](https://pettingzoo.farama.org/environments/classic/chess/). It is still missing draw by insufficient material. But other than that it is complete.

It exists because I needed a fast ChessEnv that can be accessed from python for my implementation of [AlphaZero](https://github.com/MoritzSchwerer/Alpha_Zero). 
I plan to improve upon the library in the future but it may never come to that.

##### Learning about Py-Chess

If you are here to learn how Py-Chess works then I got good news for you.
The move generation part of the engine is very clean and you can read it in `src/move_gen.hpp`.
This part was heavily inspired by [Gigantua](https://github.com/Gigantua/Gigantua) and the
[article](https://www.codeproject.com/Articles/5313417/Worlds-Fastest-Bitboard-Chess-Movegenerator).


Detecting 3-fold repetition is also interesting. Here I just use zobrist keys. They are just
a way to hash a given board into 64 bits. For each board this key will be generated and stored
in a map from hash to count. After each move we simply check if the current hash
occured 3 times and if so we draw the game. The code for this part can be found in 
`src/zobrist.hpp`. An explanation of this can be found [here](https://www.chessprogramming.org/Zobrist_Hashing).


If you are wondering about the hashing part. I use a mixer which I got from this [blog](https://jonkagstrom.com/bit-mixer-construction/).
A mixer takes a non-zero number (seed) and then does some computation on that. Usually multiplications and XORs with a shifted version of itself.
This results in very fast hash function that has low enough collision rate for our usecase.


As for the rest there is not much interesting stuff going on you use the move generation to validated
incoming moves and to tell the agent which moves are legal and when you receive a move
you just move around some state.
