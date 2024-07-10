#pragma once
#include <vector>
#include <immintrin.h>

#include "types.hpp"
#include "constants.hpp"
#include "lookup.hpp"
#include "moves.hpp"

#define SquareOf(X) _tzcnt_u64(X)
#define Bitloop(X) for(;X; X = _blsr_u64(X))


Move create_move(Bitboard from, Bitboard to, uint64_t flags) {
    return (from & 0x3f) | ((to & 0x3f) << 6) | ((flags & 0xf) << 12);
}

template<bool isWhite>
void get_legal_pawn_moves(GameState state, std::vector<Move> &moves) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    const Bitboard pawns = getPawns<isWhite>(state);
    const Bitboard enemyOrFriendly = enemies | friendlies;

    Bitboard pawnsNoPromo = pawns & ~SecondLastRank<isWhite>();
    Bitloop(pawnsNoPromo) {
        const uint64_t sourceSquare = SquareOf(pawnsNoPromo);
        const Bitboard sourceBoard = 1ull << sourceSquare;
        Bitboard targetSquares = 0ull;
        targetSquares |= PawnAttackLeft<isWhite>(sourceBoard & ~FILE_A) & enemies;
        targetSquares |= PawnAttackRight<isWhite>(sourceBoard & ~FILE_H) & enemies;
        targetSquares |= PawnPush1<isWhite>(sourceBoard) & ~enemyOrFriendly;
        targetSquares |= PawnPush1<isWhite>(PawnPush1<isWhite>(sourceBoard & RANK_2) & ~enemyOrFriendly) & ~enemyOrFriendly;
        Bitloop(targetSquares) {
            const uint64_t targetSquare = SquareOf(targetSquares);
            const Bitboard targetBoard = 1ull << targetSquare;
            uint64_t flags = 0ull;
            flags |= (targetBoard & PawnAttackLeft<isWhite>(sourceBoard & ~FILE_A)) >> targetSquare << 2;
            flags |= (targetBoard & PawnAttackRight<isWhite>(sourceBoard & ~FILE_H)) >> targetSquare << 2;
            flags |= (targetBoard & PawnPush2<isWhite>(sourceBoard)) >> targetSquare;
            moves.push_back(create_move(sourceSquare, targetSquare, flags));
        }
    }
    Bitboard pawnsPromo = pawns & SecondLastRank<isWhite>();
    Bitloop(pawnsPromo) {
        const uint64_t sourceSquare = SquareOf(pawnsPromo);
        const Bitboard sourceBoard = 1ull << sourceSquare;
        Bitboard targetSquares = 0ull;
        targetSquares |= PawnAttackLeft<isWhite>(sourceBoard & ~FILE_A) & enemies;
        targetSquares |= PawnAttackRight<isWhite>(sourceBoard & ~FILE_H) & enemies;
        targetSquares |= PawnPush1<isWhite>(sourceBoard) & ~enemyOrFriendly;
        Bitloop(targetSquares) {
            const uint64_t targetSquare = SquareOf(targetSquares);
            const Bitboard targetBoard = 1ull << targetSquare;
            uint64_t flags = 0ull;
            flags |= (targetBoard & PawnAttackLeft<isWhite>(sourceBoard & ~FILE_A)) >> targetSquare << 2;
            flags |= (targetBoard & PawnAttackRight<isWhite>(sourceBoard & ~FILE_H)) >> targetSquare << 2;
            moves.push_back(create_move(sourceSquare, targetSquare, flags | 0b1000));
            moves.push_back(create_move(sourceSquare, targetSquare, flags | 0b1001));
            moves.push_back(create_move(sourceSquare, targetSquare, flags | 0b1010));
            moves.push_back(create_move(sourceSquare, targetSquare, flags | 0b1011));
        }
    }
}


template<bool isWhite>
void get_legal_knight_moves(GameState state, std::vector<Move> &moves) {
    // TODO: pins missing still
    moves.reserve(16);

    Bitboard knights = getKnights<isWhite>(state);
    Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    Bitboard enemies = getEnemyPieces<isWhite>(state);
    // search for knights on the board
    Bitloop(knights) {
        const uint64_t sourceSquare = SquareOf(knights);
        Bitboard attackedSquares = Lookup::knightAttacks[sourceSquare] & ~friendlies;
        // for each attacked square (as) of the found knight
        Bitloop(attackedSquares) {
            const uint64_t targetSquare = SquareOf(attackedSquares);
            const Bitboard targetBoard = 1ull << targetSquare;
            // this should should check if we are capturing an 
            // enemy piece (enemy_pieces & (1ull << as)) and then
            // shift the bit that is either 1 or 0 to the 3rd least
            // significant possition which is the capture flag
            const uint64_t flags = (enemies & targetBoard) >> targetSquare << 2;
            moves.push_back(create_move(sourceSquare, targetSquare, flags));
        }
    }
}

