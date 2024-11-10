#pragma once

#include <immintrin.h>
#include <iostream>
#include <vector>

#include "constants.hpp"
#include "game_state.hpp"
#include "types.hpp"

// #define SquareOf(X) _tzcnt_u64(X)
#define Bitloop(X) for (; X; X = _blsr_u64(X))

inline uint64_t SquareOf(uint64_t x) { return _tzcnt_u64(x); }

// this should set all bits to the same value
// as the first bit
inline Bitboard broadcastBit(Bitboard number) {
    return -(number & 1ull) | (number & 1ull);
}

inline Bitboard broadcastSingleToMask(Bitboard number) {
    const uint64_t shifted = number >> SquareOf(number);
    return broadcastBit(shifted);
}

template <bool isWhite>
constexpr Bitboard getEnemyPieces(const GameState &state) {
    if constexpr (isWhite)
        return state.b_pawn | state.b_rook | state.b_knight | state.b_bishop |
               state.b_queen | state.b_king;
    else
        return state.w_pawn | state.w_rook | state.w_knight | state.w_bishop |
               state.w_queen | state.w_king;
}

template <bool isWhite>
bool isEnemyPiece(const GameState &state, uint64_t square) {
    return getEnemyPieces<isWhite>(state) & (1ull << square);
}

template <bool isWhite>
constexpr Bitboard getFriendlyPieces(const GameState &state) {
    if constexpr (isWhite)
        return state.w_pawn | state.w_rook | state.w_knight | state.w_bishop |
               state.w_queen | state.w_king;
    else
        return state.b_pawn | state.b_rook | state.b_knight | state.b_bishop |
               state.b_queen | state.b_king;
}

template <bool isWhite>
constexpr Bitboard getPawns(const GameState &state) {
    if constexpr (isWhite)
        return state.w_pawn;
    else
        return state.b_pawn;
}

template <bool isWhite>
constexpr Bitboard getEnemyPawns(const GameState &state) {
    if constexpr (isWhite)
        return state.b_pawn;
    else
        return state.w_pawn;
}

template <bool isWhite>
constexpr Bitboard getKnights(const GameState &state) {
    if constexpr (isWhite)
        return state.w_knight;
    else
        return state.b_knight;
}

template <bool isWhite>
constexpr Bitboard getEnemyKnights(const GameState &state) {
    if constexpr (isWhite)
        return state.b_knight;
    else
        return state.w_knight;
}

template <bool isWhite>
constexpr Bitboard getBishops(const GameState &state) {
    if constexpr (isWhite)
        return state.w_bishop;
    else
        return state.b_bishop;
}

template <bool isWhite>
constexpr Bitboard getEnemyBishops(const GameState &state) {
    if constexpr (isWhite)
        return state.b_bishop;
    else
        return state.w_bishop;
}

template <bool isWhite>
constexpr Bitboard getRooks(const GameState &state) {
    if constexpr (isWhite)
        return state.w_rook;
    else
        return state.b_rook;
}

template <bool isWhite>
constexpr Bitboard getEnemyRooks(const GameState &state) {
    if constexpr (isWhite)
        return state.b_rook;
    else
        return state.w_rook;
}

template <bool isWhite>
constexpr Bitboard getQueens(const GameState &state) {
    if constexpr (isWhite)
        return state.w_queen;
    else
        return state.b_queen;
}

template <bool isWhite>
constexpr Bitboard getEnemyQueens(const GameState &state) {
    if constexpr (isWhite)
        return state.b_queen;
    else
        return state.w_queen;
}

template <bool isWhite>
constexpr Bitboard getKing(const GameState &state) {
    if constexpr (isWhite)
        return state.w_king;
    else
        return state.b_king;
}

template <bool isWhite>
constexpr Bitboard getEnemyKing(const GameState &state) {
    if constexpr (isWhite)
        return state.b_king;
    else
        return state.w_king;
}

template <bool isWhite>
constexpr Bitboard secondLastRank() {
    if constexpr (isWhite)
        return RANK_7;
    else
        return RANK_2;
}

template <bool isWhite>
constexpr Bitboard secondRank() {
    if constexpr (isWhite)
        return RANK_2;
    else
        return RANK_7;
}

template <bool isWhite>
constexpr Bitboard lastRank() {
    if constexpr (isWhite)
        return RANK_8;
    else
        return RANK_1;
}

template <bool isWhite>
constexpr Bitboard pawnPush1(Bitboard pawns) {
    if constexpr (isWhite)
        return pawns << 8;
    else
        return pawns >> 8;
}

template <bool isWhite>
constexpr Bitboard pawnPush2(Bitboard pawns) {
    if constexpr (isWhite)
        return pawns << 16;
    else
        return pawns >> 16;
}

template <bool isWhite>
constexpr Bitboard pawnAttackLeft(Bitboard pawns) {
    if constexpr (isWhite)
        return pawns << 7;
    else
        return pawns >> 9;
}

template <bool isWhite>
constexpr Bitboard pawnAttackRight(Bitboard pawns) {
    if constexpr (isWhite)
        return pawns << 9;
    else
        return pawns >> 7;
}

template <bool isWhite>
constexpr Bitboard initialRookLeft() {
    if constexpr (isWhite)
        return 1ull;
    else
        return 1ull << 56;
}

template <bool isWhite>
constexpr Bitboard initialRookRight() {
    if constexpr (isWhite)
        return 0b10000000;
    else
        return 0b10000000ull << 56;
}
