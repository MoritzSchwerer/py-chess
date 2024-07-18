#pragma once
#include <vector>

#include "types.hpp"
#include "constants.hpp"
#include "lookup.hpp"
#include "moves.hpp"
#include "utils.hpp"

Move create_move(Bitboard from, Bitboard to, uint64_t flags) {
    return (from & 0x3f) | ((to & 0x3f) << 6) | ((flags & 0xf) << 12);
}

// TODO: we need to make sure that the slider checks only include
// the squares in the direction to the enemy king and not the other
// direction.
// Solution: lookup slider attacks from king towards the attacking piece
// and & that with the slider attack mask
template<bool isWhite>
Bitboard getCheckMask(GameState state) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    const Bitboard king = getKing<isWhite>(state);
    const uint64_t kingSquare = SquareOf(king);


    Bitboard checkMask = 0xffffffffffffffff;

    // pawn attacks
    Bitboard pawns = getEnemyPawns<isWhite>(state);
    Bitloop (pawns) {
        const uint64_t sourceSquare = SquareOf(pawns);
        const Bitboard sourceBoard = 1ull << sourceSquare;
        const Bitboard attacks = pawnAttackLeft<!isWhite>(sourceBoard & ~FILE_A) | pawnAttackRight<!isWhite>(sourceBoard & ~FILE_H);
        const Bitboard broadcasted = broadcastSingleToMask(king & attacks);
        checkMask ^= sourceBoard & broadcasted;
    }
    // knight attacks
    Bitboard knights = getEnemyKnights<isWhite>(state);
    Bitloop (knights) {
        const uint64_t sourceSquare = SquareOf(knights);
        const Bitboard sourceBoard = 1ull << sourceSquare;
        const Bitboard attacks = Lookup::knightAttacks[sourceSquare];
        const Bitboard broadcasted = broadcastSingleToMask(king & attacks);
        checkMask ^= sourceBoard & broadcasted;
    }

    // rook attacks (+queen rook attacks)
    Bitboard rooks = getEnemyRooks<isWhite>(state) | getEnemyQueens<isWhite>(state);
    Bitloop (rooks) {
        const uint64_t sourceSquare = SquareOf(rooks);
        const Bitboard sourceBoard = 1ull << sourceSquare;

        const Bitboard rookRelevantSquares = Lookup::rookAttacks[sourceSquare];
        const uint64_t rookBlockIdx = _pext_u64(enemies | friendlies, rookRelevantSquares);
        const Bitboard rookAttacks = Lookup::perSquareRookAttacks[sourceSquare][rookBlockIdx];

        // this is all ones if king is attacked otherwise all zeros
        const Bitboard broadcasted = broadcastSingleToMask(king & rookAttacks);

        // this is more complicated
        // 1. remove the attacks that don't point at the king
        const Bitboard kingRelevantSquares = Lookup::rookAttacks[kingSquare];
        const uint64_t kingBlockIdx = _pext_u64(enemies | friendlies, kingRelevantSquares);
        const Bitboard kingAttacks = Lookup::perSquareRookAttacks[kingSquare][kingBlockIdx];
        Bitboard tempCheckMap = rookAttacks & kingAttacks;
        // 2. include the rook source square
        tempCheckMap |= sourceBoard;
        // 3. reset if king is not attacked
        tempCheckMap &= broadcasted;
        // 4. remove attacked squares that don't point at the king

        checkMask ^= tempCheckMap;
    }

    // bishop attacks (+queen bishop attacks)
    Bitboard bishops = getEnemyBishops<isWhite>(state) | getEnemyQueens<isWhite>(state);
    Bitloop (bishops) {
        const uint64_t sourceSquare = SquareOf(bishops);
        const Bitboard sourceBoard = 1ull << sourceSquare;

        const Bitboard bishopRelevantSquares = Lookup::bishopAttacks[sourceSquare];
        const uint64_t bishopBlockIdx = _pext_u64(enemies | friendlies, bishopRelevantSquares);
        const Bitboard bishopAttacks = Lookup::perSquareBishopAttacks[sourceSquare][bishopBlockIdx];

        // same as for rook
        const Bitboard broadcasted = broadcastSingleToMask(king & bishopAttacks);

        // this is more complated
        // 1. remove the attacks that don't point at the king
        const Bitboard kingRelevantSquares = Lookup::bishopAttacks[kingSquare];
        const uint64_t kingBlockIdx = _pext_u64(enemies | friendlies, kingRelevantSquares);
        const Bitboard kingAttacks = Lookup::perSquareBishopAttacks[kingSquare][kingBlockIdx];
        Bitboard tempCheckMap = bishopAttacks & kingAttacks;
        // 2. include the bishop source square
        tempCheckMap |= sourceBoard;
        // 3. reset if king is not attacked
        tempCheckMap &= broadcasted;

        checkMask ^= tempCheckMap;
    }
    // here we flip and check if there is ones 
    // and set the entire map to one if so
    // then we have a mask that is all ones if there is a check
    // and all zeros if there is not
    // so we xor it with the checkmask which in the case of check
    // (all ones) is simply the inverse of all bits
    // and in the case of no check (checkMask is all ones) and 
    // mask is all zeros we just keep the checkmask
    // result is a no conditional function
    // that returns all ones if there is no check
    // and otherwise the "check path" including the attacker
    checkMask ^= broadcastSingleToMask(~checkMask);
    return checkMask;
}


