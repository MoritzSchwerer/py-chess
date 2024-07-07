#include <iostream>
#include <vector>
#include <array>
#include <immintrin.h>
#include <chrono>

#include "constants.h"
#include "lookup.h"
#include "types.h"

#define SquareOf(X) _tzcnt_u64(X)
#define Bitloop(X) for(;X; X = _blsr_u64(X))


/*
 * 0    0   0   0   quiet moves
 * 0	0	0	1	double pawn push
 * 0	0	1	0	king castle
 * 0	0	1	1	queen castle
 * 0	1	0	0	captures
 * 0	1	0	1	ep-capture
 * 1	0	0	0	knight-promotion
 * 1	0	0	1	bishop-promotion
 * 1	0	1	0	rook-promotion
 * 1	0	1	1	queen-promotion
 * 1	1	0	0	knight-promo capture
 * 1	1	0	1	bishop-promo capture
 * 1	1	1	0	rook-promo capture
 * 1	1	1	1	queen-promo capture
 */

void to_binary(Bitboard board) {
    for (int i = 63; i >= 0; i--) {
        bool is_set = (board & (1ull << i));
        std::cout << is_set;
    }
    std::cout << std::endl;
}

void print_move(Move move) {
    int source = move & 0x3f;
    int target = (move >> 6) & 0x3f;
    int flags  = (move >> 12) & 0xf;
    std::cout << "<Move src: " << source << ", tgt: " << target << ", flags: ";
    for (int i = 3; i >= 0; i--) {
        bool is_set = (move & (1ull << (12+i)));
        std::cout << is_set;
    }
    std::cout << ">" << std::endl;
}

void print_board(Bitboard board) {
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

Move create_move(Bitboard from, Bitboard to, uint64_t flags) {
    return (from & 0x3f) | ((to & 0x3f) << 6) | ((flags & 0xf) << 12);
}


struct GameState {
    Bitboard w_pawn   = 0x000000000000FF00;
    Bitboard w_rook   = 0x0000000000000081;
    Bitboard w_knight = 0x0000000000000042;
    Bitboard w_bishop = 0x0000000000000024;
    Bitboard w_queen  = 0x0000000000000008;
    Bitboard w_king   = 0x0000000000000010;

    Bitboard b_pawn   = 0x00FF000000000000;
    Bitboard b_rook   = 0x8100000000000000;
    Bitboard b_knight = 0x4200000000000000;
    Bitboard b_bishop = 0x2400000000000000;
    Bitboard b_queen  = 0x0800000000000000;
    Bitboard b_king   = 0x1000000000000000;

    Bitboard enpassant = 0ull;

    bool w_king_castle = true;
    bool w_queen_castle = true;
    bool b_king_castle = true;
    bool b_queen_castle = true;
};

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
    const Bitboard blockingPieces = friendlies | enemies & ~enemyKing;

    const Bitboard pawns = getPawns<isWhite>(state);
    Bitboard knights = getKnights<isWhite>(state);
    Bitboard rooks = getRooks<isWhite>(state) | getQueens<isWhite>(state);
    Bitboard bishops = getBishops<isWhite>(state) | getQueens<isWhite>(state);
    Bitboard king = getKing<isWhite>(state);

    Bitboard seenSquares = 0ull;

    // pawns
    const Bitboard attackLeft  = (pawns & ~FILE_A) << 7;
    const Bitboard attackRight = (pawns & ~FILE_H) << 9;
    seenSquares |= attackLeft | attackRight;

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

std::vector<Move> get_legal_king_moves(GameState state) {
    std::vector<Move> moves;
    moves.reserve(16);
    // TODO: simple table lookup but need enemy attack map
    return moves;
}

// TODO: Write tests to ensure the moves are actually legal



int main() {
    GameState state;

    state.w_pawn = 0x000000000000F700;
    // state.b_pawn = 0x0000000000000000;
    // state.w_knight = 0x0000000000000000;
    // state.w_bishop =  0x0000001800000000;
    print_board(state.w_pawn);
    // print_board(state.w_bishop);
    // state.w_rook = 0x0000000000000080;
    // print_board(state.w_rook);

    std::vector<Move> moves = get_legal_moves<true>(state);
    for (const Move& m : moves) {
        print_move(m);
    }
    std::cout << "Number of possible moves detected: " << moves.size() << std::endl;


    // for (int i = 0; i < 64; i++) {
    //     std::cout << i << std::endl;
    //     print_board(knightAttacks[i]);
    // }


    return 0;
}

