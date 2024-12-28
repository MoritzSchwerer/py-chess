#pragma once
#include <array>
#include <immintrin.h>
#include <iostream>

#include "constants.hpp"
#include "types.hpp"

constexpr uint64_t rookAttackMaskSize = 4096;
constexpr uint64_t bishopAttackMaskSize = 512;

namespace Lookup {

constexpr std::array<Bitboard, 64> generateKnightAttacks() {
    std::array<Bitboard, 64> squareAttacks;
    for (int i = 0; i < 64; i++) {
        Bitboard sourceSquare = 1ull << i;
        Bitboard attacks = 0ull;

        // 2 up 1 right/left
        attacks |= (sourceSquare << 15) & ~(RANK_1 | RANK_2) & ~FILE_H;
        attacks |= (sourceSquare << 17) & ~(RANK_1 | RANK_2) & ~FILE_A;

        // 2 right 1 up/down
        attacks |= (sourceSquare << 10) & ~RANK_1 & ~(FILE_A | FILE_B);
        attacks |= (sourceSquare >> 6) & ~RANK_8 & ~(FILE_A | FILE_B);

        // 2 down 1 right/left
        attacks |= (sourceSquare >> 15) & ~(RANK_7 | RANK_8) & ~FILE_A;
        attacks |= (sourceSquare >> 17) & ~(RANK_7 | RANK_8) & ~FILE_H;

        // 2 left 1 up/down
        attacks |= (sourceSquare << 6) & ~RANK_1 & ~(FILE_G | FILE_H);
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
        for (int ts = ss + 9; ts < 64; ts += 9) {
            if (ts % 8 > ss % 8) attacks |= (1ull << ts) & ~border;
        }
        // south-east
        for (int ts = ss - 7; ts >= 0; ts -= 7) {
            if (ts % 8 > ss % 8) attacks |= (1ull << ts) & ~border;
        }
        // south-west
        for (int ts = ss - 9; ts >= 0; ts -= 9) {
            if (ts % 8 < ss % 8) attacks |= (1ull << ts) & ~border;
        }
        // north-west
        for (int ts = ss + 7; ts < 64; ts += 7) {
            if (ts % 8 < ss % 8) attacks |= (1ull << ts) & ~border;
        }
        squareAttacks[ss] = attacks;
    }
    return squareAttacks;
}

constexpr std::array<Bitboard, 64> bishopAttacks = generateBishopAttacks();

constexpr std::array<std::array<Bitboard, bishopAttackMaskSize>, 64>
generatePerSquareBishopAttacks() noexcept {
    std::array<std::array<Bitboard, bishopAttackMaskSize>, 64> perSquareAttacks;
    const Bitboard border = RANK_1 | RANK_8 | FILE_A | FILE_H;
    for (uint64_t ss = 0; ss < 64; ss++) {
        const Bitboard attackMask = bishopAttacks[ss];
        for (uint64_t i = 0; i < bishopAttackMaskSize; i++) {
            const Bitboard blockers = _pdep_u64(i, attackMask);

            Bitboard attacks = 0ull;
            // north-east
            for (uint64_t ts = ss + 9; ts < 64; ts += 9) {
                if (1ull << ss & FILE_H) break;
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if ((targetBoard & blockers) | (targetBoard & border)) break;
            }
            // south-east
            for (int64_t ts = ss - 7; ts >= 0 && ts < 64; ts -= 7) {
                if (1ull << ss & FILE_H) break;
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if ((targetBoard & blockers) | (targetBoard & border)) break;
            }
            // south-west
            for (uint64_t ts = ss - 9; ts >= 0 && ts < 64; ts -= 9) {
                if (1ull << ss & FILE_A) break;
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if ((targetBoard & blockers) | (targetBoard & border)) break;
            }
            // north-west
            for (uint64_t ts = ss + 7; ts < 64; ts += 7) {
                if (1ull << ss & FILE_A) break;
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if ((targetBoard & blockers) | (targetBoard & border)) break;
            }
            perSquareAttacks[ss][i] = attacks;
        }
    }
    return perSquareAttacks;
}

std::array<std::array<Bitboard, bishopAttackMaskSize>, 64>
    perSquareBishopAttacks = generatePerSquareBishopAttacks();

constexpr std::array<std::array<Bitboard, bishopAttackMaskSize>, 64>
generatePerSquareXrayBishopAttacks() noexcept {
    std::array<std::array<Bitboard, bishopAttackMaskSize>, 64> perSquareAttacks;
    const Bitboard border = RANK_1 | RANK_8 | FILE_A | FILE_H;
    for (uint64_t ss = 0; ss < 64; ss++) {
        const Bitboard attackMask = bishopAttacks[ss];
        for (uint64_t i = 0; i < bishopAttackMaskSize; i++) {
            const Bitboard blockers = _pdep_u64(i, attackMask);

            Bitboard attacks = 0ull;
            // north-east
            int hitCount = 0;
            for (uint64_t ts = ss + 9; ts < 64; ts += 9) {
                if (1ull << ss & FILE_H) break;
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers) hitCount++;
                if (hitCount >= 2 || (targetBoard & border)) break;
            }
            // south-east
            hitCount = 0;
            for (int64_t ts = ss - 7; ts >= 0 && ts < 64; ts -= 7) {
                if (1ull << ss & FILE_H) break;
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers) hitCount++;
                if (hitCount >= 2 || (targetBoard & border)) break;
            }
            // south-west
            hitCount = 0;
            for (uint64_t ts = ss - 9; ts >= 0 && ts < 64; ts -= 9) {
                if (1ull << ss & FILE_A) break;
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers) hitCount++;
                if (hitCount >= 2 || (targetBoard & border)) break;
            }
            // north-west
            hitCount = 0;
            for (uint64_t ts = ss + 7; ts < 64; ts += 7) {
                if (1ull << ss & FILE_A) break;
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers) hitCount++;
                if (hitCount >= 2 || (targetBoard & border)) break;
            }
            perSquareAttacks[ss][i] = attacks;
        }
    }
    return perSquareAttacks;
}

