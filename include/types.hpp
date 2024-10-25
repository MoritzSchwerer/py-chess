#pragma once

#include <deque>
#include <stdint.h>
#include <vector>

using Bitboard = uint64_t;
using Move = uint16_t;
using Moves = std::vector<Move>;
using Action = uint64_t;

enum class PieceType : uint8_t {
  Pawn,
  Knight,
  Bishop,
  Rook,
  Queen,
  King,
  None,
};

struct ActionInfo {
  uint16_t sourceSquare;
  uint16_t targetSquare;
  PieceType promotion;
};