// this function adds the move to the moves vector and returns if we continue or not
// we don't continue the search if we capture an enemy or a friedly piece
bool get_slider_move(int source, int target, Bitboard own_pieces, Bitboard enemy_pieces, std::vector<Move> &moves) {
    if (own_pieces & (1ull << target)) {
        // if own piece is met we break cause we can't go there
        return false;
    } else if (enemy_pieces & (1ull << target)) {
        // if enemy piece is met we add capture and break
        moves.push_back(create_move(source, target, 0b100));
        return false;
    } else {
        // if no piece is there we add the move and continue
        moves.push_back(create_move(source, target, 0b0));
        return true;
    }
}

template<bool isWhite>
void get_legal_bishop_moves(GameState state, std::vector<Move> &moves) {
    // TODO: pins are still missing and we don't have an attack map
    moves.reserve(22);
    Bitboard bishops = getBishops<isWhite>(state);
    Bitboard enemies = getEnemyPieces<isWhite>(state);
    Bitboard friendlies = getFriendlyPieces<isWhite>(state);

    // search for bishop on bitboard
    Bitloop (bishops) {
        const uint64_t sourceSquare = SquareOf(bishops);
        Bitboard attackMask = Lookup::bishopAttacks[sourceSquare];
        uint64_t blockIdx = _pext_u64(enemies | friendlies, attackMask);
        Bitboard targetsBoard = Lookup::perSquareBishopAttacks[sourceSquare][blockIdx] & ~friendlies;
        Bitloop (targetsBoard) {
            const uint64_t targetSquare = SquareOf(targetsBoard);
            const Bitboard targetBoard = 1ull << targetSquare;
            const uint64_t flags = (enemies & targetBoard) >> targetSquare << 2;
            moves.push_back(create_move(sourceSquare, targetSquare, flags));
        }
    }
}

template<bool isWhite>
void get_legal_rook_moves(GameState state, std::vector<Move> &moves) {
    // TODO: missing pin mask
    moves.reserve(22);

    Bitboard rooks = getRooks<isWhite>(state);
    Bitboard enemies = getEnemyPieces<isWhite>(state);
    Bitboard friendlies = getFriendlyPieces<isWhite>(state);

    // search for rooks on bitboard
    Bitloop (rooks) {
        const uint64_t sourceSquare = SquareOf(rooks);
        Bitboard attackMask = Lookup::rookAttacks[sourceSquare];
        uint64_t blockIdx = _pext_u64(enemies | friendlies, attackMask);
        Bitboard targetsBoard = Lookup::perSquareRookAttacks[sourceSquare][blockIdx];
        targetsBoard &= ~friendlies;
        Bitloop (targetsBoard) {
            const uint64_t targetSquare = SquareOf(targetsBoard);
            const Bitboard targetBoard = 1ull << targetSquare;
            const uint64_t flags = (enemies & targetBoard) >> targetSquare << 2;
            moves.push_back(create_move(sourceSquare, targetSquare, flags));
        }
    }
}

template<bool isWhite>
void get_legal_queen_moves(GameState state, std::vector<Move> &moves) {
    moves.reserve(16);
    Bitboard queens = getQueens<isWhite>(state);
    Bitboard enemies = getEnemyPieces<isWhite>(state);
    Bitboard friendlies = getFriendlyPieces<isWhite>(state);

    // search for rooks on bitboard
    Bitloop (queens) {
        const uint64_t sourceSquare = SquareOf(queens);
        const Bitboard attackMaskRook = Lookup::rookAttacks[sourceSquare];
        const uint64_t blockIdxRook = _pext_u64(enemies | friendlies, attackMaskRook);
        const Bitboard targetsBoardRook = Lookup::perSquareRookAttacks[sourceSquare][blockIdxRook];

        const Bitboard attackMaskBishop = Lookup::bishopAttacks[sourceSquare];
        const uint64_t blockIdxBishop = _pext_u64(enemies | friendlies, attackMaskBishop);
        const Bitboard targetsBoardBishop = Lookup::perSquareBishopAttacks[sourceSquare][blockIdxBishop];

        Bitboard targetsBoard = (targetsBoardRook | targetsBoardBishop) & ~friendlies;
        Bitloop (targetsBoard) {
            const uint64_t targetSquare = SquareOf(targetsBoard);
            const Bitboard targetBoard = 1ull << targetSquare;
            const uint64_t flags = (enemies & targetBoard) >> targetSquare << 2;
            moves.push_back(create_move(sourceSquare, targetSquare, flags));
        }
    }
}