std::array<std::array<Bitboard, bishopAttackMaskSize>, 64>
    perSquareXrayBishopAttacks = generatePerSquareXrayBishopAttacks();

constexpr std::array<Bitboard, 64> generateRookAttacks() {
    std::array<Bitboard, 64> squareAttacks;
    const Bitboard hBorder = RANK_1 | RANK_8;
    const Bitboard vBorder = FILE_A | FILE_H;
    for (uint64_t ss = 0; ss < 64; ss++) {
        Bitboard attacks = 0ull;

        // north
        for (int ts = ss + 8; ts < 64; ts += 8) {
            attacks |= (1ull << ts) & ~hBorder;
        }
        // east
        for (int ts = ss + 1; ts < (ss / 8 * 8) + 8; ts++) {
            attacks |= (1ull << ts) & ~vBorder;
        }
        // south
        for (int ts = ss - 8; ts >= 0; ts -= 8) {
            attacks |= (1ull << ts) & ~hBorder;
        }
        // west
        for (int ts = ss - 1; ts > (ss / 8 * 8) && ts >= 0; ts--) {
            attacks |= (1ull << ts) & ~vBorder;
        }
        squareAttacks[ss] = attacks;
    }
    return squareAttacks;
}

constexpr std::array<Bitboard, 64> rookAttacks = generateRookAttacks();

