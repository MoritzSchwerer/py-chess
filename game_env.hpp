#pragma once

#include <iostream>
#include <string>
#include <array>
#include <exception>

#include "types.hpp"
#include "move_gen.hpp"
#include "utils.hpp"
#include "parse_san.hpp"

using Action = uint16_t;

constexpr int observationSpaceSize = 7104;
constexpr int actionSpaceSize = 4672; 
constexpr int numActionPlanes = 73;

struct ChessObservation {
    std::array<bool, observationSpaceSize> observation;
    std::array<bool, actionSpaceSize> actionMask;
    bool isTerminated;
    bool isTruncated;
};

class ChessGameEnv {
public:
    GameState state;

    ChessGameEnv() {}
    ChessGameEnv(const std::string &fen) : state(parseFen(fen)) {}

    Moves getPossibleMoves() const { return getLegalMoves(state); }

    void step(const Action action);
    ChessObservation observe();
};

constexpr std::array<int8_t, 73> planeToOffsetWhite = {
    /* queen moves */ 8, 16, 24, 32, 40, 48, 56, 9, 18, 27, 36, 45, 54, 63, 1, 2, 3, 4, 5, 6, 7, -7, -14, -21, -28, -35, -42, -49, -8, -16, -24, -32, -40, -48, -56, -9, -18, -27, -36, -45, -54, -63, -1, -2, -3, -4, -5, -6, -7, 7, 14, 21, 28, 35, 42, 49,
    /* knight moves */ 15, 17, 10, -6, -15, -17, 6, -10,
    /* pawn moves (L, P, R) (N, B, R)*/ 7, 7, 7, 8, 8, 8, 9, 9, 9
};
constexpr std::array<int8_t, 73> planeToOffsetBlack = {
    /* queen moves */ 8, 16, 24, 32, 40, 48, 56, 9, 18, 27, 36, 45, 54, 63, 1, 2, 3, 4, 5, 6, 7, -7, -14, -21, -28, -35, -42, -49, -8, -16, -24, -32, -40, -48, -56, -9, -18, -27, -36, -45, -54, -63, -1, -2, -3, -4, -5, -6, -7, 7, 14, 21, 28, 35, 42, 49,
    /* knight moves */ 15, 17, 10, -6, -15, -17, 6, -10,
    /* pawn moves (L, P, R) (N, B, R)*/ -9, -9, -9, -8, -8, -8, -7, -7, -7
};

template<bool isWhite>
PieceType getPieceType(const GameState& state, uint8_t square) {
    const Bitboard board = 1ull << square;
    if constexpr (isWhite) {
        if (state.w_pawn & board) return PieceType::Pawn;
        if (state.w_knight & board) return PieceType::Knight;
        if (state.w_bishop & board) return PieceType::Bishop;
        if (state.w_rook & board) return PieceType::Rook;
        if (state.w_queen & board) return PieceType::Queen;
        if (state.w_king & board) return PieceType::King;
    } else {
        if (state.b_pawn & board) return PieceType::Pawn;
        if (state.b_knight & board) return PieceType::Knight;
        if (state.b_bishop & board) return PieceType::Bishop;
        if (state.b_rook & board) return PieceType::Rook;
        if (state.b_queen & board) return PieceType::Queen;
        if (state.b_king & board) return PieceType::King;
    }
    return PieceType::None;
}

// this will throw if we can't find a match
template<bool isWhite>
Bitboard& getBitboardFromSquare(GameState& state, Bitboard board) {
    if constexpr (isWhite) {
        if (state.w_pawn & board) return state.w_pawn;
        if (state.w_knight & board) return state.w_knight;
        if (state.w_bishop & board) return state.w_bishop;
        if (state.w_rook & board) return state.w_rook;
        if (state.w_queen & board) return state.w_queen;
        if (state.w_king & board) return state.w_king;
    } else {
        if (state.b_pawn & board) return state.b_pawn;
        if (state.b_knight & board) return state.b_knight;
        if (state.b_bishop & board) return state.b_bishop;
        if (state.b_rook & board) return state.b_rook;
        if (state.b_queen & board) return state.b_queen;
        if (state.b_king & board) return state.b_king;
    }
    throw std::runtime_error("Error: getBitboardFromSquare couldn't find a match for the square.");
}

