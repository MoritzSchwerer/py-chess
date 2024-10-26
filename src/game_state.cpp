#include <iostream>

#include "game_state.hpp"
#include "zobrist.hpp"

PastGameState::PastGameState(const GameState &state)
    : w_pawn(state.w_pawn),
      w_rook(state.w_rook),
      w_knight(state.w_knight),
      w_bishop(state.w_bishop),
      w_queen(state.w_queen),
      w_king(state.w_king),
      b_pawn(state.b_pawn),
      b_rook(state.b_rook),
      b_knight(state.b_knight),
      b_bishop(state.b_bishop),
      b_queen(state.b_queen),
      b_king(state.b_king),
      enpassant_board(state.enpassant_board) {
    if (state.status.isWhite) {
        positionHash = Zobrist::hashBoard<true>(state);
    } else {
        positionHash = Zobrist::hashBoard<false>(state);
    }
}

void GameState::addHistory(const PastGameState &pastState) {
    for (int i = 6; i > 0; i--) {
        stateHistory[i] = stateHistory[i - 1];
    }
    stateHistory[0] = pastState;

    const uint64_t posHash = pastState.positionHash;
    int currentValue = this->positionHashes[posHash];
    this->positionHashes[posHash] = currentValue + 1;
}

void GameState::setEnpassant(Bitboard enpassantBoard) {
    enpassant_board = enpassantBoard;
    status.enpassant = true;
}

void GameState::clearEnpassant() {
    enpassant_board = 0ull;
    status.enpassant = false;
}
uint64_t GameState::getPositionHash() const {
    if (status.isWhite) {
        return Zobrist::hashBoard<true>(*this);
    } else {
        return Zobrist::hashBoard<false>(*this);
    }
}

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

    gameState.halfMoveClock = 0ul;
    gameState.fullMoveCount = 1ul;

    gameState.stateHistory = {};

    GameStatus status;
    status.isWhite = true;
    status.wKingC = false;
    status.wQueenC = false;
    status.bKingC = false;
    status.bQueenC = false;
    status.enpassant = false;

    gameState.status = status;
    return gameState;
}

GameState parseFen(const std::string &fen) {
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
            Bitboard *bitboard = nullptr;
            switch (c) {
                case 'r':
                    bitboard = &state.b_rook;
                    break;
                case 'n':
                    bitboard = &state.b_knight;
                    break;
                case 'b':
                    bitboard = &state.b_bishop;
                    break;
                case 'q':
                    bitboard = &state.b_queen;
                    break;
                case 'k':
                    bitboard = &state.b_king;
                    break;
                case 'p':
                    bitboard = &state.b_pawn;
                    break;
                case 'R':
                    bitboard = &state.w_rook;
                    break;
                case 'N':
                    bitboard = &state.w_knight;
                    break;
                case 'B':
                    bitboard = &state.w_bishop;
                    break;
                case 'Q':
                    bitboard = &state.w_queen;
                    break;
                case 'K':
                    bitboard = &state.w_king;
                    break;
                case 'P':
                    bitboard = &state.w_pawn;
                    break;
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
    status.wKingC = castling.find('K') != std::string::npos;
    status.wQueenC = castling.find('Q') != std::string::npos;
    status.bKingC = castling.find('k') != std::string::npos;
    status.bQueenC = castling.find('q') != std::string::npos;

    // Parse en passant target square
    std::string enPassant = tokens[3];
    if (enPassant != "-") {
        int file = enPassant[0] - 'a';
        int rank = enPassant[1] - '1';
        status.enpassant = true;
        state.enpassant_board = 1ULL << (rank * 8 + file);
    }
    state.status = status;

    std::string halfMoveClockStr = tokens[4];
    state.halfMoveClock = std::stoi(halfMoveClockStr);
    std::string fullMoveCountStr = tokens[5];
    state.fullMoveCount = std::stoi(fullMoveCountStr);

    return state;
}
