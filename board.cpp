#include <iostream>

typedef uint64_t Bitboard;

struct GameState {
    Bitboard w_pawn   = 0x000000000000FF00;
    Bitboard w_rook   = 0x0000000000000081;
    Bitboard w_knight = 0x0000000000000042;
    Bitboard w_bishop = 0x0000000000000024;
    Bitboard w_queen  = 0x0000000000000008;
    Bitboard w_king   = 0x0000000000000010;

    Bitboard b_pawn   = 0x00FF000000000000;
    Bitboard b_rook   = 0x8100000000000000;
    Bitboard b_knight = 0x4200000000000000;
    Bitboard b_bishop = 0x2400000000000000;
    Bitboard b_queen  = 0x0800000000000000;
    Bitboard b_king   = 0x1000000000000000;

    bool w_king_castle = true;
    bool w_queen_castle = true;
    bool b_king_castle = true;
    bool b_queen_castle = true;
};

void to_binary(Bitboard board) {
    for (uint64_t i = 0; i < 64; ++i) {
        bool is_set = (board & (1ull << i));
        std::cout << is_set;
    }
    std::cout << std::endl;
}

Bitboard pawn_moves(Bitboard white, Bitboard black) {
    return (white << 8) & ~black;
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
                std::cout << "  ";
            }
        }
        std::cout << "|\n";
    }
    std::cout << "-------------------" << std::endl;
}

int main() {
    GameState state;
    Bitboard white = state.w_pawn | state.w_rook | state.w_knight | state.w_bishop | state.w_queen | state.w_king;
    Bitboard black = state.b_pawn | state.b_rook | state.b_knight | state.b_bishop | state.b_queen | state.b_king;
    Bitboard enemy = 0x0000000000010000;
    print_board(state.w_pawn);
    print_board(pawn_moves(state.w_pawn, enemy));
}

