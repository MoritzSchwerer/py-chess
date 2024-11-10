#pragma once
#include <exception>
#include <vector>

#include "constants.hpp"
#include "game_state.hpp"
#include "lookup.hpp"
#include "moves.hpp"
#include "types.hpp"
#include "utils.hpp"

namespace Movegen {
Move create_move(Bitboard from, Bitboard to, uint64_t flags) {
    return (from & 0x3f) | ((to & 0x3f) << 6) | ((flags & 0xf) << 12);
}

template <bool isWhite>
Bitboard getCheckMask(GameState state) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    const Bitboard king = getKing<isWhite>(state);
    const uint64_t kingSquare = SquareOf(king);

    Bitboard checkMask = 0xffffffffffffffff;

    // pawn attacks
    Bitboard pawns = getEnemyPawns<isWhite>(state);
    Bitloop(pawns) {
        const uint64_t sourceSquare = SquareOf(pawns);
        const Bitboard sourceBoard = 1ull << sourceSquare;
        const Bitboard attacks =
            pawnAttackLeft<!isWhite>(sourceBoard & ~FILE_A) |
            pawnAttackRight<!isWhite>(sourceBoard & ~FILE_H);
        const Bitboard broadcasted = broadcastSingleToMask(king & attacks);
        checkMask ^= sourceBoard & broadcasted;
    }
    // knight attacks
    Bitboard knights = getEnemyKnights<isWhite>(state);
    Bitloop(knights) {
        const uint64_t sourceSquare = SquareOf(knights);
        const Bitboard sourceBoard = 1ull << sourceSquare;
        const Bitboard attacks = Lookup::knightAttacks[sourceSquare];
        const Bitboard broadcasted = broadcastSingleToMask(king & attacks);
        checkMask ^= sourceBoard & broadcasted;
    }

    // rook attacks (+queen rook attacks)
    Bitboard rooks =
        getEnemyRooks<isWhite>(state) | getEnemyQueens<isWhite>(state);
    Bitloop(rooks) {
        const uint64_t sourceSquare = SquareOf(rooks);
        const Bitboard sourceBoard = 1ull << sourceSquare;

        const Bitboard rookRelevantSquares = Lookup::rookAttacks[sourceSquare];
        const uint64_t rookBlockIdx =
            _pext_u64(enemies | friendlies, rookRelevantSquares);
        const Bitboard rookAttacks =
            Lookup::perSquareRookAttacks[sourceSquare][rookBlockIdx];

        // this is all ones if king is attacked otherwise all zeros
        const Bitboard broadcasted = broadcastSingleToMask(king & rookAttacks);

        // this is more complicated
        // 1. remove the attacks that don't point at the king
        const Bitboard kingRelevantSquares = Lookup::rookAttacks[kingSquare];
        const uint64_t kingBlockIdx =
            _pext_u64(enemies | friendlies, kingRelevantSquares);
        const Bitboard kingAttacks =
            Lookup::perSquareRookAttacks[kingSquare][kingBlockIdx];
        Bitboard tempCheckMap = rookAttacks & kingAttacks;
        // 2. include the rook source square
        tempCheckMap |= sourceBoard;
        // 3. reset if king is not attacked
        tempCheckMap &= broadcasted;
        // 4. remove attacked squares that don't point at the king

        checkMask ^= tempCheckMap;
    }

