#pragma once

#include <cassert>

#include "game_state.hpp"
#include "moves.hpp"

namespace Zobrist {

constexpr uint64_t numPiecePositionColorElements = 768;
constexpr uint64_t blackOffset = 384; // (768 / 2)
constexpr uint64_t movingColorOffset = numPiecePositionColorElements;
constexpr uint64_t castlingOffset = numPiecePositionColorElements + 2;
constexpr uint64_t enpassantOffset = castlingOffset + 4;

uint64_t rand64(uint64_t n) {
    assert(n > 0);
    const uint64_t z = 0x9FB21C651E98DF25;
    n ^= ((n << 49) | (n >> 15)) ^ ((n << 24) | (n >> 40));
    n *= z;
    n ^= n >> 28;
    n *= z;
    n ^= n >> 28;
    return n;
}

template<bool isWhite>
uint64_t hashMovingColor() {
    if constexpr (isWhite) return rand64(movingColorOffset);
    else return rand64(movingColorOffset+1);
}

uint64_t hashCastlingRights(const GameState& state) {
    uint64_t hash;
    const GameStatus status = state.status;
    if (status.wKingC) {
        hash ^= rand64(castlingOffset);
    }
    if (status.wQueenC) {
        hash ^= rand64(castlingOffset+1);
    }
    if (status.bKingC) {
        hash ^= rand64(castlingOffset+2);
    }
    if (status.bQueenC) {
        hash ^= rand64(castlingOffset+3);
    }
    return hash;
}

uint64_t getPieceIndex(bool isWhite, uint64_t typeIndex, uint64_t square) {
    if (isWhite) {
        return typeIndex*64 + square;
    } else {
        return typeIndex*64 + square + blackOffset;
    }
}

uint64_t hashPieces(const GameState& state) {
    Bitboard w_pawn = state.w_pawn; 
    Bitboard w_rook = state.w_rook; 
    Bitboard w_knight = state.w_knight; 
    Bitboard w_bishop = state.w_bishop; 
    Bitboard w_queen = state.w_queen; 
    Bitboard w_king = state.w_king; 

    Bitboard b_pawn = state.b_pawn; 
    Bitboard b_rook = state.b_rook; 
    Bitboard b_knight = state.b_knight; 
    Bitboard b_bishop = state.b_bishop; 
    Bitboard b_queen = state.b_queen; 
    Bitboard b_king = state.b_king; 

    uint64_t hash;
    Bitloop(w_pawn) {
        const uint64_t square = SquareOf(w_pawn);
        const uint64_t index = getPieceIndex(true, 0ull, square);
        hash ^= rand64(index);
    }

    Bitloop(w_rook) {
        const uint64_t square = SquareOf(w_rook);
        const uint64_t index = getPieceIndex(true, 1ull, square);
        hash ^= rand64(index);
    }

    Bitloop(w_knight) {
        const uint64_t square = SquareOf(w_knight);
        const uint64_t index = getPieceIndex(true, 2ull, square);
        hash ^= rand64(index);
    }

    Bitloop(w_bishop) {
        const uint64_t square = SquareOf(w_bishop);
        const uint64_t index = getPieceIndex(true, 3ull, square);
        hash ^= rand64(index);
    }

    Bitloop(w_queen) {
        const uint64_t square = SquareOf(w_queen);
        const uint64_t index = getPieceIndex(true, 4ull, square);
        hash ^= rand64(index);
    }

    Bitloop(w_king) {
        const uint64_t square = SquareOf(w_king);
        const uint64_t index = getPieceIndex(true, 5ull, square);
        hash ^= rand64(index);
    }

    Bitloop(b_pawn) {
        const uint64_t square = SquareOf(b_pawn);
        const uint64_t index = getPieceIndex(false, 6ull, square);
        hash ^= rand64(index);
    }

    Bitloop(b_rook) {
        const uint64_t square = SquareOf(b_rook);
        const uint64_t index = getPieceIndex(false, 7ull, square);
        hash ^= rand64(index);
    }

    Bitloop(b_knight) {
        const uint64_t square = SquareOf(b_knight);
        const uint64_t index = getPieceIndex(false, 8ull, square);
        hash ^= rand64(index);
    }

    Bitloop(b_bishop) {
        const uint64_t square = SquareOf(b_bishop);
        const uint64_t index = getPieceIndex(false, 9ull, square);
        hash ^= rand64(index);
    }

    Bitloop(b_queen) {
        const uint64_t square = SquareOf(b_queen);
        const uint64_t index = getPieceIndex(false, 10ull, square);
        hash ^= rand64(index);
    }

    Bitloop(b_king) {
        const uint64_t square = SquareOf(b_king);
        const uint64_t index = getPieceIndex(true, 11ull, square);
        hash ^= rand64(index);
    }
    return hash;
}

template<bool isWhite>
uint64_t hashEnpassant(const GameState& state) {
    // if there is an enpassant pawn calculate index [0, 16)
    if (state.status.enpassant) {
        const Bitboard enpassant = state.enpassant_board;
        const uint64_t enpassantSquare = SquareOf(enpassant);
        const uint64_t enpassantFile = enpassantSquare % 8;
        uint64_t enpassantIndex = enpassantFile;
        // if current color is white the pawn is black
        // so we add 8 to destinguish between white and black
        if constexpr (isWhite) enpassantIndex += 8;
        return rand64(enpassantIndex);
    }
    return 0ull;
}


template<bool isWhite>
uint64_t hashBoard(const GameState& state) {
    uint64_t hash;
    hash ^= hashMovingColor<isWhite>();
    hash ^= hashCastlingRights(state);
    hash ^= hashPieces(state);
    hash ^= hashEnpassant<isWhite>(state);
    return hash;
}


}
