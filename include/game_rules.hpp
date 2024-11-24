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
bool isDrawBy3FoldRepetition(const GameState& state) {
    const uint64_t posHash = state.getPositionHash();
    return occursMoreThan(state.positionHashes, posHash, 2);
}

uint64_t getSquareColor(Bitboard board) {
    const uint64_t square = SquareOf(board);
    const uint64_t color = (square % 2) + (square / 8) % 2;
    return color;
}

// this function determins a draw so we don't need to make it a template
// we can just look at it from the white players perspective
//
// TODO: create one function for white and one for black to avoid
// code duplication
bool isInsufficientMaterial(const GameState& state) {
    Bitboard friends = getFriendlyPieces<true>(state);
    Bitboard enemies = getEnemyPieces<true>(state);
    uint64_t friend_count = std::popcount(friends);
    uint64_t enemy_count = std::popcount(enemies);

    // only king vs king
    if (friend_count <= 1 && enemy_count <= 1) {
        return true;
    }

    // king + bishop/knight vs king
    if (friend_count == 2 && enemy_count == 1 &&
        (getBishops<true>(state) || getKnights<true>(state))) {
        return true;
    }

    // king vs king + bishop/knight
    if (friend_count == 1 && enemy_count == 2 &&
        (getEnemyBishops<true>(state) || getEnemyKnights<true>(state))) {
        return true;
    }

    // king + 2 bishops vs king
    if (friend_count == 3 && enemy_count == 1) {
        Bitboard bishops = getBishops<true>(state);
        if (std::popcount(bishops) >= 2) {
            const uint64_t f_square = SquareOf(bishops);
            // reset the least significant bit (the first bishop)
            bishops = _blsr_u64(bishops);
            const uint64_t s_square = SquareOf(bishops);

            // NOTE: we convert the index to a bitboard again because of
            // how getSquareColor works
            const uint64_t f_color = getSquareColor(1ull << f_square);
            const uint64_t s_color = getSquareColor(1ull << s_square);

            if (f_color == s_color) {
                return true;
            }
        }
    }

    // king vs king + 2 bishops
    if (friend_count == 1 && enemy_count == 3) {
        Bitboard bishops = getEnemyBishops<true>(state);
        if (std::popcount(bishops) >= 2) {
            const uint64_t f_square = SquareOf(bishops);
            // reset the least significant bit (the first bishop)
            bishops = _blsr_u64(bishops);
            const uint64_t s_square = SquareOf(bishops);

            // NOTE: we convert the index to a bitboard again because of
            // how getSquareColor works
            const uint64_t f_color = getSquareColor(1ull << f_square);
            const uint64_t s_color = getSquareColor(1ull << s_square);

            if (f_color == s_color) {
                return true;
            }
        }
    }

    // this checks if both sides only have a bishop and they are both on
    // the same colored square
    if (friend_count == 2 && enemy_count == 2) {
        const Bitboard white_bishops = getBishops<true>(state);
        const Bitboard black_bishops = getEnemyBishops<true>(state);

        if (white_bishops && black_bishops) {
            const uint64_t white_color = getSquareColor(white_bishops);
            const uint64_t black_color = getSquareColor(black_bishops);

            if (white_color == black_color) {
                return true;
            }
        }
    }

    return false;
}
