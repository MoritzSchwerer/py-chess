#pragma once

#include <vector>
#include <iostream>

#include "types.hpp"
#include "constants.hpp"

template<bool isWhite>
constexpr Bitboard getEnemyPieces(GameState state) {
    if constexpr (isWhite) return state.b_pawn | state.b_rook | state.b_knight | state.b_bishop | state.b_queen | state.b_king;
    else return state.w_pawn | state.w_rook | state.w_knight | state.w_bishop | state.w_queen | state.w_king;
}
template<bool isWhite>
constexpr Bitboard getFriendlyPieces(GameState state) {
    if constexpr (isWhite) return state.w_pawn | state.w_rook | state.w_knight | state.w_bishop | state.w_queen | state.w_king;
    else return state.b_pawn | state.b_rook | state.b_knight | state.b_bishop | state.b_queen | state.b_king;
}

template<bool isWhite>
constexpr Bitboard getPawns(GameState state) {
    if constexpr (isWhite) return state.w_pawn;
    else return state.b_pawn;
}

template<bool isWhite>
constexpr Bitboard getKnights(GameState state) {
    if constexpr (isWhite) return state.w_knight;
    else return state.b_knight;
}

template<bool isWhite>
constexpr Bitboard getBishops(GameState state) {
    if constexpr (isWhite) return state.w_bishop;
    else return state.b_bishop;
}

template<bool isWhite>
constexpr Bitboard getRooks(GameState state) {
    if constexpr (isWhite) return state.w_rook;
    else return state.b_rook;
}

template<bool isWhite>
constexpr Bitboard getQueens(GameState state) {
    if constexpr (isWhite) return state.w_queen;
    else return state.b_queen;
}

template<bool isWhite>
constexpr Bitboard getKing(GameState state) {
    if constexpr (isWhite) return state.w_king;
    else return state.b_king;
}

template<bool isWhite>
constexpr Bitboard SecondLastRank() {
    if constexpr (isWhite) return RANK_7;
    else return RANK_2;
}

template<bool isWhite>
constexpr Bitboard PawnPush1(Bitboard pawns) {
    if constexpr (isWhite) return pawns << 8;
    else return pawns >> 8;
}

template<bool isWhite>
constexpr Bitboard PawnPush2(Bitboard pawns) {
    if constexpr (isWhite) return pawns << 16;
    else return pawns >> 16;
}

template<bool isWhite>
constexpr Bitboard PawnAttackLeft(Bitboard pawns) {
    if constexpr (isWhite) return pawns << 7;
    else return pawns >> 9;
}
template<bool isWhite>
constexpr Bitboard PawnAttackRight(Bitboard pawns) {
    if constexpr (isWhite) return pawns << 9;
    else return pawns >> 7;
}

