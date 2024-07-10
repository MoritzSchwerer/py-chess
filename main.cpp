#include <vector>
#include <iostream>
#include <bitset>

#include "types.hpp"
#include "movegen.hpp"


int main() {
    GameState state;
    // std::vector<Move> moves = get_legal_moves<true>(state);
    // for (const Move& m : moves) {
    //     print_move(m);
    // }
    // std::cout << "Number of possible moves detected: " << moves.size() << std::endl;
    state.w_pawn = 0b00010000000000001110111100000000;
    Bitboard seenSquares =  getSeenSquares<true>(state);
    printBoard(state, seenSquares);
    return 0;
}