template<bool isWhite>
Bitboard getPinMaskHV(GameState state) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    const Bitboard kingBoard = getKing<isWhite>(state);
    const uint64_t kingSquare = SquareOf(kingBoard);
    const Bitboard checkMask = getCheckMask<isWhite>(state);


    Bitboard pinMask = 0ull;
    Bitboard rooks = getEnemyRooks<isWhite>(state) | getEnemyQueens<isWhite>(state);
    Bitloop (rooks) {
        const uint64_t sourceSquare = SquareOf(rooks);
        const Bitboard sourceBoard = 1ull << sourceSquare;

        const Bitboard rookRelevantIndices = Lookup::rookAttacks[sourceSquare];
        const uint64_t rookBlockIdx = _pext_u64(enemies | friendlies, rookRelevantIndices);
        const Bitboard rookAttacks = Lookup::perSquareXrayRookAttacks[sourceSquare][rookBlockIdx];

        const Bitboard kingRelevantIndices = Lookup::rookAttacks[kingSquare];
        const uint64_t kingBlockIdx = _pext_u64(enemies | friendlies, kingRelevantIndices);
        // note that here we add the king to it's own attack
        // to includ it in the combined attacks below
        const Bitboard kingAttacks = Lookup::perSquareXrayRookAttacks[kingSquare][kingBlockIdx] | kingBoard;
        pinMask |= rookAttacks & kingAttacks;

        // now do the following checks
        // 1. king must be attacked by rook
        const Bitboard kingOnMask = broadcastSingleToMask(kingBoard & rookAttacks);
        // 2. no enemy can block the rook
        const Bitboard enemyOnMask = broadcastSingleToMask(pinMask & enemies);

        // 3. remove king and add attacking piece
        pinMask &= ~kingBoard;
        pinMask |= sourceBoard;

        // 4. zero out mask if king is not on the mask
        // or there is an enemy on the mask
        pinMask &= kingOnMask;
        pinMask &= ~enemyOnMask;

        // 5. if we overlap with the checkmask clear pin mask
        const Bitboard checkExists = broadcastSingleToMask(~checkMask);
        const Bitboard checkMaskConverted = checkMask & checkExists;
        const Bitboard overlap = broadcastSingleToMask(pinMask & checkMaskConverted);
        pinMask &= ~overlap;


    }
    return pinMask;
}


