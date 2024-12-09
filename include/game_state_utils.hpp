#pragma once

#include <exception>
#include <game_state.hpp>

#include "game_rules.hpp"
#include "lookup.hpp"
#include "move_gen.hpp"
#include "observation.hpp"

constexpr int OBSERVATION_SPACE_SIZE = 7104;
constexpr int ACTION_SPACE_SIZE = 4672;
constexpr int NUM_ACTION_PLANES = 73;
constexpr int PLANE_SIZE = 64;
constexpr int MAX_GAME_LENGTH = 250;  // this means that there is 500 half moves

struct TerminationInfo {
    int32_t whiteReward;
    int32_t blackReward;
    bool isTerminated;

    TerminationInfo(int32_t whiteReward, int32_t blackReward, bool isTerminated)
        : whiteReward(whiteReward),
          blackReward(blackReward),
          isTerminated(isTerminated) {}
};

template <bool isWhite>
inline PieceType getPieceType(const GameState& state, uint8_t square) {
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

template <bool isWhite>
inline Bitboard& getBitboardFromSquare(GameState& state, Bitboard board) {
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
    throw std::runtime_error(
        "Error: getBitboardFromSquare couldn't find a match for the square.");
}

template <bool isWhite>
inline Bitboard& getBitboardFromPieceType(GameState& state, PieceType type) {
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
    throw std::runtime_error(
        "Error: getBitboardFromSquare couldn't find a match for the square.");
}

template <bool isWhite>
inline void removeEnemyPiece(GameState& state, Bitboard targetBoard) {
    if constexpr (isWhite) {
        state.b_pawn &= ~targetBoard;
        state.b_knight &= ~targetBoard;
        state.b_bishop &= ~targetBoard;
        state.b_rook &= ~targetBoard;
        state.b_queen &= ~targetBoard;
        state.b_king &= ~targetBoard;
    } else {
        state.w_pawn &= ~targetBoard;
        state.w_knight &= ~targetBoard;
        state.w_bishop &= ~targetBoard;
        state.w_rook &= ~targetBoard;
        state.w_queen &= ~targetBoard;
        state.w_king &= ~targetBoard;
    }
}

template <bool isWhite>
inline bool isCastle(uint8_t ss, uint8_t ts) {
    if constexpr (isWhite) {
        return ss == 4 && (ts == 2 || ts == 6);
    } else {
        return ss == 60 && (ts == 58 || ts == 62);
    }
}

template <bool isWhite>
inline void handleCastling(GameState& state, uint8_t sourceSquare,
                           uint8_t targetSquare) {
    // handle casteling
    if constexpr (isWhite) {
        // left castle
        if (sourceSquare > targetSquare) {
            state.w_king = 0b00000100ull;
            state.w_rook &= ~0b00000001ull;
            state.w_rook |= 0b00001000ull;
        } else {
            state.w_king = 0b01000000ull;
            state.w_rook &= ~0b10000000ull;
            state.w_rook |= 0b00100000ull;
        }
    } else {
        if (sourceSquare > targetSquare) {
            state.b_king = 0b00000100ull << 56;
            state.b_rook &= ~(0b00000001ull << 56);
            state.b_rook |= 0b00001000ull << 56;
        } else {
            state.b_king = 0b01000000ull << 56;
            state.b_rook &= ~(0b10000000ull << 56);
            state.b_rook |= 0b00100000ull << 56;
        }
    }
}

template <bool isWhite>
inline void updateCastlingRights(GameState& state, Bitboard sourceBoard,
                                 Bitboard targetBoard, PieceType type) {
    // update castling rights king moves
    if (type == PieceType::King) {
        state.status.removeCastlingRights<isWhite>();
    }

    // update castling right rook moves
    if (type == PieceType::Rook) {
        if (sourceBoard & initialRookLeft<isWhite>()) {
            state.status.removeCastlingRightsLeft<isWhite>();
        }
        if (sourceBoard & initialRookRight<isWhite>()) {
            state.status.removeCastlingRightsRight<isWhite>();
        }
    }
    // update casteling if rook is taken
    if (targetBoard & initialRookLeft<!isWhite>()) {
        state.status.removeCastlingRightsLeft<!isWhite>();
    }
    if (targetBoard & initialRookRight<!isWhite>()) {
        state.status.removeCastlingRightsRight<!isWhite>();
    }
}

template <bool isWhite>
inline void moveToTargetPosition(GameState& state, Bitboard& pieceBoard,
                                 Bitboard targetBoard, PieceType promotion,
                                 PieceType type) {
    if (type == PieceType::Pawn && targetBoard & lastRank<isWhite>()) {
        Bitboard& promotionBoard =
            getBitboardFromPieceType<isWhite>(state, promotion);
        promotionBoard |= targetBoard;
    } else {
        pieceBoard |= targetBoard;
    }
}

template <bool isWhite>
inline void handleEnpassantCapture(GameState& state, Bitboard targetBoard) {
    removeEnemyPiece<isWhite>(state, pawnPush1<!isWhite>(targetBoard));
}

template <bool isWhite>
inline uint64_t convertToColorSquare(uint64_t square) {
    if constexpr (isWhite) {
        return square;
    } else {
        const uint64_t rank = square / 8;
        const uint64_t file = square % 8;
        const int64_t negRank = rank - 7;
        return -negRank * 8 + file;
    }
}

template <bool isWhite>
inline int64_t getOffset(uint64_t source, uint64_t target) {
    if constexpr (isWhite) {
        return target - source;
    } else {
        const int64_t whiteOffset = target - source;
        const int64_t vOffset = whiteOffset / 8;
        const int64_t hOffset = whiteOffset - (8 * vOffset);
        return -vOffset * 8 + hOffset;
    }
}

template <bool isWhite>
inline ActionInfo parseAction(Action action) {
    const uint16_t sourceFile = action / (NUM_ACTION_PLANES * 8);
    const uint16_t sourceRank = (action / NUM_ACTION_PLANES) % 8;
    const uint16_t tempSourceSquare = sourceRank * 8 + sourceFile;
    const uint16_t plane = action % NUM_ACTION_PLANES;
    const int16_t offset = Lookup::getOffsetFromPlane<isWhite>(plane);

    const uint16_t targetSquare =
        convertToColorSquare<isWhite>(tempSourceSquare + offset);
    const uint16_t sourceSquare =
        convertToColorSquare<isWhite>(tempSourceSquare);

    const PieceType promotion = Lookup::getPromotion(plane);
    return ActionInfo{
        sourceSquare,
        targetSquare,
        promotion,
    };
}

template <bool isWhite>
inline bool enablesEnpassant(GameState& state, Bitboard sourceBoard,
                             Bitboard targetBoard, PieceType type) {
    const Bitboard enemyPawns = getEnemyPawns<isWhite>(state);
    return (type == PieceType::Pawn &&
            targetBoard & pawnPush2<isWhite>(sourceBoard) &&
            (pawnAttackLeft<isWhite>(pawnPush1<isWhite>(sourceBoard)) &
                 enemyPawns ||
             pawnAttackRight<isWhite>(pawnPush1<isWhite>(sourceBoard)) &
                 enemyPawns));
}

template <bool isWhite>
inline void updateMoveCount(GameState& state, bool isPawnMove, bool isCapture) {
    if (isPawnMove || isCapture) {
        state.halfMoveClock = 0;
    } else {
        state.halfMoveClock++;
    }

    if constexpr (!isWhite) state.fullMoveCount++;
}

inline void fillObservationWithBoard(std::vector<bool>& obs,
                                     const PastGameState& pastState,
                                     int startOffset, bool isWhite,
                                     bool is2FoldRep) {
    constexpr int pawnOffset = 0;
    constexpr int rookOffset = 1;
    constexpr int knightOffset = 2;
    constexpr int bishopOffset = 3;
    constexpr int queenOffset = 4;
    constexpr int kingOffset = 5;
    constexpr int whiteOffset = 0;
    constexpr int blackOffset = 6;
    constexpr int repetitionOffset = 12;

    Bitboard w_pawn = pastState.w_pawn;
    Bitboard w_rook = pastState.w_pawn;
    Bitboard w_knight = pastState.w_pawn;
    Bitboard w_bishop = pastState.w_pawn;
    Bitboard w_queen = pastState.w_pawn;
    Bitboard w_king = pastState.w_pawn;

    Bitboard b_pawn = pastState.b_pawn;
    Bitboard b_rook = pastState.b_pawn;
    Bitboard b_knight = pastState.b_pawn;
    Bitboard b_bishop = pastState.b_pawn;
    Bitboard b_queen = pastState.b_pawn;
    Bitboard b_king = pastState.b_pawn;

    Bitboard enpassant = pastState.enpassant_board;

    if (isWhite) {
        // black pawn can be taken enpassant on 6th rank we move it to 8th rank
        b_pawn |= enpassant << 16;
    } else {
        // white pawn can be taken enpassant on 3rd rank we move it to 8th rank
        w_pawn |= enpassant << 40;
    }

    Bitloop(w_pawn) {
        const uint64_t sourceSquare = SquareOf(w_pawn);
        obs[startOffset + (whiteOffset + pawnOffset) * PLANE_SIZE +
            sourceSquare] = true;
    }
    Bitloop(w_rook) {
        const uint64_t sourceSquare = SquareOf(w_rook);
        obs[startOffset + (whiteOffset + rookOffset) * PLANE_SIZE +
            sourceSquare] = true;
    }
    Bitloop(w_knight) {
        const uint64_t sourceSquare = SquareOf(w_knight);
        obs[startOffset + (whiteOffset + knightOffset) * PLANE_SIZE +
            sourceSquare] = true;
    }
    Bitloop(w_bishop) {
        const uint64_t sourceSquare = SquareOf(w_bishop);
        obs[startOffset + (whiteOffset + bishopOffset) * PLANE_SIZE +
            sourceSquare] = true;
    }
    Bitloop(w_queen) {
        const uint64_t sourceSquare = SquareOf(w_queen);
        obs[startOffset + (whiteOffset + queenOffset) * PLANE_SIZE +
            sourceSquare] = true;
    }
    Bitloop(w_king) {
        const uint64_t sourceSquare = SquareOf(w_king);
        obs[startOffset + (whiteOffset + kingOffset) * PLANE_SIZE +
            sourceSquare] = true;
    }

    Bitloop(b_pawn) {
        const uint64_t sourceSquare = SquareOf(b_pawn);
        obs[startOffset + (blackOffset + pawnOffset) * PLANE_SIZE +
            sourceSquare] = true;
    }
    Bitloop(b_rook) {
        const uint64_t sourceSquare = SquareOf(b_rook);
        obs[startOffset + (blackOffset + rookOffset) * PLANE_SIZE +
            sourceSquare] = true;
    }
    Bitloop(b_knight) {
        const uint64_t sourceSquare = SquareOf(b_knight);
        obs[startOffset + (blackOffset + knightOffset) * PLANE_SIZE +
            sourceSquare] = true;
    }
    Bitloop(b_bishop) {
        const uint64_t sourceSquare = SquareOf(b_bishop);
        obs[startOffset + (blackOffset + bishopOffset) * PLANE_SIZE +
            sourceSquare] = true;
    }
    Bitloop(b_queen) {
        const uint64_t sourceSquare = SquareOf(b_queen);
        obs[startOffset + (blackOffset + queenOffset) * PLANE_SIZE +
            sourceSquare] = true;
    }
    Bitloop(b_king) {
        const uint64_t sourceSquare = SquareOf(b_king);
        obs[startOffset + (blackOffset + kingOffset) * PLANE_SIZE +
            sourceSquare] = true;
    }
    std::fill_n(obs.begin() + startOffset + PLANE_SIZE * repetitionOffset,
                PLANE_SIZE, is2FoldRep);
}

inline void addEdgeFinder(std::vector<bool>& obs, int startOffset) {
    Bitboard border = RANK_1 | RANK_8 | FILE_A | FILE_H;
    Bitloop(border) {
        const uint64_t offset = SquareOf(border);
        obs[startOffset + offset] = true;
    }
}

inline bool isRookMove(uint64_t sourceSquare, uint64_t targetSquare) {
    const Bitboard attacks = Lookup::perSquareRookAttacks[sourceSquare][0];
    return attacks & (1ull << targetSquare);
}

inline bool isBishopMove(uint64_t sourceSquare, uint64_t targetSquare) {
    const Bitboard attacks = Lookup::perSquareBishopAttacks[sourceSquare][0];
    return attacks & (1ull << targetSquare);
}

template <bool isWhite>
inline Action getMoveIndex(Move move) {
    // TODO: make the bit ops here into a function
    const uint64_t sourceSquare =
        convertToColorSquare<isWhite>(move & 0b111111);
    const uint64_t sourceFile = sourceSquare % 8;
    const uint64_t sourceRank = sourceSquare / 8;

    const uint64_t targetSquare =
        convertToColorSquare<isWhite>((move >> 6) & 0b111111);
    const uint64_t flags = (move >> 12) & 0b1111;

    const int64_t offset = targetSquare - sourceSquare;

    // check for non queen promotion
    // TODO: turn this into a function
    if (flags >= 0b1000 && ((flags & 0b0011) != 0b0011)) {
        const uint64_t promoOffset = flags & 0b0011;
        const uint64_t plane = ((offset - 7) * 3) + 64 + promoOffset;
        return sourceFile * (NUM_ACTION_PLANES * 8) +
               sourceRank * NUM_ACTION_PLANES + plane;
    }

    // table lookup what move that is
    uint64_t plane;
    if (isRookMove(sourceSquare, targetSquare)) {
        plane = Lookup::getPlaneRook(offset);
    } else if (isBishopMove(sourceSquare, targetSquare)) {
        plane = Lookup::getPlaneBishop(offset);
    } else {
        plane = Lookup::getPlaneKnight(offset);
    }

    return sourceFile * (NUM_ACTION_PLANES * 8) +
           sourceRank * NUM_ACTION_PLANES + plane;
}
