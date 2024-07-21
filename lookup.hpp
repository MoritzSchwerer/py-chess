#pragma once
#include <array>
#include <immintrin.h>
#include <iostream>

#include "types.hpp"
#include "constants.hpp"

constexpr uint64_t rookAttackMaskSize = 4096;
constexpr uint64_t bishopAttackMaskSize = 512;

namespace Lookup {

constexpr std::array<Bitboard,64> generateKnightAttacks() {
    std::array<Bitboard, 64> squareAttacks;
    for (int i = 0; i < 64; i++) {
        Bitboard sourceSquare = 1ull << i;
        Bitboard attacks = 0ull;

        // 2 up 1 right/left
        attacks |= (sourceSquare << 15) & ~(RANK_1 | RANK_2) & ~FILE_H;
        attacks |= (sourceSquare << 17) & ~(RANK_1 | RANK_2) & ~FILE_A; 

        // 2 right 1 up/down
        attacks |= (sourceSquare << 10) & ~RANK_1 & ~(FILE_A | FILE_B);
        attacks |= (sourceSquare >> 6 ) & ~RANK_8 & ~(FILE_A | FILE_B);

        // 2 down 1 right/left
        attacks |= (sourceSquare >> 15) & ~(RANK_7 | RANK_8) & ~FILE_A;
        attacks |= (sourceSquare >> 17) & ~(RANK_7 | RANK_8) & ~FILE_H;

        // 2 left 1 up/down
        attacks |= (sourceSquare << 6 ) & ~RANK_1 & ~(FILE_G | FILE_H);
        attacks |= (sourceSquare >> 10) & ~RANK_8 & ~(FILE_G | FILE_H);

        squareAttacks[i] = attacks;
    }
    return squareAttacks;
}

constexpr std::array<Bitboard, 64> knightAttacks = generateKnightAttacks();

constexpr std::array<Bitboard, 64> generateBishopAttacks() {
    std::array<Bitboard, 64> squareAttacks;
    const Bitboard border = RANK_1 | RANK_8 | FILE_A | FILE_H;
    for (int ss = 0; ss < 64; ss++) {
        Bitboard attacks = 0ull;

        // north-east
        for (int ts = ss+9; ts < 64; ts+=9) {
            if (ts % 8 > ss % 8) attacks |= (1ull << ts) & ~border;
        }
        // south-east
        for (int ts = ss-7; ts >= 0; ts-=7) {
            if (ts % 8 > ss % 8) attacks |= (1ull << ts) & ~border;
        }
        // south-west
        for (int ts = ss-9; ts >= 0; ts-=9) { 
            if (ts % 8 < ss % 8) attacks |= (1ull << ts) & ~border;
        }
        // north-west
        for (int ts = ss+7; ts < 64; ts+=7) {
            if (ts % 8 < ss % 8) attacks |= (1ull << ts) & ~border;
        }
        squareAttacks[ss] = attacks;
    }
    return squareAttacks;
}

constexpr std::array<Bitboard, 64> bishopAttacks = generateBishopAttacks();

std::array<std::array<Bitboard, bishopAttackMaskSize>, 64> generatePerSquareBishopAttacks() {
    std::array<std::array<Bitboard, bishopAttackMaskSize>, 64> perSquareAttacks;
    const Bitboard border = RANK_1 | RANK_8 | FILE_A | FILE_H;
    for (uint64_t ss = 0; ss < 64; ss++) {
        const Bitboard attackMask = bishopAttacks[ss];
        for (uint64_t i = 0; i < bishopAttackMaskSize; i++) {
            const Bitboard blockers = _pdep_u64(i, attackMask);

            Bitboard attacks = 0ull;
            // north-east
            for (uint64_t ts = ss+9; ts < 64; ts+=9) {
                if (1ull << ss & FILE_H) break;
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers | targetBoard & border) break;
            }
            // south-east
            for (int64_t ts = ss-7; ts >= 0 && ts < 64; ts-=7) {
                if (1ull << ss & FILE_H) break;
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers | targetBoard & border) break;
            }
            // south-west
            for (uint64_t ts = ss-9; ts >= 0 && ts < 64; ts-=9) {
                if (1ull << ss & FILE_A) break;
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers | targetBoard & border) break;
            }
            // north-west
            for (uint64_t ts = ss+7; ts < 64; ts+=7) {
                if (1ull << ss & FILE_A) break;
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers | targetBoard & border) break;
            }
            perSquareAttacks[ss][i] = attacks;
        }
    }
    return perSquareAttacks;
}

std::array<std::array<Bitboard, bishopAttackMaskSize>, 64> perSquareBishopAttacks = generatePerSquareBishopAttacks();


std::array<std::array<Bitboard, bishopAttackMaskSize>, 64> generatePerSquareXrayBishopAttacks() {
    std::array<std::array<Bitboard, bishopAttackMaskSize>, 64> perSquareAttacks;
    const Bitboard border = RANK_1 | RANK_8 | FILE_A | FILE_H;
    for (uint64_t ss = 0; ss < 64; ss++) {
        const Bitboard attackMask = bishopAttacks[ss];
        for (uint64_t i = 0; i < bishopAttackMaskSize; i++) {
            const Bitboard blockers = _pdep_u64(i, attackMask);

            Bitboard attacks = 0ull;
            // north-east
            int hitCount = 0;
            for (uint64_t ts = ss+9; ts < 64; ts+=9) {
                if (1ull << ss & FILE_H) break;
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers) hitCount ++;
                if (hitCount >= 2 || targetBoard & border) break;
            }
            // south-east
            hitCount = 0;
            for (int64_t ts = ss-7; ts >= 0 && ts < 64; ts-=7) {
                if (1ull << ss & FILE_H) break;
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers) hitCount ++;
                if (hitCount >= 2 || targetBoard & border) break;
            }
            // south-west
            hitCount = 0;
            for (uint64_t ts = ss-9; ts >= 0 && ts < 64; ts-=9) {
                if (1ull << ss & FILE_A) break;
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers) hitCount ++;
                if (hitCount >= 2 || targetBoard & border) break;
            }
            // north-west
            hitCount = 0;
            for (uint64_t ts = ss+7; ts < 64; ts+=7) {
                if (1ull << ss & FILE_A) break;
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers) hitCount ++;
                if (hitCount >= 2 || targetBoard & border) break;
            }
            perSquareAttacks[ss][i] = attacks;
        }
    }
    return perSquareAttacks;
}