constexpr std::array<std::array<Bitboard, rookAttackMaskSize>, 64>
generatePerSquareRookAttacks() noexcept {
    std::array<std::array<Bitboard, rookAttackMaskSize>, 64> perSquareAttacks;
    const Bitboard hBorder = RANK_1 | RANK_8;
    const Bitboard vBorder = FILE_A | FILE_H;
    for (uint64_t ss = 0; ss < 64; ss++) {
        const Bitboard attackMask = rookAttacks[ss];
        for (uint64_t i = 0; i < rookAttackMaskSize; i++) {
            const Bitboard blockers = _pdep_u64(i, attackMask);

            Bitboard attacks = 0ull;
            // north
            for (int ts = ss + 8; ts < 64; ts += 8) {
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if ((targetBoard & blockers) | (targetBoard & hBorder)) break;
            }
            // east
            for (int ts = ss + 1; ts < 64 && ts % 8 > 0; ts++) {
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if ((targetBoard & blockers) | (targetBoard & vBorder)) break;
            }
            // south
            for (int ts = ss - 8; ts >= 0; ts -= 8) {
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if ((targetBoard & blockers) | (targetBoard & hBorder)) break;
            }
            // west
            for (int ts = ss - 1; ts >= 0 && ts % 8 < 7; ts--) {
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if ((targetBoard & blockers) | (targetBoard & vBorder)) break;
            }
            perSquareAttacks[ss][i] = attacks;
        }
    }
    return perSquareAttacks;
}

std::array<std::array<Bitboard, rookAttackMaskSize>, 64> perSquareRookAttacks =
    generatePerSquareRookAttacks();

std::array<std::array<Bitboard, rookAttackMaskSize>, 64>
generatePerSquareXrayRookAttacks() noexcept {
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
            for (int ts = ss + 8; ts < 64; ts += 8) {
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers) hitCount++;
                if (hitCount >= 2 || (targetBoard & hBorder)) break;
            }
            // east
            hitCount = 0;
            for (int ts = ss + 1; ts < 64 && ts % 8 > 0; ts++) {
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers) hitCount++;
                if (hitCount >= 2 || (targetBoard & vBorder)) break;
            }
            // south
            hitCount = 0;
            for (int ts = ss - 8; ts >= 0; ts -= 8) {
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers) hitCount++;
                if ((hitCount >= 2) || (targetBoard & hBorder)) break;
            }
            // west
            hitCount = 0;
            for (int ts = ss - 1; ts >= 0 && ts % 8 < 7; ts--) {
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers) hitCount++;
                if (hitCount >= 2 || (targetBoard & vBorder)) break;
            }
            perSquareAttacks[ss][i] = attacks;
        }
    }
    return perSquareAttacks;
}

std::array<std::array<Bitboard, rookAttackMaskSize>, 64>
    perSquareXrayRookAttacks = generatePerSquareXrayRookAttacks();

constexpr std::array<int8_t, 73> planeToOffsetWhite = {
    -9,  -1,  7,   -8,  8,   -7,  1,   9,   -18, -2,  14,  -16, 16,  -14, 2,
    18,  -27, -3,  21,  -24, 24,  -21, 3,   27,  -36, -4,  28,  -32, 32,  -28,
    4,   36,  -45, -5,  35,  -40, 40,  -35, 5,   45,  -54, -6,  42,  -48, 48,
    -42, 6,   54,  -63, -7,  49,  -56, 56,  -49, 7,   63,  -10, 6,   -17, 15,
    -15, 17,  -6,  10,  7,   7,   7,   8,   8,   8,   9,   9,   9};

constexpr std::array<int8_t, 73> planeToOffsetBlack = {
    -9,  -1,  7,   -8,  8,   -7,  1,   9,   -18, -2,  14,  -16, 16,  -14, 2,
    18,  -27, -3,  21,  -24, 24,  -21, 3,   27,  -36, -4,  28,  -32, 32,  -28,
    4,   36,  -45, -5,  35,  -40, 40,  -35, 5,   45,  -54, -6,  42,  -48, 48,
    -42, 6,   54,  -63, -7,  49,  -56, 56,  -49, 7,   63,  -10, 6,   -17, 15,
    -15, 17,  -6,  10,  7,   7,   7,   8,   8,   8,   9,   9,   9};

constexpr std::array<int8_t, 56> planeToOffsetRook = {
    0,   -1, 0, -8, 8, 0,   1,  0, 0,   -2, 0, -16, 16, 0,   2,  0, 0,   -3, 0,
    -24, 24, 0, 3,  0, 0,   -4, 0, -32, 32, 0, 4,   0,  0,   -5, 0, -40, 40, 0,
    5,   0,  0, -6, 0, -48, 48, 0, 6,   0,  0, -7,  0,  -56, 56, 0, 7,   0,
};