template<bool isWhite>
Bitboard getPinMaskDG(GameState state) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    const Bitboard kingBoard = getKing<isWhite>(state);
    const uint64_t kingSquare = SquareOf(kingBoard);
    const Bitboard checkMask = getCheckMask<isWhite>(state);


    Bitboard pinMask = 0ull;
    Bitboard bishops = getEnemyBishops<isWhite>(state) | getEnemyQueens<isWhite>(state);
    Bitloop (bishops) {
        const uint64_t sourceSquare = SquareOf(bishops);
        const Bitboard sourceBoard = 1ull << sourceSquare;

        const Bitboard bishopRelevantIndices = Lookup::bishopAttacks[sourceSquare];
        const uint64_t bishopBlockIdx = _pext_u64(enemies | friendlies, bishopRelevantIndices);
        const Bitboard bishopAttacks = Lookup::perSquareXrayBishopAttacks[sourceSquare][bishopBlockIdx];

        const Bitboard kingRelevantIndices = Lookup::bishopAttacks[kingSquare];
        const uint64_t kingBlockIdx = _pext_u64(enemies | friendlies, kingRelevantIndices);
        // note that here we add the king to it's own attack
        // to includ it in the combined attacks below
        const Bitboard kingAttacks = Lookup::perSquareXrayBishopAttacks[kingSquare][kingBlockIdx] | kingBoard;
        pinMask |= bishopAttacks & kingAttacks;

        // now do the following checks
        // 1. king must be attacked by bishop
        const Bitboard kingOnMask = broadcastSingleToMask(kingBoard & bishopAttacks);
        // 2. no enemy can block the bishop
        const Bitboard enemyOnMask = broadcastSingleToMask(pinMask & enemies);

        // 3. remove king and add attacking piece
        pinMask &= ~kingBoard;
        pinMask |= sourceBoard;

        // 4. zero out mask if king is not on the mask
        // or there is an enemy on the mask
        pinMask &= kingOnMask;
        pinMask &= ~enemyOnMask;

        // 5. if we overlap with the checkmask clear pin mask
        const Bitboard checkExists = broadcastSingleToMask(~checkMask);
        const Bitboard checkMaskConverted = checkMask & checkExists;
        const Bitboard overlap = broadcastSingleToMask(pinMask & checkMaskConverted);
        pinMask &= ~overlap;


    }
    return pinMask;
}