template<bool isWhite>
Bitboard& getBitboardFromPieceType(GameState& state, PieceType type) {
    if constexpr (isWhite) {
        if (type == PieceType::Pawn) return state.w_pawn;
        if (type == PieceType::Knight) return state.w_knight;
        if (type == PieceType::Bishop) return state.w_bishop;
        if (type == PieceType::Rook) return state.w_rook;
        if (type == PieceType::Queen) return state.w_queen;
        if (type == PieceType::King) return state.w_king;
    } else {
        if (type == PieceType::Pawn) return state.b_pawn;
        if (type == PieceType::Knight) return state.b_knight;
        if (type == PieceType::Bishop) return state.b_bishop;
        if (type == PieceType::Rook) return state.b_rook;
        if (type == PieceType::Queen) return state.b_queen;
        if (type == PieceType::King) return state.b_king;
    }
    throw std::runtime_error("Error: getBitboardFromSquare couldn't find a match for the square.");
}

// returns if we actually removed a piece
template<bool isWhite>
bool removeEnemyPiece(GameState& state, Bitboard targetBoard) {
    if constexpr (isWhite) {
        state.b_pawn &= ~targetBoard;
        state.b_knight &= ~targetBoard;
        state.b_bishop &= ~targetBoard;
        state.b_rook &= ~targetBoard;
        state.b_queen &= ~targetBoard;
        state.b_king &= ~targetBoard;
        return state.b_pawn & targetBoard | state.b_knight & targetBoard | state.b_bishop & targetBoard | state.b_rook & targetBoard | state.b_queen & targetBoard | state.b_king & targetBoard;
    } else {
        state.w_pawn &= ~targetBoard;
        state.w_knight &= ~targetBoard;
        state.w_bishop &= ~targetBoard;
        state.w_rook &= ~targetBoard;
        state.w_queen &= ~targetBoard;
        state.w_king &= ~targetBoard;
        return state.w_pawn & targetBoard | state.w_knight & targetBoard | state.w_bishop & targetBoard | state.w_rook & targetBoard | state.w_queen & targetBoard | state.w_king & targetBoard;
    }
}

template<bool isWhite>
inline bool isCastle(uint8_t ss, uint8_t ts) {
    if constexpr (isWhite) {
        return ss == 4 && (ts == 0 || ts == 7);
    } else {
        return ss == 60 && (ts == 56 || ts == 63);
    }
}

template<bool isWhite>
void handleCastling(GameState& state, uint8_t sourceSquare, uint8_t targetSquare) {
    // handle casteling
    if constexpr (isWhite) {
        // left castle
        if (sourceSquare > targetSquare) {
            state.w_king =   0b00000100ull;
            state.w_rook &= ~0b00000001ull;
            state.w_rook |=  0b00001000ull;
        } else {
            state.w_king =   0b01000000ull;
            state.w_rook &= ~0b10000000ull;
            state.w_rook |=  0b00100000ull;
        }
    } else {
        if (sourceSquare > targetSquare) {
            state.w_king =   0b00000100ull << 56;
            state.w_rook &= ~0b00000001ull << 56;
            state.w_rook |=  0b00001000ull << 56;
        } else {
            state.w_king =   0b01000000ull << 56;
            state.w_rook &= ~0b10000000ull << 56;
            state.w_rook |=  0b00100000ull << 56;
        }
    }
    state.status.removeCastlingRights<isWhite>();
}

template<bool isWhite>
void updateCastlingRights(GameState& state, Bitboard sourceBoard, PieceType type) {
    // update castling rights king moves
    if (type == PieceType::King) {
        state.status.removeCastlingRights<isWhite>();
    }

    // update castling right rook moves
    if (type == PieceType::Rook) {
        if (sourceBoard & initialRookLeft<isWhite>() ) state.status.removeCastlingRightsLeft<isWhite>();
        if (sourceBoard & initialRookRight<isWhite>()) state.status.removeCastlingRightsRight<isWhite>();
    }
}

template<bool isWhite>
void handlePromotion(GameState& state, Bitboard& pieceBoard, Bitboard targetBoard, PieceType promotion, PieceType type) {
    if (type == PieceType::Pawn && targetBoard & lastRank<isWhite>()) {
        Bitboard& promotionBoard = getBitboardFromPieceType<isWhite>(state, promotion);
        promotionBoard |= targetBoard;
    } else {
        pieceBoard |= targetBoard;
    }
}