    // bishop attacks (+queen bishop attacks)
    Bitboard bishops =
        getEnemyBishops<isWhite>(state) | getEnemyQueens<isWhite>(state);
    Bitloop(bishops) {
        const uint64_t sourceSquare = SquareOf(bishops);
        const Bitboard sourceBoard = 1ull << sourceSquare;

        const Bitboard bishopRelevantSquares =
            Lookup::bishopAttacks[sourceSquare];
        const uint64_t bishopBlockIdx =
            _pext_u64(enemies | friendlies, bishopRelevantSquares);
        const Bitboard bishopAttacks =
            Lookup::perSquareBishopAttacks[sourceSquare][bishopBlockIdx];

        // same as for rook
        const Bitboard broadcasted =
            broadcastSingleToMask(king & bishopAttacks);

        // this is more complated
        // 1. remove the attacks that don't point at the king
        const Bitboard kingRelevantSquares = Lookup::bishopAttacks[kingSquare];
        const uint64_t kingBlockIdx =
            _pext_u64(enemies | friendlies, kingRelevantSquares);
        const Bitboard kingAttacks =
            Lookup::perSquareBishopAttacks[kingSquare][kingBlockIdx];
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

template <bool isWhite>
Bitboard getPinMaskHV(GameState state) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    const Bitboard kingBoard = getKing<isWhite>(state);
    const uint64_t kingSquare = SquareOf(kingBoard);
    const Bitboard checkMask = getCheckMask<isWhite>(state);

    Bitboard pinMask = 0ull;
    Bitboard rooks =
        getEnemyRooks<isWhite>(state) | getEnemyQueens<isWhite>(state);
    Bitloop(rooks) {
        const uint64_t sourceSquare = SquareOf(rooks);
        const Bitboard sourceBoard = 1ull << sourceSquare;

        const Bitboard rookRelevantIndices = Lookup::rookAttacks[sourceSquare];
        const uint64_t rookBlockIdx =
            _pext_u64(enemies | friendlies, rookRelevantIndices);
        const Bitboard rookAttacks =
            Lookup::perSquareXrayRookAttacks[sourceSquare][rookBlockIdx];

        const Bitboard kingRelevantIndices = Lookup::rookAttacks[kingSquare];
        const uint64_t kingBlockIdx =
            _pext_u64(enemies | friendlies, kingRelevantIndices);
        // note that here we add the king to it's own attack
        // to includ it in the combined attacks below
        const Bitboard kingAttacks =
            Lookup::perSquareXrayRookAttacks[kingSquare][kingBlockIdx] |
            kingBoard;
        Bitboard tempPinMask = 0ull;
        tempPinMask |= rookAttacks & kingAttacks;

        // now do the following checks
        // 1. king must be attacked by rook
        const Bitboard kingOnMask =
            broadcastSingleToMask(kingBoard & rookAttacks);
        // 2. no enemy can block the rook
        const Bitboard enemyOnMask =
            broadcastSingleToMask(tempPinMask & enemies);

        // 3. remove king and add attacking piece
        tempPinMask &= ~kingBoard;
        tempPinMask |= sourceBoard;

        // 4. zero out mask if king is not on the mask
        // or there is an enemy on the mask
        tempPinMask &= kingOnMask;
        tempPinMask &= ~enemyOnMask;

        // 5. if we overlap with the checkmask clear pin mask
        const Bitboard checkExists = broadcastSingleToMask(~checkMask);
        const Bitboard checkMaskConverted = checkMask & checkExists;
        const Bitboard overlap =
            broadcastSingleToMask(tempPinMask & checkMaskConverted);
        tempPinMask &= ~overlap;
        pinMask |= tempPinMask;
    }
    return pinMask;
}

template <bool isWhite>
Bitboard getPinMaskDG(GameState state) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    const Bitboard kingBoard = getKing<isWhite>(state);
    const uint64_t kingSquare = SquareOf(kingBoard);
    const Bitboard checkMask = getCheckMask<isWhite>(state);

    Bitboard pinMask = 0ull;
    Bitboard bishops =
        getEnemyBishops<isWhite>(state) | getEnemyQueens<isWhite>(state);
    Bitloop(bishops) {
        const uint64_t sourceSquare = SquareOf(bishops);
        const Bitboard sourceBoard = 1ull << sourceSquare;

        const Bitboard bishopRelevantIndices =
            Lookup::bishopAttacks[sourceSquare];
        const uint64_t bishopBlockIdx =
            _pext_u64(enemies | friendlies, bishopRelevantIndices);
        const Bitboard bishopAttacks =
            Lookup::perSquareXrayBishopAttacks[sourceSquare][bishopBlockIdx];

        const Bitboard kingRelevantIndices = Lookup::bishopAttacks[kingSquare];
        const uint64_t kingBlockIdx =
            _pext_u64(enemies | friendlies, kingRelevantIndices);
        // note that here we add the king to it's own attack
        // to includ it in the combined attacks below
        const Bitboard kingAttacks =
            Lookup::perSquareXrayBishopAttacks[kingSquare][kingBlockIdx] |
            kingBoard;
        Bitboard tempPinMask = 0ull;
        tempPinMask |= bishopAttacks & kingAttacks;

        // now do the following checks
        // 1. king must be attacked by bishop
        const Bitboard kingOnMask =
            broadcastSingleToMask(kingBoard & bishopAttacks);
        // 2. no enemy can block the bishop
        const Bitboard enemyOnMask =
            broadcastSingleToMask(tempPinMask & enemies);

        // 3. remove king and add attacking piece
        tempPinMask &= ~kingBoard;
        tempPinMask |= sourceBoard;

        // 4. zero out mask if king is not on the mask
        // or there is an enemy on the mask
        tempPinMask &= kingOnMask;
        tempPinMask &= ~enemyOnMask;

        // 5. if we overlap with the checkmask clear pin mask
        const Bitboard checkExists = broadcastSingleToMask(~checkMask);
        const Bitboard checkMaskConverted = checkMask & checkExists;
        const Bitboard overlap =
            broadcastSingleToMask(tempPinMask & checkMaskConverted);
        tempPinMask &= ~overlap;

        pinMask |= tempPinMask;
    }
    return pinMask;
}

