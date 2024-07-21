#pragma once

#include <vector>
#include <deque>

#include "game_status.hpp"

using Bitboard = uint64_t;
using Move = uint16_t;
using Moves = std::vector<Move>;
using Action = uint16_t;


enum class PieceType : uint8_t {
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
    None,
};