template<bool isWhite>
Bitboard getKingAttacks(GameState state) {
    const Bitboard king = getKing<isWhite>(state);
    Bitboard attacks = 0ull;
    attacks |= king << 8;
    attacks |= (king & ~FILE_H) << 9;
    attacks |= (king & ~FILE_H) << 1;
    attacks |= (king & ~FILE_H) >> 7;
    attacks |= king >> 8;
    attacks |= (king & ~FILE_A) >> 9;
    attacks |= (king & ~FILE_A) >> 1;
    attacks |= (king & ~FILE_A) << 7;
    return attacks;
}

template<bool isWhite>
Bitboard getSeenSquares(GameState state) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    const Bitboard enemyKing = getKing<!isWhite>(state);
    const Bitboard blockingPieces = friendlies | enemies;// & ~enemyKing;

    const Bitboard pawns = getPawns<isWhite>(state);
    Bitboard knights = getKnights<isWhite>(state);
    Bitboard rooks = getRooks<isWhite>(state) | getQueens<isWhite>(state);
    Bitboard bishops = getBishops<isWhite>(state) | getQueens<isWhite>(state);

    Bitboard seenSquares = 0ull;

    // pawns
    seenSquares |= PawnAttackLeft<isWhite>(pawns & ~FILE_A);
    seenSquares |= PawnAttackRight<isWhite>(pawns & ~FILE_H);

    // knights
    Bitloop(knights) {
        const uint64_t sourceSquare = SquareOf(knights);
        const Bitboard attackBoard = Lookup::knightAttacks[sourceSquare];
        seenSquares |= attackBoard;
    }

    // rooks (+ queen)
    Bitloop(rooks) {
        const uint64_t sourceSquare = SquareOf(rooks);
        const Bitboard relevantMask = Lookup::rookAttacks[sourceSquare];
        const uint64_t blockIdx = _pext_u64(blockingPieces, relevantMask);
        const Bitboard targetsBoard = Lookup::perSquareRookAttacks[sourceSquare][blockIdx];
        seenSquares |= targetsBoard;
    }

    // bishops (+ queen)
    Bitloop(bishops) {
        const uint64_t sourceSquare = SquareOf(bishops);
        const Bitboard relevantMask = Lookup::bishopAttacks[sourceSquare];
        const uint64_t blockIdx = _pext_u64(blockingPieces, relevantMask);
        const Bitboard targetsBoard = Lookup::perSquareBishopAttacks[sourceSquare][blockIdx];
        seenSquares |= targetsBoard;
    }
    seenSquares |= getKingAttacks<isWhite>(state);

    return seenSquares;
} 

template<bool isWhite>
void get_legal_king_moves(GameState state, std::vector<Move> &moves) {
    const uint64_t kingSquare = SquareOf(getKing<isWhite>(state));
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    const Bitboard attackedSquares = getKingAttacks<isWhite>(state);
    const Bitboard enemySeenSquares = getSeenSquares<!isWhite>(state);
    Bitboard validTargets = attackedSquares & ~enemySeenSquares & ~friendlies;
    Bitloop (validTargets) {
        const uint64_t targetSquare = SquareOf(validTargets);
        const Bitboard targetBoard = 1ull << targetSquare;
        const uint64_t flags = (enemies & targetBoard) >> targetSquare << 2;
        moves.push_back(create_move(kingSquare, targetSquare, flags));
    }

}

template<bool isWhite>
std::vector<Move> get_legal_moves(GameState state) {
    std::vector<Move> moves;
    get_legal_pawn_moves<isWhite>(state, moves);
    get_legal_knight_moves<isWhite>(state, moves);
    get_legal_rook_moves<isWhite>(state, moves);
    get_legal_bishop_moves<isWhite>(state, moves);
    get_legal_queen_moves<isWhite>(state, moves);
    get_legal_king_moves<isWhite>(state, moves);
    return moves;
}

// TODO: test if this does what is should do

// this should set all bits to the same value
// as the first bit
inline Bitboard broadcast_bit(uint64_t number) {
    return -(number & 1ull) | (number & 1ull);
}