template <bool isWhite>
void getLegalPawnMoves(const GameState &state, Bitboard checkMask,
                       Bitboard pinMaskHV, Bitboard pinMaskDG, Moves &moves) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    const Bitboard pawns = getPawns<isWhite>(state);
    const Bitboard enemyOrFriendly = enemies | friendlies;

    Bitboard pawnsNoPromo = pawns & ~secondLastRank<isWhite>();
    Bitloop(pawnsNoPromo) {
        const uint64_t sourceSquare = SquareOf(pawnsNoPromo);
        const Bitboard sourceBoard = 1ull << sourceSquare;
        Bitboard targetSquares = 0ull;
        targetSquares |=
            pawnAttackLeft<isWhite>(sourceBoard & ~FILE_A) & enemies;
        targetSquares |=
            pawnAttackRight<isWhite>(sourceBoard & ~FILE_H) & enemies;
        targetSquares |= pawnPush1<isWhite>(sourceBoard) & ~enemyOrFriendly;
        targetSquares |=
            pawnPush1<isWhite>(
                pawnPush1<isWhite>(sourceBoard & secondRank<isWhite>()) &
                ~enemyOrFriendly) &
            ~enemyOrFriendly;

        // this handles HV pinns
        const Bitboard isPinnedHV =
            broadcastSingleToMask(sourceBoard & pinMaskHV);
        const Bitboard onPinHVMask = ~isPinnedHV | pinMaskHV;
        targetSquares &= onPinHVMask;

        // this handles DG pinns
        const Bitboard isPinnedDG =
            broadcastSingleToMask(sourceBoard & pinMaskDG);
        const Bitboard onPinDGMask = ~isPinnedDG | pinMaskDG;
        targetSquares &= onPinDGMask;

        // this handles checks
        targetSquares &= checkMask;

        Bitloop(targetSquares) {
            const uint64_t targetSquare = SquareOf(targetSquares);
            const Bitboard targetBoard = 1ull << targetSquare;
            uint64_t flags = 0ull;
            flags |= (targetBoard &
                      pawnAttackLeft<isWhite>(sourceBoard & ~FILE_A)) >>
                     targetSquare << 2;
            flags |= (targetBoard &
                      pawnAttackRight<isWhite>(sourceBoard & ~FILE_H)) >>
                     targetSquare << 2;
            flags |=
                (targetBoard & pawnPush2<isWhite>(sourceBoard)) >> targetSquare;
            moves.push_back(create_move(sourceSquare, targetSquare, flags));
        }
    }
    Bitboard pawnsPromo = pawns & secondLastRank<isWhite>();
    Bitloop(pawnsPromo) {
        const uint64_t sourceSquare = SquareOf(pawnsPromo);
        const Bitboard sourceBoard = 1ull << sourceSquare;
        Bitboard targetSquares = 0ull;
        targetSquares |=
            pawnAttackLeft<isWhite>(sourceBoard & ~FILE_A) & enemies;
        targetSquares |=
            pawnAttackRight<isWhite>(sourceBoard & ~FILE_H) & enemies;
        targetSquares |= pawnPush1<isWhite>(sourceBoard) & ~enemyOrFriendly;

        // this handles HV pinns
        const Bitboard isPinnedHV =
            broadcastSingleToMask(sourceBoard & pinMaskHV);
        const Bitboard onPinHVMask = ~isPinnedHV | pinMaskHV;
        targetSquares &= onPinHVMask;

        // this handles DG pinns
        const Bitboard isPinnedDG =
            broadcastSingleToMask(sourceBoard & pinMaskDG);
        const Bitboard onPinDGMask = ~isPinnedDG | pinMaskDG;
        targetSquares &= onPinDGMask;

        // this handles checks
        targetSquares &= checkMask;

        Bitloop(targetSquares) {
            const uint64_t targetSquare = SquareOf(targetSquares);
            const Bitboard targetBoard = 1ull << targetSquare;
            uint64_t flags = 0ull;
            flags |= (targetBoard &
                      pawnAttackLeft<isWhite>(sourceBoard & ~FILE_A)) >>
                     targetSquare << 2;
            flags |= (targetBoard &
                      pawnAttackRight<isWhite>(sourceBoard & ~FILE_H)) >>
                     targetSquare << 2;
            moves.push_back(
                create_move(sourceSquare, targetSquare, flags | 0b1000));
            moves.push_back(
                create_move(sourceSquare, targetSquare, flags | 0b1001));
            moves.push_back(
                create_move(sourceSquare, targetSquare, flags | 0b1010));
            moves.push_back(
                create_move(sourceSquare, targetSquare, flags | 0b1011));
        }
    }
}

