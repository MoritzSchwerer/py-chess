#include <vector>
#include <iostream>

#include "types.hpp"
#include "movegen.hpp"

void to_binary(Bitboard board) {
    for (int i = 63; i >= 0; i--) {
        bool is_set = (board & (1ull << i));
        std::cout << is_set;
    }
    std::cout << std::endl;
}

void print_move(Move move) {
    int source = move & 0x3f;
    int target = (move >> 6) & 0x3f;
    int flags  = (move >> 12) & 0xf;
    std::cout << "<Move src: " << source << ", tgt: " << target << ", flags: ";
    for (int i = 3; i >= 0; i--) {
        bool is_set = (move & (1ull << (12+i)));
        std::cout << is_set;
    }
    std::cout << ">" << std::endl;
}

void print_board(Bitboard board) {
    std::cout << "-------------------\n";
    for (int rank = 7; rank >= 0; --rank){
        for (int col = 0; col < 8; ++col) {
            if (col == 0) std::cout << "| ";
            bool is_set = board & (1ull << (rank*8+col));
            if (is_set) {
                std::cout << "x ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << "|\n";
    }
    std::cout << "-------------------" << std::endl;
}

int main() {
    GameState state;
    std::vector<Move> moves = get_legal_moves<true>(state);
    for (const Move& m : moves) {
        print_move(m);
    }
    std::cout << "Number of possible moves detected: " << moves.size() << std::endl;
    return 0;
}
