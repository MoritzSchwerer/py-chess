#pragma once

#include <algorithm>
#include <array>
#include <exception>
#include <iostream>
#include <map>
#include <string>

#include "game_state.hpp"
#include "game_state_utils.hpp"
#include "types.hpp"
#include "utils.hpp"

class ChessGameEnv {
   public:
    ChessGameEnv() {}
    ChessGameEnv(const std::string& fen) : state(parseFen(fen)) {}
    ChessGameEnv(const ChessGameEnv& other) : state(other.state) {}

    Moves getPossibleMoves() const;
    void step(const Move move);
    ChessObservation observe();
    void showBoard() const;

    GameState getState() const;

   private:
    GameState state;
};

void ChessGameEnv::showBoard() const { printBoard(state, 0ull); }

Moves ChessGameEnv::getPossibleMoves() const {
    return Movegen::getLegalMoves(state);
}
ChessObservation ChessGameEnv::observe() {
    if (state.status.isWhite)
        return observeTemplate<true>(state);
    else
        return observeTemplate<false>(state);
}
void ChessGameEnv::step(Move move) {
    state.addHistory(PastGameState(state));

    if (state.status.isWhite)
        makeMove<true>(state, move);
    else
        makeMove<false>(state, move);
}

GameState ChessGameEnv::getState() const { return state; }
