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
    void addHistory(const PastGameState& pastState);

    GameState state;
    std::array<PastGameState, 7> stateHistory;
    std::map<uint64_t, int> positionHashes;

   private:
};

inline std::vector<bool> generateObservation(const ChessGameEnv& env);

template <bool isWhite>
inline void makeMove(ChessGameEnv& env, Action action);

template <bool isWhite>
inline ChessObservation observeTemplate(const ChessGameEnv& env);

template <bool isWhite>
inline void updateGameState(ChessGameEnv& env, ActionInfo ai);

template <bool isWhite>

inline TerminationInfo checkForTermination(const ChessGameEnv& env);

template <bool isWhite>
inline std::vector<bool> generateLegalActionMask(const GameState& state);

// -----------------------------------------------------------------------------
// IMPLEMENTATIONS
void ChessGameEnv::showBoard() const { printBoard(state, 0ull); }

Moves ChessGameEnv::getPossibleMoves() const {
    return Movegen::getLegalMoves(state);
}

void ChessGameEnv::step(Move move) {
    addHistory(PastGameState(state));

    if (state.status.isWhite)
        makeMove<true>(*this, move);
    else
        makeMove<false>(*this, move);
}

ChessObservation ChessGameEnv::observe() {
    if (state.status.isWhite)
        return observeTemplate<true>(*this);
    else
        return observeTemplate<false>(*this);
}

void ChessGameEnv::addHistory(const PastGameState& pastState) {
    for (int i = 6; i > 0; i--) {
        stateHistory[i] = stateHistory[i - 1];
    }
    stateHistory[0] = pastState;

    const uint64_t posHash = pastState.positionHash;
    int currentValue = this->positionHashes[posHash];
    this->positionHashes[posHash] = currentValue + 1;
}

template <bool isWhite>
inline void updateGameState(ChessGameEnv& env, ActionInfo ai) {
    GameState& state = env.state;
    const uint16_t sourceSquare = ai.sourceSquare;
    const uint16_t targetSquare = ai.targetSquare;
    const PieceType promotion = ai.promotion;

    const Bitboard sourceBoard = 1ull << sourceSquare;
    const Bitboard targetBoard = 1ull << targetSquare;
    const PieceType type = getPieceType<isWhite>(state, sourceSquare);

    // need to do this to ensure the reset of the enpassant
    const bool isEnpassantPossible = state.status.enpassant;
    const Bitboard enpassantBoard = state.enpassant_board;
    state.clearEnpassant();

    // TODO: make this more readable
    if (type == PieceType::King &&
        isCastle<isWhite>(sourceSquare, targetSquare)) {
        handleCastling<isWhite>(state, sourceSquare, targetSquare);
        state.status.removeCastlingRights<isWhite>();
        env.positionHashes.clear();
        return;
    }

    updateCastlingRights<isWhite>(state, sourceBoard, targetBoard, type);

    Bitboard& pieceBoard = getBitboardFromSquare<isWhite>(state, sourceBoard);
    pieceBoard &= ~sourceBoard;

    // handle move
    moveToTargetPosition<isWhite>(state, pieceBoard, targetBoard, promotion,
                                  type);

    // handle enpassant take
    if (type == PieceType::Pawn && isEnpassantPossible &&
        targetBoard & enpassantBoard) {
        handleEnpassantCapture<isWhite>(state, targetBoard);
        return;
    }

    // handle enable enpassant
    if (enablesEnpassant<isWhite>(state, sourceBoard, targetBoard, type)) {
        state.setEnpassant(pawnPush1<isWhite>(sourceBoard));
        return;
    }
    removeEnemyPiece<isWhite>(state, targetBoard);
}

template <bool isWhite>
inline TerminationInfo checkForTermination(const ChessGameEnv& env) {
    const GameState& state = env.state;
    if (isCheckMate<isWhite>(state)) {
        if constexpr (isWhite) {
            return TerminationInfo{-1, 1, true};
        } else {
            return TerminationInfo{1, -1, true};
        }
    }

    if (isStaleMate<isWhite>(state) || isDrawBy50Moves(state) ||
        isDrawBy3FoldRepetition<isWhite>(state, env.positionHashes) ||
        isInsufficientMaterial(state)) {
        return TerminationInfo{0, 0, true};
    }

    if (state.fullMoveCount == (MAX_GAME_LENGTH + 1)) {
        return TerminationInfo{0, 0, true};
    }
    return TerminationInfo{0, 0, false};
}

