#pragma once

#include <bit>

#include "game_state.hpp"
#include "move_gen.hpp"

template <bool isWhite>
bool isCheckMate(const GameState& state) {
    const Bitboard enemySeenSquares = Movegen::getSeenSquares<!isWhite>(state);
    const Bitboard ownKing = getKing<isWhite>(state);

    // king is in check
    if (enemySeenSquares & ownKing) {
        // there are no legal moves
        const Moves legalMoves = Movegen::getLegalMoves(state);
        if (legalMoves.size() == 0) {
            return true;
        }
    }
    return false;
}

template <bool isWhite>
bool isStaleMate(const GameState& state) {
    const Bitboard enemySeenSquares = Movegen::getSeenSquares<!isWhite>(state);
    const Bitboard ownKing = getKing<isWhite>(state);

    // king is in check
    if (enemySeenSquares & ownKing) {
        return false;
    }

    // there are no legal moves
    const Moves legalMoves = Movegen::getLegalMoves(state);
    if (legalMoves.size() == 0) {
        return true;
    }
    return false;
}

bool isDrawBy50Moves(const GameState& state) {
    if (state.halfMoveClock >= 100) {
        return true;
    }
    return false;
}

bool occursMoreThan(const std::map<uint64_t, int>& map, uint64_t hash,
                    int threshold) {
    auto it = map.find(hash);
    if (it != map.end() && it->second > threshold) {
        return true;
    }
    return false;
}

template <bool isWhite>
bool isDrawBy3FoldRepetition(const GameState& state,
                             const std::map<uint64_t, int>& positionHashes) {
    const uint64_t posHash = getPositionHash(state);
    // std::cout << state.positionHashes.size() << std::endl;
    return occursMoreThan(positionHashes, posHash, 1);
}

uint64_t getSquareColor(Bitboard board) {
    const uint64_t square = SquareOf(board);
    const uint64_t color = ((square % 2) + (square / 8)) % 2;
    return color;
}

bool isInsufficientMaterial(const GameState& state) {
    Bitboard friends = getFriendlyPieces<true>(state);
    Bitboard enemies = getEnemyPieces<true>(state);
    uint64_t friend_count = std::popcount(friends);
    uint64_t enemy_count = std::popcount(enemies);

    // only kings
    if (friend_count == 1 && enemy_count == 1) {
        return true;
    }

    // only king + knight vs king
    if (friend_count == 2 && getKnights<true>(state) && enemy_count == 1) {
        return true;
    }
    // only king vs king + knight
    if (enemy_count == 2 && getEnemyKnights<true>(state) && friend_count == 1) {
        return true;
    }

    const Bitboard white_bishops = getBishops<true>(state);
    const Bitboard black_bishops = getEnemyBishops<true>(state);
    Bitboard bishops = white_bishops | black_bishops;
    const uint64_t num_bishops = std::popcount(bishops);

    // we only have bishops
    if (num_bishops && num_bishops == (friend_count + enemy_count - 2)) {
        bool bishop_insufficient = true;
        const uint64_t first_color = getSquareColor(bishops);
        bishops = _blsr_u64(bishops);
        Bitloop(bishops) {
            const uint64_t color = getSquareColor(bishops);
            if (color != first_color) {
                bishop_insufficient = false;
            }
        }
        return bishop_insufficient;
    }

    return false;
}
