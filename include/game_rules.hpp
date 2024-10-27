#pragma once

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
bool isDrawBy3FoldRepetition(const GameState& state) {
    const uint64_t posHash = state.getPositionHash();
    return occursMoreThan(state.positionHashes, posHash, 2);
}
