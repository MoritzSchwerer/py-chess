#pragma once

#include <array>
#include <map>
#include <string>

#include "game_status.hpp"
#include "types.hpp"

struct GameState;

struct PastGameState {
    Bitboard w_pawn;
    Bitboard w_rook;
    Bitboard w_knight;
    Bitboard w_bishop;
    Bitboard w_queen;
    Bitboard w_king;

    Bitboard b_pawn;
    Bitboard b_rook;
    Bitboard b_knight;
    Bitboard b_bishop;
    Bitboard b_queen;
    Bitboard b_king;

    Bitboard enpassant_board;

    uint64_t positionHash;

    PastGameState() = default;
    PastGameState(const GameState &state);
};

struct GameState {
    Bitboard w_pawn;
    Bitboard w_rook;
    Bitboard w_knight;
    Bitboard w_bishop;
    Bitboard w_queen;
    Bitboard w_king;

    Bitboard b_pawn;
    Bitboard b_rook;
    Bitboard b_knight;
    Bitboard b_bishop;
    Bitboard b_queen;
    Bitboard b_king;

    Bitboard enpassant_board;
    uint32_t halfMoveClock;
    uint32_t fullMoveCount;

    GameStatus status;

    GameState()
        : w_pawn(0x000000000000FF00),
          w_rook(0x0000000000000081),
          w_knight(0x0000000000000042),
          w_bishop(0x0000000000000024),
          w_queen(0x0000000000000008),
          w_king(0x0000000000000010),
          b_pawn(0x00FF000000000000),
          b_rook(0x8100000000000000),
          b_knight(0x4200000000000000),
          b_bishop(0x2400000000000000),
          b_queen(0x0800000000000000),
          b_king(0x1000000000000000),
          enpassant_board(0ull),
          halfMoveClock(0ul),
          fullMoveCount(1ul),
          status() {
        status.isWhite = true;
        status.wKingC = true;
        status.wQueenC = true;
        status.bKingC = true;
        status.bQueenC = true;
        status.enpassant = false;
    }

    void setEnpassant(Bitboard enpassantBoard);
    void clearEnpassant();
};

uint64_t getPositionHash(const GameState &state);
GameState GameStateEmpty();
GameState parseFen(const std::string &fen);
