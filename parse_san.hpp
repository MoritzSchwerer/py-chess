#include <regex>
#include <string>
#include <iostream>
#include <exception>

#include "types.hpp"
#include "moves.hpp"
#include "movegen.hpp"

Bitboard charToFile(const char file) {
    static const Bitboard fileBitboards[8] = {
        0x0101010101010101, // a-file
        0x0202020202020202, // b-file
        0x0404040404040404, // c-file
        0x0808080808080808, // d-file
        0x1010101010101010, // e-file
        0x2020202020202020, // f-file
        0x4040404040404040, // g-file
        0x8080808080808080  // h-file
    };
    return fileBitboards[file - 'a'];
}

Bitboard charToRank(const char rank) {
    static const Bitboard rankBitboards[8] = {
        0x00000000000000FF, // 1th rank
        0x000000000000FF00, // 2th rank
        0x0000000000FF0000, // 3th rank
        0x00000000FF000000, // 4th rank
        0x000000FF00000000, // 5th rank
        0x0000FF0000000000, // 6rd rank
        0x00FF000000000000, // 7nd rank
        0xFF00000000000000, // 8st rank
    };
    return rankBitboards[rank - '1'];
}

uint64_t sanToSquare(std::string squareSAN) {
    if (squareSAN.length() != 2) throw std::runtime_error("Error: SAN must be of length 2");
    const uint64_t file = squareSAN[0] - 'a';
    const uint64_t rank = squareSAN[1] - '1';
    return rank * 8 + file;
}

template<bool isWhite>
Moves getLegalMovesPerPiece(const GameState &state, const std::string pieceType) {
    Moves moves;
    switch(pieceType[0]) {
        case 'R': 
            getLegalRookMoves<isWhite>(state, moves);
            break;
        case 'N': 
            getLegalKnightMoves<isWhite>(state, moves);
            break;
        case 'B': 
            getLegalBishopMoves<isWhite>(state, moves);
            break;
        case 'Q': 
            getLegalQueenMoves<isWhite>(state, moves);
            break;
        case 'K': 
            getLegalKingMoves<isWhite>(state, moves);
            break;
        default: 
            getLegalPawnMoves<isWhite>(state, moves);
            break;
    }
    return moves;
}

uint64_t getPromotionFlags(const char promoType) {
    switch (promoType) {
        case 'N': 
            return 0b1000;
        case 'B': 
            return 0b1001;
        case 'R': 
            return 0b1010;
        case 'Q': 
            return 0b1011;
        default:
            throw std::runtime_error("Error: promoType must be NBRQ");
    }
}

template<bool isWhite>
Move sanToMove(std::string san, const GameState &state) {
    std::regex pattern("^([NBKRQ])?([a-h])?([1-8])?([‐x])?([a-h][1-8])(=[nbrqkNBRQK])?[#+]?$");
    std::smatch match;

    if (!std::regex_match(san, match, pattern)) {
        // if regex doesn't match check for castle
        if (san == "O-O") {
            if constexpr (isWhite) {
                return create_move(4ull, 7ull, 0b0010);
            } else {
                return create_move(60ull, 63ull, 0b0010);
            }
        } 
        if (san == "O-O-O") {
            if constexpr (isWhite) {
                return create_move(4ull, 0ull, 0b0011);
            } else {
                return create_move(60ull, 56ull, 0b0011);
            }
        }
        throw std::runtime_error("Error: couldn't match SAN: " + san);
    }



    const uint64_t targetSquare = sanToSquare(match.str(5));
    const Bitboard file = match.str(2).empty() ? 0xffffffffffffffff : charToFile(match.str(2)[0]);
    const Bitboard rank = match.str(3).empty() ? 0xffffffffffffffff : charToFile(match.str(2)[0]);
    const std::string pieceType = match.str(1);

    Moves moves = getLegalMovesPerPiece<isWhite>(state, pieceType);
    Move validMove = 0ull;
    for (const Move &m : moves) {
        const uint64_t moveSource = m & 0b111111;
        const uint64_t moveTarget = (m >> 6) & 0b111111;
        const uint64_t moveFlags = (m >> 12) & 0b1011;

        // check if target and file match the move
        if (moveTarget == targetSquare && (1ull << moveSource & file & rank)) {

            // if the piece is a pawn check if it can promote
            if (pieceType.empty() && (1ull << targetSquare) & lastRank<isWhite>()) {
                if (moveFlags == getPromotionFlags(match.str(6)[1])) {
                    validMove = m;
                    break;
                }

            // if it is not a pawn or a pawn that cannot promote
            } else {
                validMove = m;
                break;
            }
        }
    }
    if (validMove == 0ull) throw std::runtime_error("Error: Didn't find a valid move for: " + san);
    return validMove;
}


template<bool isWhite>
Moves sansToMoves(const std::vector<std::string> &sans, const GameState &state) {
    Moves moves;
    for (const std::string &san : sans) {
        moves.push_back(sanToMove<isWhite>(san, state));
    }
    return moves;
}





