template <bool isWhite>
void getLegalKnightMoves(const GameState &state, Bitboard checkMask,
                         Bitboard pinMaskHV, Bitboard pinMaskDG, Moves &moves) {
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    const Bitboard enemies = getEnemyPieces<isWhite>(state);

    // search for knights on the board
    Bitboard knights = getKnights<isWhite>(state) & ~(pinMaskHV | pinMaskDG);
    Bitloop(knights) {
        const uint64_t sourceSquare = SquareOf(knights);
        Bitboard attackedSquares =
            Lookup::knightAttacks[sourceSquare] & ~friendlies;
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

template <bool isWhite>
void getLegalBishopMoves(const GameState &state, Bitboard checkMask,
                         Bitboard pinMaskHV, Bitboard pinMaskDG, Moves &moves) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);

    // hv bishops can not move
    Bitboard bishops = getBishops<isWhite>(state) & ~pinMaskHV;
    Bitloop(bishops) {
        const uint64_t sourceSquare = SquareOf(bishops);
        const Bitboard sourceBoard = 1ull << sourceSquare;
        const Bitboard attackMask = Lookup::bishopAttacks[sourceSquare];
        const uint64_t blockIdx = _pext_u64(enemies | friendlies, attackMask);
        Bitboard targetsBoard =
            Lookup::perSquareBishopAttacks[sourceSquare][blockIdx];
        targetsBoard &= ~friendlies;

        // this handles diagonal pins
        const Bitboard isPinnedDG =
            broadcastSingleToMask(sourceBoard & pinMaskDG);
        const Bitboard onPinDGMask = ~isPinnedDG | pinMaskDG;
        targetsBoard &= onPinDGMask;

        // this handles checks
        targetsBoard &= checkMask;
        Bitloop(targetsBoard) {
            const uint64_t targetSquare = SquareOf(targetsBoard);
            const Bitboard targetBoard = 1ull << targetSquare;
            const uint64_t flags = (enemies & targetBoard) >> targetSquare << 2;
            moves.push_back(create_move(sourceSquare, targetSquare, flags));
        }
    }
}

template <bool isWhite>
void getLegalRookMoves(const GameState &state, Bitboard checkMask,
                       Bitboard pinMaskHV, Bitboard pinMaskDG, Moves &moves) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);

    // a diagonally pinned rook can never move
    Bitboard rooks = getRooks<isWhite>(state) & ~pinMaskDG;
    Bitloop(rooks) {
        const uint64_t sourceSquare = SquareOf(rooks);
        const Bitboard sourceBoard = 1ull << sourceSquare;
        Bitboard attackMask = Lookup::rookAttacks[sourceSquare];
        uint64_t blockIdx = _pext_u64(enemies | friendlies, attackMask);
        Bitboard targetsBoard =
            Lookup::perSquareRookAttacks[sourceSquare][blockIdx];
        targetsBoard &= ~friendlies;

        // this handles diagonal pins
        const Bitboard isPinnedHV =
            broadcastSingleToMask(sourceBoard & pinMaskHV);
        const Bitboard onPinHVMask = ~isPinnedHV | pinMaskHV;
        targetsBoard &= onPinHVMask;

        // this handles checks
        targetsBoard &= checkMask;
        Bitloop(targetsBoard) {
            const uint64_t targetSquare = SquareOf(targetsBoard);
            const Bitboard targetBoard = 1ull << targetSquare;
            const uint64_t flags = (enemies & targetBoard) >> targetSquare << 2;
            moves.push_back(create_move(sourceSquare, targetSquare, flags));
        }
    }
}