template<bool isWhite>
void getLegalPawnMoves(const GameState &state, std::vector<Move> &moves) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    const Bitboard pawns = getPawns<isWhite>(state);
    const Bitboard enemyOrFriendly = enemies | friendlies;
    const Bitboard pinMaskHV = getPinMaskHV<isWhite>(state);
    const Bitboard pinMaskDG = getPinMaskDG<isWhite>(state);
    const Bitboard checkMask = getCheckMask<isWhite>(state);

    Bitboard pawnsNoPromo = pawns & ~secondLastRank<isWhite>();
    Bitloop(pawnsNoPromo) {
        const uint64_t sourceSquare = SquareOf(pawnsNoPromo);
        const Bitboard sourceBoard = 1ull << sourceSquare;
        Bitboard targetSquares = 0ull;
        targetSquares |= pawnAttackLeft<isWhite>(sourceBoard & ~FILE_A) & enemies;
        targetSquares |= pawnAttackRight<isWhite>(sourceBoard & ~FILE_H) & enemies;
        targetSquares |= pawnPush1<isWhite>(sourceBoard) & ~enemyOrFriendly;
        targetSquares |= pawnPush1<isWhite>(pawnPush1<isWhite>(sourceBoard & secondRank<isWhite>()) & ~enemyOrFriendly) & ~enemyOrFriendly;

        // this handles HV pinns
        const Bitboard isPinnedHV = broadcastSingleToMask(sourceSquare & pinMaskHV);
        const Bitboard onPinHVMask = ~isPinnedHV | pinMaskHV;
        targetSquares &= onPinHVMask;

        // this handles DG pinns
        const Bitboard isPinnedDG = broadcastSingleToMask(sourceSquare & pinMaskDG);
        const Bitboard onPinDGMask = ~isPinnedDG | pinMaskDG;
        targetSquares &= onPinDGMask;

        // this handles checks
        targetSquares &= checkMask;

        Bitloop(targetSquares) {
            const uint64_t targetSquare = SquareOf(targetSquares);
            const Bitboard targetBoard = 1ull << targetSquare;
            uint64_t flags = 0ull;
            flags |= (targetBoard & pawnAttackLeft<isWhite>(sourceBoard & ~FILE_A)) >> targetSquare << 2;
            flags |= (targetBoard & pawnAttackRight<isWhite>(sourceBoard & ~FILE_H)) >> targetSquare << 2;
            flags |= (targetBoard & pawnPush2<isWhite>(sourceBoard)) >> targetSquare;
            moves.push_back(create_move(sourceSquare, targetSquare, flags));
        }
    }
    Bitboard pawnsPromo = pawns & secondLastRank<isWhite>();
    Bitloop(pawnsPromo) {
        const uint64_t sourceSquare = SquareOf(pawnsPromo);
        const Bitboard sourceBoard = 1ull << sourceSquare;
        Bitboard targetSquares = 0ull;
        targetSquares |= pawnAttackLeft<isWhite>(sourceBoard & ~FILE_A) & enemies;
        targetSquares |= pawnAttackRight<isWhite>(sourceBoard & ~FILE_H) & enemies;
        targetSquares |= pawnPush1<isWhite>(sourceBoard) & ~enemyOrFriendly;

        // this handles HV pinns
        const Bitboard isPinnedHV = broadcastSingleToMask(sourceSquare & pinMaskHV);
        const Bitboard onPinHVMask = ~isPinnedHV | pinMaskHV;
        targetSquares &= onPinHVMask;

        // this handles DG pinns
        const Bitboard isPinnedDG = broadcastSingleToMask(sourceSquare & pinMaskDG);
        const Bitboard onPinDGMask = ~isPinnedDG | pinMaskDG;
        targetSquares &= onPinDGMask;

        // this handles checks
        targetSquares &= checkMask;

        Bitloop(targetSquares) {
            const uint64_t targetSquare = SquareOf(targetSquares);
            const Bitboard targetBoard = 1ull << targetSquare;
            uint64_t flags = 0ull;
            flags |= (targetBoard & pawnAttackLeft<isWhite>(sourceBoard & ~FILE_A)) >> targetSquare << 2;
            flags |= (targetBoard & pawnAttackRight<isWhite>(sourceBoard & ~FILE_H)) >> targetSquare << 2;
            moves.push_back(create_move(sourceSquare, targetSquare, flags | 0b1000));
            moves.push_back(create_move(sourceSquare, targetSquare, flags | 0b1001));
            moves.push_back(create_move(sourceSquare, targetSquare, flags | 0b1010));
            moves.push_back(create_move(sourceSquare, targetSquare, flags | 0b1011));
        }
    }
}


