#pragma once
#include <array>
#include <immintrin.h>
#include <iostream>

#include "types.hpp"
#include "constants.hpp"

void to_binary_(Bitboard board) {
    for (int i = 63; i >= 0; i--) {
        bool is_set = (board & (1ull << i));
        std::cout << is_set;
    }
    std::cout << std::endl;
}


void print_board_(Bitboard board) {
    std::cout << "-------------------\n";
    for (int rank = 7; rank >= 0; --rank){
        for (int col = 0; col < 8; ++col) {
            if (col == 0) std::cout << "| ";
            bool is_set = board & (1ull << (rank*8+col));
            if (is_set) {
                std::cout << "x ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << "|\n";
    }
    std::cout << "-------------------" << std::endl;
}

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
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers | targetBoard & border) break;
            }
            // south-east
            for (int64_t ts = ss-7; ts >= 0 && ts < 64; ts-=7) {
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers | targetBoard & border) break;
            }
            // south-west
            for (uint64_t ts = ss-9; ts >= 0 && ts < 64; ts-=9) {
                const Bitboard targetBoard = 1ull << ts;
                attacks |= targetBoard;
                if (targetBoard & blockers | targetBoard & border) break;
            }
            // north-west
            for (uint64_t ts = ss+7; ts < 64; ts+=7) {
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

}