constexpr std::array<int8_t, 56> planeToOffsetBishop = {
    -9, 0,   7,   0,   0,   -7, 0,   9,   -18, 0,   14,  0,   0,   -14,
    0,  18,  -27, 0,   21,  0,  0,   -21, 0,   27,  -36, 0,   28,  0,
    0,  -28, 0,   36,  -45, 0,  35,  0,   0,   -35, 0,   45,  -54, 0,
    42, 0,   0,   -42, 0,   54, -63, 0,   49,  0,   0,   -49, 0,   63,
};

constexpr std::array<int8_t, 8> planeToOffsetKnight = {
    -10, 6, -17, 15, -15, 17, -6, 10,
};

constexpr std::array<int8_t, 9> planeToOffsetPawnWhite = {
    /* pawn moves (L, P, R) (N, B, R)*/ 7,
    7,
    7,
    8,
    8,
    8,
    9,
    9,
    9};
constexpr std::array<int8_t, 9> planeToOffsetPawnBlack = {
    /* pawn moves (L, P, R) (N, B, R)*/ -9,
    -9,
    -9,
    -8,
    -8,
    -8,
    -7,
    -7,
    -7};

constexpr std::array<uint8_t, 128> generateOffsetToPlaneRook() noexcept {
    std::array<uint8_t, 128> inverted;
    // we do + 64 to ensure that we don't occur negative values
    for (int i = 0; i < 56; i++) {
        const int8_t element = planeToOffsetRook[i] + 64;
        inverted[element] = i;
    }
    // for (int i = 0; i < 128; i++) {
    //     std::cout << i << ": " << static_cast<uint64_t>(inverted[i]) <<
    //     std::endl;
    // }
    return inverted;
}

constexpr std::array<uint8_t, 128> generateOffsetToPlaneBishop() noexcept {
    std::array<uint8_t, 128> inverted;
    // we do + 64 to ensure that we don't occur negative values
    for (int i = 0; i < 56; i++) {
        const int8_t element = planeToOffsetBishop[i] + 64;
        inverted[element] = i;
    }
    // for (int i = 0; i < 128; i++) {
    //     std::cout << i << ": " << static_cast<uint64_t>(inverted[i]) <<
    //     std::endl;
    // }
    return inverted;
}

constexpr std::array<uint8_t, 36> generateOffsetToPlaneKnight() noexcept {
    std::array<uint8_t, 36> inverted;
    // we do + 18 to ensure that we don't occur negative values
    for (int i = 0; i < 8; i++) {
        const int8_t element = planeToOffsetKnight[i] + 18;
        inverted[element] = i;
    }
    return inverted;
}

std::array<uint8_t, 128> offsetToPlaneRook = generateOffsetToPlaneRook();
std::array<uint8_t, 128> offsetToPlaneBishop = generateOffsetToPlaneBishop();
std::array<uint8_t, 36> offsetToPlaneKnight = generateOffsetToPlaneKnight();
// for white do -7 to get to [0, 3] and for black +9
// std::array<uint8_t, 3> offsetToPlanePawn = {7, 8, 9}

PieceType getPromotion(uint8_t plane) {
    if (plane < 64) return PieceType::Queen;
    switch ((plane - 64) % 3) {
        case 0:
            return PieceType::Knight;
        case 1:
            return PieceType::Bishop;
        case 2:
            return PieceType::Rook;
        default:
            return PieceType::None;
    }
}

template <bool isWhite>
int8_t getOffsetFromPlane(uint8_t plane) {
    if (isWhite)
        return planeToOffsetWhite[plane];
    else
        return planeToOffsetBlack[plane];
}

// offset + 64 to make it non negative
uint8_t getPlaneRook(int8_t offset) { return offsetToPlaneRook[offset + 64]; }

uint8_t getPlaneBishop(int8_t offset) {
    return offsetToPlaneBishop[offset + 64];
}

// 56 here since the queen moves are the first 56 and after
uint8_t getPlaneKnight(int8_t offset) {
    return 56 + offsetToPlaneKnight[offset + 18];
}

}  // namespace Lookup