template <bool isWhite>
inline void makeMove(ChessGameEnv& env, Action action) {
    GameState& state = env.state;
    ActionInfo actionInfo = parseAction<isWhite>(action);

    const bool isPawnMove =
        getPawns<isWhite>(state) & (1ull << actionInfo.sourceSquare);
    const bool isCapture =
        isEnemyPiece<isWhite>(state, actionInfo.targetSquare);
    updateGameState<isWhite>(env, actionInfo);
    updateMoveCount<isWhite>(state, isPawnMove, isCapture);

    // TODO: this can be better since moves that loose the right
    // to castle are also irreversible
    if (isPawnMove || isCapture) {
        env.positionHashes.clear();
    }
    state.status.nextPlayer();
}

inline std::vector<bool> generateObservation(const ChessGameEnv& env) {
    const GameState& state = env.state;
    constexpr int sideToMoveOffset = PLANE_SIZE * 4;
    constexpr int moveClockOffset = PLANE_SIZE * 5;
    constexpr int edgeFinderOffset = PLANE_SIZE * 6;
    constexpr int currentBoardOffset = PLANE_SIZE * 7;
    constexpr int boardSize = PLANE_SIZE * 13;
    constexpr int numPastBoards = 7;

    std::vector<bool> obs(OBSERVATION_SPACE_SIZE);

    // castling
    std::fill_n(obs.begin() + PLANE_SIZE * 0, PLANE_SIZE, state.status.wQueenC);
    std::fill_n(obs.begin() + PLANE_SIZE * 1, PLANE_SIZE, state.status.wKingC);
    std::fill_n(obs.begin() + PLANE_SIZE * 2, PLANE_SIZE, state.status.bQueenC);
    std::fill_n(obs.begin() + PLANE_SIZE * 3, PLANE_SIZE, state.status.wKingC);

    // side to move
    std::fill_n(obs.begin() + sideToMoveOffset, PLANE_SIZE,
                state.status.isWhite);

    // 50 move clock
    const int moveClockIndex = state.halfMoveClock;
    obs[moveClockOffset + moveClockIndex] = true;

    // edge finder
    addEdgeFinder(obs, edgeFinderOffset);

    const PastGameState curState = PastGameState(state);

    // check if board existed before
    bool is2FoldRep =
        occursMoreThan(env.positionHashes, getPositionHash(state), 0);
    fillObservationWithBoard(obs, curState, currentBoardOffset,
                             state.status.isWhite, is2FoldRep);

    // past boards
    for (int i = 0; i < numPastBoards; i++) {
        const bool isWhite =
            (i % 2 == 0) ? state.status.isWhite : !state.status.isWhite;
        const PastGameState oldState = env.stateHistory[i];
        const int startOffset = currentBoardOffset + (boardSize * (i + 1));

        const uint64_t posHash = oldState.positionHash;
        bool isRep = occursMoreThan(env.positionHashes, posHash, 1);

        fillObservationWithBoard(obs, oldState, startOffset, isWhite, isRep);
    }
    return obs;
}

template <bool isWhite>
inline ChessObservation observeTemplate(const ChessGameEnv& env) {
    const GameState& state = env.state;
    const TerminationInfo term = checkForTermination<isWhite>(env);
    return ChessObservation{
        generateObservation(env), generateLegalActionMask<isWhite>(state),
        term.whiteReward, term.blackReward, term.isTerminated};
}

template <bool isWhite>
inline std::vector<bool> generateLegalActionMask(const GameState& state) {
    Moves moves = Movegen::getLegalMoves(state);
    std::vector<bool> legalActionMask(ACTION_SPACE_SIZE);
    for (const Move& move : moves) {
        const uint64_t moveIndex = getMoveIndex<isWhite>(move);
        legalActionMask[moveIndex] = true;
    }
    return legalActionMask;
}