template <bool isWhite>
void getLegalQueenMoves(const GameState &state, Bitboard checkMask,
                        Bitboard pinMaskHV, Bitboard pinMaskDG, Moves &moves) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);

    // rook attacks
    Bitboard queens = getQueens<isWhite>(state) & ~pinMaskDG;
    Bitloop(queens) {
        const uint64_t sourceSquare = SquareOf(queens);
        const Bitboard sourceBoard = 1ull << sourceSquare;
        const Bitboard attackMaskRook = Lookup::rookAttacks[sourceSquare];
        const uint64_t blockIdxRook =
            _pext_u64(enemies | friendlies, attackMaskRook);
        Bitboard targetsBoard =
            Lookup::perSquareRookAttacks[sourceSquare][blockIdxRook];
        targetsBoard &= ~friendlies;

        const Bitboard isPinnedHV =
            broadcastSingleToMask(sourceBoard & pinMaskHV);
        const Bitboard onPinHVMask = ~isPinnedHV | pinMaskHV;
        targetsBoard &= onPinHVMask;

        // this handles checks
        targetsBoard &= checkMask;
        Bitloop(targetsBoard) {
            const uint64_t targetSquare = SquareOf(targetsBoard);
            const Bitboard targetBoard = 1ull << targetSquare;
            const uint64_t flags = (enemies & targetBoard) >> targetSquare << 2;
            moves.push_back(create_move(sourceSquare, targetSquare, flags));
        }
    }
    // bishop attacks
    queens = getQueens<isWhite>(state) & ~pinMaskHV;
    Bitloop(queens) {
        const uint64_t sourceSquare = SquareOf(queens);
        const Bitboard sourceBoard = 1ull << sourceSquare;
        const Bitboard attackMaskBishop = Lookup::bishopAttacks[sourceSquare];
        const uint64_t blockIdxBishop =
            _pext_u64(enemies | friendlies, attackMaskBishop);
        Bitboard targetsBoard =
            Lookup::perSquareBishopAttacks[sourceSquare][blockIdxBishop];

        targetsBoard &= ~friendlies;

        const Bitboard isPinnedDG =
            broadcastSingleToMask(sourceBoard & pinMaskDG);
        const Bitboard onPinDGMask = ~isPinnedDG | pinMaskDG;
        targetsBoard &= onPinDGMask;

        // this handles checks
        targetsBoard &= checkMask;
        Bitloop(targetsBoard) {
            const uint64_t targetSquare = SquareOf(targetsBoard);
            const Bitboard targetBoard = 1ull << targetSquare;
            const uint64_t flags = (enemies & targetBoard) >> targetSquare << 2;
            moves.push_back(create_move(sourceSquare, targetSquare, flags));
        }
    }
}

template <bool isWhite>
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

template <bool isWhite>
Bitboard getSeenSquares(const GameState &state) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    const Bitboard enemyKing = getEnemyKing<isWhite>(state);
    const Bitboard blockingPieces = friendlies | (enemies & ~enemyKing);

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
        const Bitboard targetsBoard =
            Lookup::perSquareRookAttacks[sourceSquare][blockIdx];
        seenSquares |= targetsBoard;
    }

    // bishops (+ queen)
    Bitloop(bishops) {
        const uint64_t sourceSquare = SquareOf(bishops);
        const Bitboard relevantMask = Lookup::bishopAttacks[sourceSquare];
        const uint64_t blockIdx = _pext_u64(blockingPieces, relevantMask);
        const Bitboard targetsBoard =
            Lookup::perSquareBishopAttacks[sourceSquare][blockIdx];
        seenSquares |= targetsBoard;
    }

    seenSquares |= getKingAttacks<isWhite>(state);

    return seenSquares;
}