template<bool isWhite>
Bitboard get_check_mask(GameState state) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    const Bitboard king = getKing<isWhite>(state);
    const uint64_t kingSquare = SquareOf(king);


    Bitboard checkMask = 0ull;

    // pawn attacks
    Bitboard pawns = getEnemyPawns<isWhite>(state);
    Bitloop (pawns) {
        const uint64_t sourceSquare = SquareOf(pawns);
        const Bitboard sourceBoard = 1ull << sourceSquare;
        const Bitboard attacks = PawnAttackLeft<!isWhite>(sourceBoard & ~FILE_A) | PawnAttackRight<!isWhite>(sourceBoard & ~FILE_H);

        // this will be 1 at the index where the king 
        // if the king is attacked otherwise 0
        const Bitboard kingAttacked = king & attacks;
        // shift bit to least significant possition
        const uint64_t isAttacked = kingAttacked >> kingSquare;
        // no if the king is attacked all bits will be 1 
        // and if not all bits will be 0
        const Bitboard broadcasted = broadcast_bit(isAttacked);

        checkMask |= sourceBoard & broadcasted;
    }
    // knight attacks
    Bitboard knights = getEnemyKnights<isWhite>(state);
    Bitloop (knights) {
        const uint64_t sourceSquare = SquareOf(knights);
        const Bitboard sourceBoard = 1ull << sourceSquare;
        const Bitboard attacks = Lookup::knightAttacks[sourceSquare];

        // this will be 1 at the index where the king 
        // if the king is attacked otherwise 0
        const Bitboard kingAttacked = king & attacks;
        // shift bit to least significant possition
        const uint64_t isAttacked = kingAttacked >> kingSquare;
        // no if the king is attacked all bits will be 1 
        // and if not all bits will be 0
        const Bitboard broadcasted = broadcast_bit(isAttacked);

        checkMask |= sourceBoard & broadcasted;
    }

    // rook attacks (+queen rook attacks)
    Bitboard rooks = getEnemyRooks<isWhite>(state) | getEnemyQueens<isWhite>(state);
    Bitloop (rooks) {
        const uint64_t sourceSquare = SquareOf(rooks);
        const Bitboard sourceBoard = 1ull << sourceSquare;

        const Bitboard relevantIndices = Lookup::rookAttacks[sourceSquare];
        const uint64_t blockIdx = _pext_u64(enemies | friendlies, relevantIndices);
        const Bitboard attacks = Lookup::perSquareRookAttacks[sourceSquare][blockIdx];

        // this will be 1 at the index where the king 
        // if the king is attacked otherwise 0
        const Bitboard kingAttacked = king & attacks;
        // shift bit to least significant possition
        const uint64_t isAttacked = kingAttacked >> kingSquare;
        // no if the king is attacked all bits will be 1 
        // and if not all bits will be 0
        const Bitboard broadcasted = broadcast_bit(isAttacked);

        // this is more complated
        // 1. remove the attacks that don't point at the king
        Bitboard tempCheckMap = attacks & Lookup::rookAttacks[kingSquare];
        // 2. include the rook source square
        tempCheckMap |= sourceBoard;
        // 3. reset if king is not attacked
        tempCheckMap &= broadcasted;

        checkMask |= tempCheckMap;
    }

    // bishop attacks (+queen bishop attacks)
    Bitboard bishops = getEnemyBishops<isWhite>(state) | getEnemyQueens<isWhite>(state);
    Bitloop (bishops) {
        const uint64_t sourceSquare = SquareOf(bishops);
        const Bitboard sourceBoard = 1ull << sourceSquare;

        const Bitboard relevantIndices = Lookup::bishopAttacks[sourceSquare];
        const uint64_t blockIdx = _pext_u64(enemies | friendlies, relevantIndices);
        const Bitboard attacks = Lookup::perSquareBishopAttacks[sourceSquare][blockIdx];

        // this will be 1 at the index where the king 
        // if the king is attacked otherwise 0
        const Bitboard kingAttacked = king & attacks;
        // shift bit to least significant possition
        const uint64_t isAttacked = kingAttacked >> kingSquare;
        // no if the king is attacked all bits will be 1 
        // and if not all bits will be 0
        const Bitboard broadcasted = broadcast_bit(isAttacked);

        // this is more complated
        // 1. remove the attacks that don't point at the king
        Bitboard tempCheckMap = attacks & Lookup::bishopAttacks[kingSquare];
        // 2. include the bishop source square
        tempCheckMap |= sourceBoard;
        // 3. reset if king is not attacked
        tempCheckMap &= broadcasted;

        checkMask |= tempCheckMap;
    }
    return checkMask;
}



















