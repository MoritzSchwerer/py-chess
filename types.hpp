#pragma once

#include <vector>

typedef uint64_t Bitboard;
typedef uint16_t Move;
typedef std::vector<Move> Moves;

struct GameStatus {
    bool isWhite = true;
    bool w_king_castle = true;
    bool w_queen_castle = true;
    bool b_king_castle = true;
    bool b_queen_castle = true;
    bool enpassant = true;
};

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

    Bitboard enpassant_board = 0ull;

    GameStatus status;
};

GameState GameStateEmpty() {
    GameState gameState;
    gameState.w_pawn = 0ull;
    gameState.w_rook = 0ull;
    gameState.w_knight = 0ull;
    gameState.w_bishop = 0ull;
    gameState.w_queen = 0ull;
    gameState.w_king = 0ull;

    gameState.b_pawn = 0ull;
    gameState.b_rook = 0ull;
    gameState.b_knight = 0ull;
    gameState.b_bishop = 0ull;
    gameState.b_queen = 0ull;
    gameState.b_king = 0ull;

    gameState.enpassant_board = 0ull;

    GameStatus status;
    status.isWhite = true;
    status.w_king_castle = false;
    status.w_queen_castle = false;
    status.b_king_castle = false;
    status.b_queen_castle = false;
    status.enpassant = false;

    gameState.status = status;
    return gameState;
}


GameState parseFen(const std::string& fen) {
    std::string tokens[6];
    int tokenCount = 0;
    std::string token;
    for (char c : fen) {
        if (c == ' ') {
            tokens[tokenCount++] = token;
            token.clear();
        } else {
            token += c;
        }
    }
    tokens[tokenCount++] = token;

    // Parse piece placement
    std::string piecePlacement = tokens[0];
    int rank = 7;
    int file = 0;
    GameState state = GameStateEmpty();
    for (char c : piecePlacement) {
        if (c == '/') {
            rank--;
            file = 0;
        } else if (c >= '1' && c <= '8') {
            file += c - '0';
        } else {
            Bitboard* bitboard = nullptr;
            switch (c) {
                case 'r': bitboard = &state.b_rook; break;
                case 'n': bitboard = &state.b_knight; break;
                case 'b': bitboard = &state.b_bishop; break;
                case 'q': bitboard = &state.b_queen; break;
                case 'k': bitboard = &state.b_king; break;
                case 'p': bitboard = &state.b_pawn; break;
                case 'R': bitboard = &state.w_rook; break;
                case 'N': bitboard = &state.w_knight; break;
                case 'B': bitboard = &state.w_bishop; break;
                case 'Q': bitboard = &state.w_queen; break;
                case 'K': bitboard = &state.w_king; break;
                case 'P': bitboard = &state.w_pawn; break;
            }
            if (bitboard) {
                *bitboard |= 1ULL << (rank * 8 + file);
                file++;
            }
        }
    }
    GameStatus status;
    status.isWhite = tokens[1] == "w";

    // Parse castling availability
    std::string castling = tokens[2];
    status.w_king_castle = castling.find('K') != std::string::npos;
    status.w_queen_castle = castling.find('Q') != std::string::npos;
    status.b_king_castle = castling.find('k') != std::string::npos;
    status.b_queen_castle = castling.find('q') != std::string::npos;

    // Parse en passant target square
    std::string enPassant = tokens[3];
    if (enPassant != "-") {
        int file = enPassant[0] - 'a';
        int rank = enPassant[1] - '1';
        status.enpassant = true;
        state.enpassant_board = 1ULL << (rank * 8 + file);
    }
    state.status = status;

    return state;
}