template <bool isWhite>
void getLegalKingMoves(const GameState &state, Bitboard enemySeenSquares,
                       Moves &moves) {
    const uint64_t kingSquare = SquareOf(getKing<isWhite>(state));
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    const Bitboard attackedSquares = getKingAttacks<isWhite>(state);

    Bitboard validTargets = attackedSquares & ~enemySeenSquares & ~friendlies;
    Bitloop(validTargets) {
        const uint64_t targetSquare = SquareOf(validTargets);
        const Bitboard targetBoard = 1ull << targetSquare;
        const uint64_t flags = (enemies & targetBoard) >> targetSquare << 2;
        moves.push_back(create_move(kingSquare, targetSquare, flags));
    }
}
template <GameStatus status>
void getLegalCastleMoves(const GameState &state, Bitboard seenSquares,
                         Bitboard checkMask, Moves &moves) {
    const Bitboard enemies = getEnemyPieces<status.isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<status.isWhite>(state);
    if (SquareOf(~checkMask) < 64) {
        return;
    }
    if constexpr (status.isWhite && status.wQueenC) {
        const Bitboard relevantPieceSquares = 0b00001110;
        const Bitboard relevantSeenSquares = 0b00001100;
        if (!((relevantSeenSquares & seenSquares) |
              (relevantPieceSquares & friendlies) |
              (relevantPieceSquares & enemies))) {
            moves.push_back(create_move(4ull, 1ull, 0b0011));
        }
    }
    if constexpr (status.isWhite && status.wKingC) {
        const Bitboard relevantPieceSquares = 0b01100000;
        const Bitboard relevantSeenSquares = 0b01100000;
        if (!((relevantSeenSquares & seenSquares) |
              (relevantPieceSquares & friendlies) |
              (relevantPieceSquares & enemies))) {
            moves.push_back(create_move(4ull, 6ull, 0b0010));
        }
    }
    if constexpr (!status.isWhite && status.bQueenC) {
        const Bitboard relevantPieceSquares = 0b00001110ull << 56;
        const Bitboard relevantSeenSquares = 0b00001100ull << 56;
        if (!((relevantSeenSquares & seenSquares) |
              (relevantPieceSquares & friendlies) |
              (relevantPieceSquares & enemies))) {
            moves.push_back(create_move(60ull, 57ull, 0b0011));
        }
    }
    if constexpr (!status.isWhite && status.bKingC) {
        const Bitboard relevantPieceSquares = 0b01100000ull << 56;
        const Bitboard relevantSeenSquares = 0b01100000ull << 56;
        if (!((relevantSeenSquares & seenSquares) |
              (relevantPieceSquares & friendlies) |
              (relevantPieceSquares & enemies))) {
            moves.push_back(create_move(60ull, 62ull, 0b0010));
        }
    }
}

template <bool isWhite>
Bitboard getEnemyBishopSeenSquaresAfterEnpassant(const GameState &state,
                                                 uint64_t attackingPawnSquare) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    const Bitboard enpassantSquare = state.enpassant_board;
    const Bitboard pawnSquare = pawnPush1<!isWhite>(enpassantSquare);

    const Bitboard blockingPieces = (enemies | friendlies | enpassantSquare) &
                                    ~pawnSquare &
                                    ~(1ull << attackingPawnSquare);

    Bitboard bishops = getBishops<!isWhite>(state) | getQueens<!isWhite>(state);
    Bitboard seenSquares = 0ull;
    Bitloop(bishops) {
        const uint64_t sourceSquare = SquareOf(bishops);
        const Bitboard relevantMask = Lookup::bishopAttacks[sourceSquare];
        const uint64_t blockIdx = _pext_u64(blockingPieces, relevantMask);
        const Bitboard targetsBoard =
            Lookup::perSquareBishopAttacks[sourceSquare][blockIdx];
        seenSquares |= targetsBoard;
    }
    return seenSquares;
}

template <bool isWhite>
Bitboard getEnemyRookSeenSquaresAfterEnpassant(const GameState &state,
                                               uint64_t attackingPawnSquare) {
    const Bitboard enemies = getEnemyPieces<isWhite>(state);
    const Bitboard friendlies = getFriendlyPieces<isWhite>(state);
    const Bitboard enpassantSquare = state.enpassant_board;
    const Bitboard pawnSquare = pawnPush1<!isWhite>(enpassantSquare);

    const Bitboard blockingPieces = (enemies | friendlies | enpassantSquare) &
                                    ~pawnSquare &
                                    ~(1ull << attackingPawnSquare);

    Bitboard rooks = getRooks<!isWhite>(state) | getQueens<!isWhite>(state);
    Bitboard seenSquares = 0ull;
    Bitloop(rooks) {
        const uint64_t sourceSquare = SquareOf(rooks);
        const Bitboard relevantMask = Lookup::rookAttacks[sourceSquare];
        const uint64_t blockIdx = _pext_u64(blockingPieces, relevantMask);
        const Bitboard targetsBoard =
            Lookup::perSquareRookAttacks[sourceSquare][blockIdx];
        seenSquares |= targetsBoard;
    }
    return seenSquares;
}

