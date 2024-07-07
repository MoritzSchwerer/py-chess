#pragma once

typedef uint64_t Bitboard;
typedef uint16_t Move;

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