std::array<std::array<Bitboard, bishopAttackMaskSize>, 64> perSquareXrayBishopAttacks = generatePerSquareXrayBishopAttacks();


std::array<Bitboard, 64> generateRookAttacks() {
    std::array<Bitboard, 64> squareAttacks;
    const Bitboard hBorder = RANK_1 | RANK_8;
    const Bitboard vBorder = FILE_A | FILE_H;
    for (uint64_t ss = 0; ss < 64; ss++) {
        Bitboard attacks = 0ull;

        // north
        for (int ts = ss+8; ts < 64; ts+=8) {
            attacks |= (1ull << ts) & ~hBorder;
        }
        // east
        for (int ts = ss+1; ts < (ss / 8 * 8) + 8; ts++) {
            attacks |= (1ull << ts) & ~vBorder;
        }
        // south
        for (int ts = ss-8; ts >= 0; ts-=8) { 
            attacks |= (1ull << ts) & ~hBorder;
        }
        // west
        for (int ts = ss-1; ts > (ss / 8 * 8) && ts >=0; ts--) {
            attacks |= (1ull << ts) & ~vBorder;
        }
        squareAttacks[ss] = attacks;
    }
    return squareAttacks;
}

std::array<Bitboard, 64> rookAttacks = generateRookAttacks();


std::array<std::array<Bitboard, rookAttackMaskSize>, 64> generatePerSquareRookAttacks() {
    std::array<std::array<Bitboard, rookAttackMaskSize>, 64> perSquareAttacks;
    const Bitboard hBorder = RANK_1 | RANK_8;
    const Bitboard vBorder = FILE_A | FILE_H;
    for (uint64_t ss = 0; ss < 64; ss++) {
        const Bitboard attackMask = rookAttacks[ss];
        for (uint64_t i = 0; i < rookAttackMaskSize; i++) {
            const Bitboard blockers = _pdep_u64(i, attackMask);

            Bitboard attacks = 0ull;
            // north
            for (int ts = ss+8; ts < 64; ts+=8) {
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers | targetBoard & hBorder) break;
            }
            // east
            for (int ts = ss+1; ts < 64 && ts % 8 > 0; ts++) {
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers | targetBoard & vBorder) break;
            }
            // south
            for (int ts = ss-8; ts >= 0; ts-=8) {
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers | targetBoard & hBorder) break;
            }
            // west
            for (int ts = ss-1; ts >= 0 && ts % 8 < 7; ts--) {
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers | targetBoard & vBorder) break;
            }
            perSquareAttacks[ss][i] = attacks;
        }
    }
    return perSquareAttacks;
}

std::array<std::array<Bitboard, rookAttackMaskSize>, 64> perSquareRookAttacks = generatePerSquareRookAttacks();


std::array<std::array<Bitboard, rookAttackMaskSize>, 64> generatePerSquareXrayRookAttacks() {
    std::array<std::array<Bitboard, rookAttackMaskSize>, 64> perSquareAttacks;
    const Bitboard hBorder = RANK_1 | RANK_8;
    const Bitboard vBorder = FILE_A | FILE_H;
    for (uint64_t ss = 0; ss < 64; ss++) {
        const Bitboard attackMask = rookAttacks[ss];
        for (uint64_t i = 0; i < rookAttackMaskSize; i++) {
            const Bitboard blockers = _pdep_u64(i, attackMask);

            Bitboard attacks = 0ull;
            // north
            int hitCount = 0;
            for (int ts = ss+8; ts < 64; ts+=8) {
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers) hitCount++;
                if (hitCount >= 2 || targetBoard & hBorder) break;
            }
            // east
            hitCount = 0;
            for (int ts = ss+1; ts < 64 && ts % 8 > 0; ts++) {
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers) hitCount++;
                if (hitCount >= 2 || targetBoard & vBorder) break;
            }
            // south
            hitCount = 0;
            for (int ts = ss-8; ts >= 0; ts-=8) {
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers) hitCount++;
                if (hitCount >= 2| targetBoard & hBorder) break;
            }
            // west
            hitCount = 0;
            for (int ts = ss-1; ts >= 0 && ts % 8 < 7; ts--) {
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers) hitCount++;
                if (hitCount >= 2 || targetBoard & vBorder) break;
            }
            perSquareAttacks[ss][i] = attacks;
        }
    }
    return perSquareAttacks;
}

std::array<std::array<Bitboard, rookAttackMaskSize>, 64> perSquareXrayRookAttacks = generatePerSquareXrayRookAttacks();


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

PieceType getPromotion(uint8_t plane) {
    if (plane < 64) return PieceType::Queen;
    switch ((plane - 64) % 3) {
        case 0: return PieceType::Knight;
        case 1: return PieceType::Bishop;
        case 2: return PieceType::Rook;
        default: return PieceType::None;
    }
}

}