template <bool isWhite>
void getLegalEnpassantCaptures(const GameState &state, Moves &moves) {
    const Bitboard king = getKing<isWhite>(state);
    const Bitboard enemyEnpassant = state.enpassant_board;
    const Bitboard pawns = getPawns<isWhite>(state);
    const Bitboard leftAttack =
        pawnAttackLeft<!isWhite>(enemyEnpassant) & ~FILE_H;
    const Bitboard rightAttack =
        pawnAttackRight<!isWhite>(enemyEnpassant) & ~FILE_A;
    const Bitboard attackingSquares = leftAttack | rightAttack;
    Bitboard attackingPawns = attackingSquares & pawns;
    Bitloop(attackingPawns) {
        const uint64_t sourceSquare = SquareOf(attackingPawns);
        const Bitboard bishopSeenSquares =
            getEnemyBishopSeenSquaresAfterEnpassant<isWhite>(state,
                                                             sourceSquare);
        const Bitboard rookSeenSquares =
            getEnemyRookSeenSquaresAfterEnpassant<isWhite>(state, sourceSquare);
        const Bitboard seenSquares = bishopSeenSquares | rookSeenSquares;
        if (!(seenSquares & king)) {
            moves.push_back(
                create_move(sourceSquare, SquareOf(enemyEnpassant), 0b0101));
        }
    }
}

template <GameStatus status>
Moves getLegalMovesTemplate(const GameState &state) {
    const Bitboard checkMask = getCheckMask<status.isWhite>(state);
    const Bitboard pinMaskHV = getPinMaskHV<status.isWhite>(state);
    const Bitboard pinMaskDG = getPinMaskDG<status.isWhite>(state);
    const Bitboard enemySeenSquares = getSeenSquares<!status.isWhite>(state);

    Moves moves;
    getLegalPawnMoves<status.isWhite>(state, checkMask, pinMaskHV, pinMaskDG,
                                      moves);
    getLegalKnightMoves<status.isWhite>(state, checkMask, pinMaskHV, pinMaskDG,
                                        moves);
    getLegalRookMoves<status.isWhite>(state, checkMask, pinMaskHV, pinMaskDG,
                                      moves);
    getLegalBishopMoves<status.isWhite>(state, checkMask, pinMaskHV, pinMaskDG,
                                        moves);
    getLegalQueenMoves<status.isWhite>(state, checkMask, pinMaskHV, pinMaskDG,
                                       moves);
    getLegalKingMoves<status.isWhite>(state, enemySeenSquares, moves);
    if constexpr (status.wKingC || status.wQueenC || status.bKingC ||
                  status.bQueenC) {
        getLegalCastleMoves<status>(state, enemySeenSquares, checkMask, moves);
    }
    if constexpr (status.enpassant) {
        getLegalEnpassantCaptures<status.isWhite>(state, moves);
    }
    return moves;
}

