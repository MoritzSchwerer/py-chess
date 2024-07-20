#pragma once

#include <vector>
#include <deque>

typedef uint64_t Bitboard;
typedef uint16_t Move;
typedef std::vector<Move> Moves;

enum class PieceType : uint8_t {
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
    None,
};

class GameStatus {
    public:
    bool isWhite;
    bool wKingC;
    bool wQueenC;
    bool bKingC;
    bool bQueenC;
    bool enpassant;

    constexpr GameStatus() : isWhite(true), wKingC(true), wQueenC(true), bKingC(true), bQueenC(true), enpassant(false) {}

    constexpr GameStatus(bool isWhite, bool wKingC, bool wQueenC, bool bKingC, bool bQueenC, bool enpassant)
        : isWhite(isWhite), wKingC(wKingC), wQueenC(wQueenC), bKingC(bKingC), bQueenC(bQueenC), enpassant(enpassant) {}

    constexpr GameStatus(uint8_t pattern) {
        isWhite   = (pattern & 0b000001) != 0;
        wKingC    = (pattern & 0b000010) != 0;
        wQueenC   = (pattern & 0b000100) != 0;
        bKingC    = (pattern & 0b001000) != 0;
        bQueenC   = (pattern & 0b010000) != 0;
        enpassant = (pattern & 0b100000) != 0;
    }

    constexpr void silentMove() {
        isWhite = !isWhite;
    }

    constexpr void wCastle() {
        wKingC = false;
        wQueenC = false;
    }

    constexpr void bCastle() {
        bKingC = false;
        bQueenC = false;
    }

    constexpr void wMoveRookLeft() {
        wQueenC = false;
    }

    constexpr void wMoveRookRight() {
        wKingC = false;
    }

    constexpr void bMoveRookLeft() {
        bQueenC = false;
    }

    constexpr void bMoveRookRight() {
        bKingC = false;
    }

    constexpr void enableEnpassant() {
        enpassant = true;
    }

    constexpr void disableEnpassant() {
        enpassant = false;
    }

    template<bool isWhite>
    constexpr void removeCastlingRights() {
        if constexpr (isWhite) {
            wKingC = false;
            wQueenC = false;
        } else {
            bKingC = false;
            bQueenC = false;
        }
        
    }

    template<bool isWhite>
    constexpr void removeCastlingRightsLeft() {
        if constexpr (isWhite) wQueenC = false;
        else bQueenC = false;
    }

    template<bool isWhite>
    constexpr void removeCastlingRightsRight() {
        if constexpr (isWhite) wKingC = false;
        else bKingC = false;
    }

};

inline uint8_t getStatusPattern(const GameStatus status) {
    return status.isWhite | status.wKingC << 1 | status.wQueenC << 2 | status.bKingC << 3 | status.bQueenC << 4 | status.enpassant << 5;
}

// this is here to store past game states without the need
// for halfMoveClock fullMoveCount and status
struct PastGameState {
    Bitboard w_pawn;
    Bitboard w_rook;
    Bitboard w_knight;
    Bitboard w_bishop;
    Bitboard w_queen;
    Bitboard w_king;

    Bitboard b_pawn;
    Bitboard b_rook;
    Bitboard b_knight;
    Bitboard b_bishop;
    Bitboard b_queen;
    Bitboard b_king;

    Bitboard enpassant_board;

    PastGameState(const GameState& state) :
        w_pawn(state.w_pawn),
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
        enpassant_board(state.enpassant_board) {}
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

    uint32_t halfMoveClock = 0ul;
    uint32_t fullMoveCount = 1ul;

    std::array<PastGameState, 7> history;

    GameStatus status;
};

void addGameStateHistory(GameState& state, const PastGameState& pastState) {
    for (int i = 6; i > 0; i--) {
        state.history[i] = state.history[i - 1];
    }
    state.history[0] = pastState;
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

void setEnpassant(GameState& state, Bitboard enpassantBoard) {
    state.enpassant_board = enpassantBoard;
    state.status.enpassant = true;
}

void clearEnpassant(GameState& state) {
    state.enpassant_board = 0ull;
    state.status.enpassant = false;
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