template<bool isWhite>
void getLegalKnightMoves(const GameState &state, std::vector<Move> &moves) {
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard pinMaskHV = getPinMaskHV<isWhite>(state);
    const Bitboard pinMaskDG = getPinMaskDG<isWhite>(state);
    const Bitboard checkMask = getCheckMask<isWhite>(state);

    // search for knights on the board
    Bitboard knights = getKnights<isWhite>(state) & ~(pinMaskHV | pinMaskDG);
    Bitloop(knights) {
        const uint64_t sourceSquare = SquareOf(knights);
        Bitboard attackedSquares = Lookup::knightAttacks[sourceSquare] & ~friendlies;
        // for each attacked square (as) of the found knight
        attackedSquares &= checkMask;
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

template<bool isWhite>
void getLegalBishopMoves(const GameState &state, std::vector<Move> &moves) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);

    const Bitboard pinMaskHV = getPinMaskHV<isWhite>(state);
    const Bitboard pinMaskDG = getPinMaskDG<isWhite>(state);
    const Bitboard checkMask = getCheckMask<isWhite>(state);

    // hv bishops can not move
    Bitboard bishops = getBishops<isWhite>(state) & ~pinMaskHV;
    Bitloop (bishops) {
        const uint64_t sourceSquare = SquareOf(bishops);
        const Bitboard attackMask = Lookup::bishopAttacks[sourceSquare];
        const uint64_t blockIdx = _pext_u64(enemies | friendlies, attackMask);
        Bitboard targetsBoard = Lookup::perSquareBishopAttacks[sourceSquare][blockIdx];
        targetsBoard &= ~friendlies;

        // this handles diagonal pins
        const Bitboard isPinnedDG = broadcastSingleToMask(sourceSquare & pinMaskDG);
        const Bitboard onPinDGMask = ~isPinnedDG | pinMaskDG;
        targetsBoard &= onPinDGMask;

        // this handles checks
        targetsBoard &= checkMask;
        Bitloop (targetsBoard) {
            const uint64_t targetSquare = SquareOf(targetsBoard);
            const Bitboard targetBoard = 1ull << targetSquare;
            const uint64_t flags = (enemies & targetBoard) >> targetSquare << 2;
            moves.push_back(create_move(sourceSquare, targetSquare, flags));
        }
    }
}

template<bool isWhite>
void getLegalRookMoves(const GameState &state, std::vector<Move> &moves) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);

    const Bitboard pinMaskHV = getPinMaskHV<isWhite>(state);
    const Bitboard pinMaskDG = getPinMaskDG<isWhite>(state);
    const Bitboard checkMask = getCheckMask<isWhite>(state);

    // a diagonally pinned rook can never move
    Bitboard rooks = getRooks<isWhite>(state) & ~pinMaskDG;
    Bitloop (rooks) {
        const uint64_t sourceSquare = SquareOf(rooks);
        Bitboard attackMask = Lookup::rookAttacks[sourceSquare];
        uint64_t blockIdx = _pext_u64(enemies | friendlies, attackMask);
        Bitboard targetsBoard = Lookup::perSquareRookAttacks[sourceSquare][blockIdx];
        targetsBoard &= ~friendlies;

        // this handles diagonal pins
        const Bitboard isPinnedHV = broadcastSingleToMask(sourceSquare & pinMaskHV);
        const Bitboard onPinHVMask = ~isPinnedHV | pinMaskHV;
        targetsBoard &= onPinHVMask;

        // this handles checks
        targetsBoard &= checkMask;
        Bitloop (targetsBoard) {
            const uint64_t targetSquare = SquareOf(targetsBoard);
            const Bitboard targetBoard = 1ull << targetSquare;
            const uint64_t flags = (enemies & targetBoard) >> targetSquare << 2;
            moves.push_back(create_move(sourceSquare, targetSquare, flags));
        }
    }
}

