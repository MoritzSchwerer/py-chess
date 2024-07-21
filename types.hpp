#pragma once

#include <vector>
#include <deque>

#include "game_status.hpp"

typedef uint64_t Bitboard;
typedef uint16_t Move;
typedef std::vector<Move> Moves;

enum class PieceType : uint8_t {
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
    None,
};

