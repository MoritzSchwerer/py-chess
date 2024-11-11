#pragma once
#include <iostream>
#include <string>

#include "game_state.hpp"
#include "types.hpp"

// Function to convert a GameState to FEN piece placement
std::string bitboardToFEN(const GameState& state) {
    std::string fen;

    for (int rank = 7; rank >= 0; --rank) {
        int empty_count = 0;

        for (int file = 0; file < 8; ++file) {
            Bitboard mask = 1ULL << (rank * 8 + file);

            // Check each piece type for white
            if (state.w_pawn & mask) {
                if (empty_count > 0) {
                    fen += std::to_string(empty_count);
                    empty_count = 0;
                }
                fen += 'P';  // White pawn
            } else if (state.w_rook & mask) {
                if (empty_count > 0) {
                    fen += std::to_string(empty_count);
                    empty_count = 0;
                }
                fen += 'R';  // White rook
            } else if (state.w_knight & mask) {
                if (empty_count > 0) {
                    fen += std::to_string(empty_count);
                    empty_count = 0;
                }
                fen += 'N';  // White knight
            } else if (state.w_bishop & mask) {
                if (empty_count > 0) {
                    fen += std::to_string(empty_count);
                    empty_count = 0;
                }
                fen += 'B';  // White bishop
            } else if (state.w_queen & mask) {
                if (empty_count > 0) {
                    fen += std::to_string(empty_count);
                    empty_count = 0;
                }
                fen += 'Q';  // White queen
            } else if (state.w_king & mask) {
                if (empty_count > 0) {
                    fen += std::to_string(empty_count);
                    empty_count = 0;
                }
                fen += 'K';  // White king
            }
            // Check each piece type for black
            else if (state.b_pawn & mask) {
                if (empty_count > 0) {
                    fen += std::to_string(empty_count);
                    empty_count = 0;
                }
                fen += 'p';  // Black pawn
            } else if (state.b_rook & mask) {
                if (empty_count > 0) {
                    fen += std::to_string(empty_count);
                    empty_count = 0;
                }
                fen += 'r';  // Black rook
            } else if (state.b_knight & mask) {
                if (empty_count > 0) {
                    fen += std::to_string(empty_count);
                    empty_count = 0;
                }
                fen += 'n';  // Black knight
            } else if (state.b_bishop & mask) {
                if (empty_count > 0) {
                    fen += std::to_string(empty_count);
                    empty_count = 0;
                }
                fen += 'b';  // Black bishop
            } else if (state.b_queen & mask) {
                if (empty_count > 0) {
                    fen += std::to_string(empty_count);
                    empty_count = 0;
                }
                fen += 'q';  // Black queen
            } else if (state.b_king & mask) {
                if (empty_count > 0) {
                    fen += std::to_string(empty_count);
                    empty_count = 0;
                }
                fen += 'k';  // Black king
            } else {
                empty_count++;  // Increment empty square count
            }
        }

        // If there are empty squares at the end of the rank
        if (empty_count > 0) {
            fen += std::to_string(empty_count);
        }

        // Separate ranks with '/'
        if (rank > 0) {
            fen += '/';
        }
    }

    return fen;
}

// Function to generate the full FEN string from the GameState
std::string generateFEN(const GameState& state) {
    std::string fen;

    // 1. Piece Placement
    fen += bitboardToFEN(state);

    // 2. Active Color
    fen += state.status.isWhite ? " w " : " b ";

    // 3. Castling Availability
    std::string castling;
    if (state.status.wKingC) castling += 'K';
    if (state.status.wQueenC) castling += 'Q';
    if (state.status.bKingC) castling += 'k';
    if (state.status.bQueenC) castling += 'q';
    if (castling.empty()) castling = "-";
    fen += castling + " ";

    // 4. En Passant Target Square
    if (state.enpassant_board) {
        // Calculate the en passant square from the Bitboard (assumes single
        // square)
        int ep_square =
            __builtin_ctzll(state.enpassant_board);  // Get the index of the
                                                     // least significant bit
        char file = 'a' + (ep_square % 8);
        char rank = '1' + (ep_square / 8);
        fen += std::string(1, file) + std::string(1, rank) + " ";
    } else {
        fen += "- ";
    }

    // 5. Halfmove Clock
    fen += std::to_string(state.halfMoveClock) + " ";

    // 6. Fullmove Number
    fen += std::to_string(state.fullMoveCount);

    return fen;
}

int fenPosToIndex(const std::string& notation) {
    char letter = notation[0];
    int number = notation[1] - '0';  // convert char to int
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
    // int flags  = (move >> 12) & 0xf;
    std::cout << "<Move src: " << source << ", tgt: " << target << ", flags: ";
    for (int i = 3; i >= 0; i--) {
        bool is_set = (move & (1ull << (12 + i)));
        std::cout << is_set;
    }
    std::cout << ">" << std::endl;
}

void printPiece(Bitboard board) {
    std::cout << "-------------------\n";
    for (int rank = 7; rank >= 0; --rank) {
        for (int col = 0; col < 8; ++col) {
            if (col == 0) std::cout << "| ";
            bool is_set = board & (1ull << (rank * 8 + col));
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
                std::cout << "\033[31m";  // Red
            }
            if (state.w_pawn & mask)
                std::cout << "♙";
            else if (state.w_rook & mask)
                std::cout << "♖";
            else if (state.w_knight & mask)
                std::cout << "♘";
            else if (state.w_bishop & mask)
                std::cout << "♗";
            else if (state.w_queen & mask)
                std::cout << "♕";
            else if (state.w_king & mask)
                std::cout << "♔";
            else if (state.b_pawn & mask)
                std::cout << "♟";
            else if (state.b_rook & mask)
                std::cout << "♜";
            else if (state.b_knight & mask)
                std::cout << "♞";
            else if (state.b_bishop & mask)
                std::cout << "♝";
            else if (state.b_queen & mask)
                std::cout << "♛";
            else if (state.b_king & mask)
                std::cout << "♚";
            else
                std::cout << "-";
            if (highlight & mask) {
                std::cout << "\033[0m";  // Reset color
            } else {
                std::cout << "\033[37m\033[0m";  // White
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