Moves getLegalMoves(const GameState &state) {
    switch (state.status.getStatusPattern()) {
        case 0b000000:
            return getLegalMovesTemplate<GameStatus(0b000000ull)>(state);
        case 0b000001:
            return getLegalMovesTemplate<GameStatus(0b000001ull)>(state);
        case 0b000010:
            return getLegalMovesTemplate<GameStatus(0b000010ull)>(state);
        case 0b000011:
            return getLegalMovesTemplate<GameStatus(0b000011ull)>(state);
        case 0b000100:
            return getLegalMovesTemplate<GameStatus(0b000100ull)>(state);
        case 0b000101:
            return getLegalMovesTemplate<GameStatus(0b000101ull)>(state);
        case 0b000110:
            return getLegalMovesTemplate<GameStatus(0b000110ull)>(state);
        case 0b000111:
            return getLegalMovesTemplate<GameStatus(0b000111ull)>(state);
        case 0b001000:
            return getLegalMovesTemplate<GameStatus(0b001000ull)>(state);
        case 0b001001:
            return getLegalMovesTemplate<GameStatus(0b001001ull)>(state);
        case 0b001010:
            return getLegalMovesTemplate<GameStatus(0b001010ull)>(state);
        case 0b001011:
            return getLegalMovesTemplate<GameStatus(0b001011ull)>(state);
        case 0b001100:
            return getLegalMovesTemplate<GameStatus(0b001100ull)>(state);
        case 0b001101:
            return getLegalMovesTemplate<GameStatus(0b001101ull)>(state);
        case 0b001110:
            return getLegalMovesTemplate<GameStatus(0b001110ull)>(state);
        case 0b001111:
            return getLegalMovesTemplate<GameStatus(0b001111ull)>(state);
        case 0b010000:
            return getLegalMovesTemplate<GameStatus(0b010000ull)>(state);
        case 0b010001:
            return getLegalMovesTemplate<GameStatus(0b010001ull)>(state);
        case 0b010010:
            return getLegalMovesTemplate<GameStatus(0b010010ull)>(state);
        case 0b010011:
            return getLegalMovesTemplate<GameStatus(0b010011ull)>(state);
        case 0b010100:
            return getLegalMovesTemplate<GameStatus(0b010100ull)>(state);
        case 0b010101:
            return getLegalMovesTemplate<GameStatus(0b010101ull)>(state);
        case 0b010110:
            return getLegalMovesTemplate<GameStatus(0b010110ull)>(state);
        case 0b010111:
            return getLegalMovesTemplate<GameStatus(0b010111ull)>(state);
        case 0b011000:
            return getLegalMovesTemplate<GameStatus(0b011000ull)>(state);
        case 0b011001:
            return getLegalMovesTemplate<GameStatus(0b011001ull)>(state);
        case 0b011010:
            return getLegalMovesTemplate<GameStatus(0b011010ull)>(state);
        case 0b011011:
            return getLegalMovesTemplate<GameStatus(0b011011ull)>(state);
        case 0b011100:
            return getLegalMovesTemplate<GameStatus(0b011100ull)>(state);
        case 0b011101:
            return getLegalMovesTemplate<GameStatus(0b011101ull)>(state);
        case 0b011110:
            return getLegalMovesTemplate<GameStatus(0b011110ull)>(state);
        case 0b011111:
            return getLegalMovesTemplate<GameStatus(0b011111ull)>(state);
        case 0b100000:
            return getLegalMovesTemplate<GameStatus(0b100000ull)>(state);
        case 0b100001:
            return getLegalMovesTemplate<GameStatus(0b100001ull)>(state);
        case 0b100010:
            return getLegalMovesTemplate<GameStatus(0b100010ull)>(state);
        case 0b100011:
            return getLegalMovesTemplate<GameStatus(0b100011ull)>(state);
        case 0b100100:
            return getLegalMovesTemplate<GameStatus(0b100100ull)>(state);
        case 0b100101:
            return getLegalMovesTemplate<GameStatus(0b100101ull)>(state);
        case 0b100110:
            return getLegalMovesTemplate<GameStatus(0b100110ull)>(state);
        case 0b100111:
            return getLegalMovesTemplate<GameStatus(0b100111ull)>(state);
        case 0b101000:
            return getLegalMovesTemplate<GameStatus(0b101000ull)>(state);
        case 0b101001:
            return getLegalMovesTemplate<GameStatus(0b101001ull)>(state);
        case 0b101010:
            return getLegalMovesTemplate<GameStatus(0b101010ull)>(state);
        case 0b101011:
            return getLegalMovesTemplate<GameStatus(0b101011ull)>(state);
        case 0b101100:
            return getLegalMovesTemplate<GameStatus(0b101100ull)>(state);
        case 0b101101:
            return getLegalMovesTemplate<GameStatus(0b101101ull)>(state);
        case 0b101110:
            return getLegalMovesTemplate<GameStatus(0b101110ull)>(state);
        case 0b101111:
            return getLegalMovesTemplate<GameStatus(0b101111ull)>(state);
        case 0b110000:
            return getLegalMovesTemplate<GameStatus(0b110000ull)>(state);
        case 0b110001:
            return getLegalMovesTemplate<GameStatus(0b110001ull)>(state);
        case 0b110010:
            return getLegalMovesTemplate<GameStatus(0b110010ull)>(state);
        case 0b110011:
            return getLegalMovesTemplate<GameStatus(0b110011ull)>(state);
        case 0b110100:
            return getLegalMovesTemplate<GameStatus(0b110100ull)>(state);
        case 0b110101:
            return getLegalMovesTemplate<GameStatus(0b110101ull)>(state);
        case 0b110110:
            return getLegalMovesTemplate<GameStatus(0b110110ull)>(state);
        case 0b110111:
            return getLegalMovesTemplate<GameStatus(0b110111ull)>(state);
        case 0b111000:
            return getLegalMovesTemplate<GameStatus(0b111000ull)>(state);
        case 0b111001:
            return getLegalMovesTemplate<GameStatus(0b111001ull)>(state);
        case 0b111010:
            return getLegalMovesTemplate<GameStatus(0b111010ull)>(state);
        case 0b111011:
            return getLegalMovesTemplate<GameStatus(0b111011ull)>(state);
        case 0b111100:
            return getLegalMovesTemplate<GameStatus(0b111100ull)>(state);
        case 0b111101:
            return getLegalMovesTemplate<GameStatus(0b111101ull)>(state);
        case 0b111110:
            return getLegalMovesTemplate<GameStatus(0b111110ull)>(state);
        case 0b111111:
            return getLegalMovesTemplate<GameStatus(0b111111ull)>(state);
        default:
            throw std::runtime_error(
                "Error: the pattern should be exaustive but failed");
    }
}

}  // namespace Movegen