template<bool isWhite>
void getLegalQueenMoves(const GameState &state, std::vector<Move> &moves) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);

    const Bitboard pinMaskHV = getPinMaskHV<isWhite>(state);
    const Bitboard pinMaskDG = getPinMaskDG<isWhite>(state);
    const Bitboard checkMask = getCheckMask<isWhite>(state);

    // rook attacks
    Bitboard queens = getQueens<isWhite>(state) & ~pinMaskDG;
    Bitloop (queens) {
        const uint64_t sourceSquare = SquareOf(queens);
        const Bitboard attackMaskRook = Lookup::rookAttacks[sourceSquare];
        const uint64_t blockIdxRook = _pext_u64(enemies | friendlies, attackMaskRook);
        Bitboard targetsBoard = Lookup::perSquareRookAttacks[sourceSquare][blockIdxRook];
        targetsBoard &= ~friendlies;

        const Bitboard isPinnedHV = broadcastSingleToMask(sourceSquare & pinMaskHV);
        const Bitboard onPinHVMask = ~isPinnedHV | pinMaskHV;
        targetsBoard &= onPinHVMask;

        // this handles checks
        targetsBoard &= checkMask;
        Bitloop (targetsBoard) {
            const uint64_t targetSquare = SquareOf(targetsBoard);
            const Bitboard targetBoard = 1ull << targetSquare;
            const uint64_t flags = (enemies & targetBoard) >> targetSquare << 2;
            moves.push_back(create_move(sourceSquare, targetSquare, flags));
        }
    }
    // bishop attacks
    queens = getQueens<isWhite>(state) & ~pinMaskHV;
    Bitloop (queens) {
        const uint64_t sourceSquare = SquareOf(queens);
        const Bitboard attackMaskBishop = Lookup::bishopAttacks[sourceSquare];
        const uint64_t blockIdxBishop = _pext_u64(enemies | friendlies, attackMaskBishop);
        Bitboard targetsBoard = Lookup::perSquareBishopAttacks[sourceSquare][blockIdxBishop];
        targetsBoard &= ~friendlies;

        const Bitboard isPinnedDG = broadcastSingleToMask(sourceSquare & pinMaskDG);
        const Bitboard onPinDGMask = ~isPinnedDG | pinMaskDG;
        targetsBoard &= onPinDGMask;

        // this handles checks
        targetsBoard &= checkMask;
        Bitloop (targetsBoard) {
            const uint64_t targetSquare = SquareOf(targetsBoard);
            const Bitboard targetBoard = 1ull << targetSquare;
            const uint64_t flags = (enemies & targetBoard) >> targetSquare << 2;
            moves.push_back(create_move(sourceSquare, targetSquare, flags));
        }
    }
}

template<bool isWhite>
Bitboard getKingAttacks(const GameState &state) {
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
Bitboard getSeenSquares(const GameState &state) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    const Bitboard enemyKing = getEnemyKing<isWhite>(state);
    const Bitboard blockingPieces = friendlies | enemies & ~enemyKing;

    const Bitboard pawns = getPawns<isWhite>(state);
    Bitboard knights = getKnights<isWhite>(state);
    Bitboard rooks = getRooks<isWhite>(state) | getQueens<isWhite>(state);
    Bitboard bishops = getBishops<isWhite>(state) | getQueens<isWhite>(state);

    Bitboard seenSquares = 0ull;

    // pawns
    seenSquares |= pawnAttackLeft<isWhite>(pawns & ~FILE_A);
    seenSquares |= pawnAttackRight<isWhite>(pawns & ~FILE_H);

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
void getLegalKingMoves(const GameState &state, std::vector<Move> &moves) {
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
template<bool isWhite, bool kingSide>
void getLegalCastleMoves(const GameState &state, Moves &moves) {
    if constexpr (isWhite && kingSide) {
        moves.push_back(create_move(4ull, 7ull, 0b0010));
    } else if constexpr (isWhite && !kingSide) {
        moves.push_back(create_move(4ull, 0ull, 0b0011));
    } else if constexpr (!isWhite && kingSide) {
        moves.push_back(create_move(60ull, 63ull, 0b0010));
    } else {
        moves.push_back(create_move(60ull, 56ull, 0b0011));
    }
}

template<bool isWhite>
std::vector<Move> getLegalMoves(const GameState &state) {
    std::vector<Move> moves;
    getLegalPawnMoves<isWhite>(state, moves);
    getLegalKnightMoves<isWhite>(state, moves);
    getLegalRookMoves<isWhite>(state, moves);
    getLegalBishopMoves<isWhite>(state, moves);
    getLegalQueenMoves<isWhite>(state, moves);
    getLegalKingMoves<isWhite>(state, moves);
    // getLegalCastleMoves<isWhite, state.
    return moves;
}