template<bool isWhite>
void handleEnpassantCapture(GameState& state, Bitboard targetBoard) {
    removeEnemyPiece<isWhite>(state, pawnPush1<!isWhite>(targetBoard));
}

template<bool isWhite>
bool updateGameState(GameState& state, uint8_t sourceSquare, uint8_t targetSquare, PieceType promotion) {
    const GameStatus& status = state.status;
    const Bitboard sourceBoard = 1ull << sourceSquare;
    const Bitboard targetBoard = 1ull << targetSquare;

    const PieceType type = getPieceType<isWhite>(state, sourceSquare);
    const bool isEnpassantPossible = state.status.enpassant;
    state.clearEnpassant();

    if (isCastle<isWhite>(sourceSquare, targetSquare)) {
        handleCastling<isWhite>(state, sourceSquare, targetSquare);
        return false;
    }

    updateCastlingRights<isWhite>(state, sourceBoard, type);

    // remove piece from source position
    Bitboard& pieceBoard = getBitboardFromSquare<isWhite>(state, sourceBoard);
    pieceBoard &= ~sourceBoard;

    // handle promotion
    handlePromotion<isWhite>(state, pieceBoard, targetBoard, promotion, type);

    // handle enpassant take
    if (type == PieceType::Pawn && isEnpassantPossible && targetBoard & state.enpassant_board) {
        handleEnpassantCapture<isWhite>(state, targetBoard);
        return true;
    }

    // handle enable enpassant
    if (enablesEnpassant<isWhite>(state, sourceBoard, targetBoard, type)) {
        state.setEnpassant(pawnPush1<isWhite>(sourceBoard));
        return true;
    }


    const bool tookEnemyPiece = removeEnemyPiece<isWhite>(state, targetBoard);
    return tookEnemyPiece || type == PieceType::Pawn;
}

PieceType getPromotion(uint8_t plane) {
    if (plane < 64) return PieceType::Queen;
    switch ((plane - 64) % 3) {
        case 0: return PieceType::Knight;
        case 1: return PieceType::Bishop;
        case 2: return PieceType::Rook;
        default: return PieceType::None;
    }
}

template<bool isWhite>
bool enablesEnpassant(GameState& state, Bitboard sourceBoard, Bitboard targetBoard, PieceType type) {
    const Bitboard enemyPawns = getEnemyPawns<isWhite>(state);
    return (
        type == PieceType::Pawn &&
        targetBoard & pawnPush2<isWhite>(sourceBoard) &&
        (pawnAttackLeft<isWhite>(pawnPush1<isWhite>(sourceBoard)) & enemyPawns ||
        pawnAttackRight<isWhite>(pawnPush1<isWhite>(sourceBoard)) & enemyPawns)
    );
}

template<bool isWhite>
void updateMoveCount(GameState& state, bool reset) {
    if (reset) state.halfMoveClock = 0;
    else state.halfMoveClock++;

    if constexpr (!isWhite) state.fullMoveCount++;
}

template<bool isWhite>
uint8_t getOffsetFromPlane(uint8_t plane) {
    if constexpr (isWhite) return planeToOffsetWhite[plane];
    else return planeToOffsetWhite[plane];
}

template<bool isWhite>
void makeMove(GameState& state, Action action) {
    const uint8_t sourceSquare = action / numActionPlanes;
    const uint8_t plane = action % numActionPlanes;
    const int8_t offset = getOffsetFromPlane<isWhite>(plane);
    const uint8_t targetSquare = sourceSquare + offset;

    const PieceType promotion = getPromotion(plane);

    bool resetHalfMoveClock = updateGameState<isWhite>(state, sourceSquare, targetSquare, promotion);

    updateMoveCount<isWhite>(state, resetHalfMoveClock);

    state.status.silentMove();
}

// this will perform a move in the game
// this is a no legality check version
void ChessGameEnv::step(Action action) {
    if (state.status.isWhite) makeMove<true>(state, action);
    else makeMove<false>(state, action);
}

ChessObservation ChessGameEnv::observe() {
    ChessObservation obs;
    return obs;
}


