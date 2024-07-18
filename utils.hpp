#pragma once
#include "types.hpp"

int fenPosToIndex(const std::string& notation) {
  char letter = notation[0];
  int number = notation[1] - '0'; // convert char to int
  int index = (number - 1) * 8 + (letter - 'a');
  return index;
}

std::string squareToFenPos(int square) {
    static const char files[] = "abcdefgh";
    int file = square % 8;
    int rank = square / 8;
    return std::string(1, files[file]) + std::to_string(rank + 1);
}

void printBinary(Bitboard board) {
    for (int i = 63; i >= 0; i--) {
        bool is_set = (board & (1ull << i));
        std::cout << is_set;
    }
    std::cout << std::endl;
}

void printMove(Move move) {
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

void printPiece(Bitboard board) {
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

// thanks to https://labs.perplexity.ai/ for this nice printing
void printBoard(const GameState& state, const Bitboard& highlight) {
    std::cout << "\033[90m";
    for (int i = 0; i < 19; ++i) {
        std::cout << '-';
    }
    std::cout << "\n";

    for (int i = 7; i >= 0; --i) {
        std::cout << "\033[90m|\033[0m ";
        for (int j = 0; j < 8; ++j) {
            Bitboard mask = 1ULL << (i * 8 + j);
            if (highlight & mask) {
                std::cout << "\033[31m"; // Red
            }
            if (state.w_pawn & mask) std::cout << "P";
            else if (state.w_rook & mask) std::cout << "R";
            else if (state.w_knight & mask) std::cout << "N";
            else if (state.w_bishop & mask) std::cout << "B";
            else if (state.w_queen & mask) std::cout << "Q";
            else if (state.w_king & mask) std::cout << "K";
            else if (state.b_pawn & mask) std::cout << "p";
            else if (state.b_rook & mask) std::cout << "r";
            else if (state.b_knight & mask) std::cout << "n";
            else if (state.b_bishop & mask) std::cout << "b";
            else if (state.b_queen & mask) std::cout << "q";
            else if (state.b_king & mask) std::cout << "k";
            else std::cout << "-";
            if (highlight & mask) {
                std::cout << "\033[0m"; // Reset color
            } else {
                std::cout << "\033[37m\033[0m"; // White
            }
            std::cout << ' ';
        }
        std::cout << "\033[90m|\033[0m\n";
    }

    std::cout << "\033[90m";
    for (int i = 0; i < 19; ++i) {
        std::cout << '-';
    }
    std::cout << "\n\033[0m";
}
