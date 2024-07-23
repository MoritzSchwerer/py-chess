#pragma once

#include <iostream>
#include <string>
#include <array>
#include <exception>
#include <algorithm>
#include <map>

#include "types.hpp"
#include "move_gen.hpp"
#include "utils.hpp"
#include "lookup.hpp"
#include "game_state.hpp"

constexpr int OBSERVATION_SPACE_SIZE = 7104;
constexpr int ACTION_SPACE_SIZE = 4672; 
constexpr int NUM_ACTION_PLANES = 73;
constexpr int PLANE_SIZE = 64;
constexpr int MAX_GAME_LENGTH = 500;

struct TerminationInfo {
    int32_t whiteReward;
    int32_t blackReward;
    bool isTerminated;

    TerminationInfo(int32_t whiteReward, int32_t blackReward, bool isTerminated)
      : whiteReward(whiteReward),
        blackReward(blackReward),
        isTerminated(isTerminated) {}
};

struct ChessObservation {
    std::vector<bool> observation;
    std::vector<bool> actionMask;
    int32_t whiteReward;
    int32_t blackReward;
    bool isTerminated;

    ChessObservation() = default;
    ChessObservation(ChessObservation&& other)  noexcept
        : observation(std::move(other.observation)),
          actionMask(std::move(other.actionMask)),
          whiteReward(other.whiteReward),
          blackReward(other.blackReward),
          isTerminated(other.isTerminated) {}

    ChessObservation(std::vector<bool>&& observation, std::vector<bool>&& actionMask, int whiteReward, int blackReward, bool isTerminated)
        : observation(std::move(observation)),
          actionMask(std::move(actionMask)),
          whiteReward(whiteReward),
          blackReward(blackReward),
          isTerminated(isTerminated) {}
};

class ChessGameEnv {
public:
    GameState state;

    ChessGameEnv() {}
    ChessGameEnv(const std::string &fen) : state(parseFen(fen)) {}
    ChessGameEnv(const ChessGameEnv& other) : state(other.state) {}


    Moves getPossibleMoves() const { return Movegen::getLegalMoves(state); }

    void step(const Action action);
    ChessObservation observe();
};

template<bool isWhite>
PieceType getPieceType(const GameState& state, uint8_t square) {
    const Bitboard board = 1ull << square;
    if constexpr (isWhite) {
        if (state.w_pawn & board) return PieceType::Pawn;
        if (state.w_knight & board) return PieceType::Knight;
        if (state.w_bishop & board) return PieceType::Bishop;
        if (state.w_rook & board) return PieceType::Rook;
        if (state.w_queen & board) return PieceType::Queen;
        if (state.w_king & board) return PieceType::King;
    } else {
        if (state.b_pawn & board) return PieceType::Pawn;
        if (state.b_knight & board) return PieceType::Knight;
        if (state.b_bishop & board) return PieceType::Bishop;
        if (state.b_rook & board) return PieceType::Rook;
        if (state.b_queen & board) return PieceType::Queen;
        if (state.b_king & board) return PieceType::King;
    }
    return PieceType::None;
}

template<bool isWhite>
Bitboard& getBitboardFromSquare(GameState& state, Bitboard board) {
    if constexpr (isWhite) {
        if (state.w_pawn & board) return state.w_pawn;
        if (state.w_knight & board) return state.w_knight;
        if (state.w_bishop & board) return state.w_bishop;
        if (state.w_rook & board) return state.w_rook;
        if (state.w_queen & board) return state.w_queen;
        if (state.w_king & board) return state.w_king;
    } else {
        if (state.b_pawn & board) return state.b_pawn;
        if (state.b_knight & board) return state.b_knight;
        if (state.b_bishop & board) return state.b_bishop;
        if (state.b_rook & board) return state.b_rook;
        if (state.b_queen & board) return state.b_queen;
        if (state.b_king & board) return state.b_king;
    }
    throw std::runtime_error("Error: getBitboardFromSquare couldn't find a match for the square.");
}

template<bool isWhite>
Bitboard& getBitboardFromPieceType(GameState& state, PieceType type) {
    if constexpr (isWhite) {
        if (type == PieceType::Pawn) return state.w_pawn;
        if (type == PieceType::Knight) return state.w_knight;
        if (type == PieceType::Bishop) return state.w_bishop;
        if (type == PieceType::Rook) return state.w_rook;
        if (type == PieceType::Queen) return state.w_queen;
        if (type == PieceType::King) return state.w_king;
    } else {
        if (type == PieceType::Pawn) return state.b_pawn;
        if (type == PieceType::Knight) return state.b_knight;
        if (type == PieceType::Bishop) return state.b_bishop;
        if (type == PieceType::Rook) return state.b_rook;
        if (type == PieceType::Queen) return state.b_queen;
        if (type == PieceType::King) return state.b_king;
    }
    throw std::runtime_error("Error: getBitboardFromSquare couldn't find a match for the square.");
}

// returns if we actually removed a piece
template<bool isWhite>
bool removeEnemyPiece(GameState& state, Bitboard targetBoard) {
    if constexpr (isWhite) {
        state.b_pawn &= ~targetBoard;
        state.b_knight &= ~targetBoard;
        state.b_bishop &= ~targetBoard;
        state.b_rook &= ~targetBoard;
        state.b_queen &= ~targetBoard;
        state.b_king &= ~targetBoard;
        return (state.b_pawn & targetBoard) | (state.b_knight & targetBoard) | (state.b_bishop & targetBoard) | (state.b_rook & targetBoard) | (state.b_queen & targetBoard) | (state.b_king & targetBoard);
    } else {
        state.w_pawn &= ~targetBoard;
        state.w_knight &= ~targetBoard;
        state.w_bishop &= ~targetBoard;
        state.w_rook &= ~targetBoard;
        state.w_queen &= ~targetBoard;
        state.w_king &= ~targetBoard;
        return (state.w_pawn & targetBoard) | (state.w_knight & targetBoard) | (state.w_bishop & targetBoard) | (state.w_rook & targetBoard) | (state.w_queen & targetBoard) | (state.w_king & targetBoard);
    }
}

template<bool isWhite>
inline bool isCastle(uint8_t ss, uint8_t ts) {
    if constexpr (isWhite) {
        return ss == 4 && (ts == 0 || ts == 7);
    } else {
        return ss == 60 && (ts == 56 || ts == 63);
    }
}

template<bool isWhite>
void handleCastling(GameState& state, uint8_t sourceSquare, uint8_t targetSquare) {
    // handle casteling
    if constexpr (isWhite) {
        // left castle
        if (sourceSquare > targetSquare) {
            state.w_king =   0b00000100ull;
            state.w_rook &= ~0b00000001ull;
            state.w_rook |=  0b00001000ull;
        } else {
            state.w_king =   0b01000000ull;
            state.w_rook &= ~0b10000000ull;
            state.w_rook |=  0b00100000ull;
        }
    } else {
        if (sourceSquare > targetSquare) {
            state.w_king =   0b00000100ull << 56;
            state.w_rook &= ~0b00000001ull << 56;
            state.w_rook |=  0b00001000ull << 56;
        } else {
            state.w_king =   0b01000000ull << 56;
            state.w_rook &= ~0b10000000ull << 56;
            state.w_rook |=  0b00100000ull << 56;
        }
    }
    state.status.removeCastlingRights<isWhite>();
}

template<bool isWhite>
void updateCastlingRights(GameState& state, Bitboard sourceBoard, PieceType type) {
    // update castling rights king moves
    if (type == PieceType::King) {
        state.status.removeCastlingRights<isWhite>();
    }

    // update castling right rook moves
    if (type == PieceType::Rook) {
        if (sourceBoard & initialRookLeft<isWhite>() ) state.status.removeCastlingRightsLeft<isWhite>();
        if (sourceBoard & initialRookRight<isWhite>()) state.status.removeCastlingRightsRight<isWhite>();
    }
}

template<bool isWhite>
void handlePromotion(GameState& state, Bitboard& pieceBoard, Bitboard targetBoard, PieceType promotion, PieceType type) {
    if (type == PieceType::Pawn && targetBoard & lastRank<isWhite>()) {
        Bitboard& promotionBoard = getBitboardFromPieceType<isWhite>(state, promotion);
        promotionBoard |= targetBoard;
    } else {
        pieceBoard |= targetBoard;
    }
}

template<bool isWhite>
void handleEnpassantCapture(GameState& state, Bitboard targetBoard) {
    removeEnemyPiece<isWhite>(state, pawnPush1<!isWhite>(targetBoard));
}

template<bool isWhite>
bool updateGameState(GameState& state, uint8_t sourceSquare, uint8_t targetSquare, PieceType promotion) {
    const GameStatus& status = state.status;
    const Bitboard sourceBoard = 1ull << sourceSquare;
    const Bitboard targetBoard = 1ull << targetSquare;

    const PieceType type = getPieceType<isWhite>(state, sourceSquare);
    const bool isEnpassantPossible = state.status.enpassant;
    state.clearEnpassant();

    if (isCastle<isWhite>(sourceSquare, targetSquare)) {
        handleCastling<isWhite>(state, sourceSquare, targetSquare);
        return false;
    }

    updateCastlingRights<isWhite>(state, sourceBoard, type);

    // NOTE: fails here
    // remove piece from source position
    Bitboard& pieceBoard = getBitboardFromSquare<isWhite>(state, sourceBoard);
    pieceBoard &= ~sourceBoard;

    // handle promotion
    handlePromotion<isWhite>(state, pieceBoard, targetBoard, promotion, type);

    // handle enpassant take
    if (type == PieceType::Pawn && isEnpassantPossible && targetBoard & state.enpassant_board) {
        handleEnpassantCapture<isWhite>(state, targetBoard);
        return true;
    }

    // handle enable enpassant
    if (enablesEnpassant<isWhite>(state, sourceBoard, targetBoard, type)) {
        state.setEnpassant(pawnPush1<isWhite>(sourceBoard));
        return true;
    }


    const bool tookEnemyPiece = removeEnemyPiece<isWhite>(state, targetBoard);
    return tookEnemyPiece || type == PieceType::Pawn;
}

template<bool isWhite>
bool enablesEnpassant(GameState& state, Bitboard sourceBoard, Bitboard targetBoard, PieceType type) {
    const Bitboard enemyPawns = getEnemyPawns<isWhite>(state);
    return (
        type == PieceType::Pawn &&
        targetBoard & pawnPush2<isWhite>(sourceBoard) &&
        (pawnAttackLeft<isWhite>(pawnPush1<isWhite>(sourceBoard)) & enemyPawns ||
        pawnAttackRight<isWhite>(pawnPush1<isWhite>(sourceBoard)) & enemyPawns)
    );
}

template<bool isWhite>
void updateMoveCount(GameState& state, bool reset) {
    if (reset) state.halfMoveClock = 0;
    else state.halfMoveClock++;

    if constexpr (!isWhite) state.fullMoveCount++;
}

template<bool isWhite>
int8_t getOffsetFromPlane(uint8_t plane) {
    if constexpr (isWhite) return Lookup::planeToOffsetWhite[plane];
    else return Lookup::planeToOffsetWhite[plane];
}

template<bool isWhite>
void makeMove(GameState& state, Action action) {
    const int sourceSquare = action / NUM_ACTION_PLANES;
    const int plane = action % NUM_ACTION_PLANES;
    const int offset = getOffsetFromPlane<isWhite>(plane);
    const int targetSquare = sourceSquare + offset;

    // if (isWhite) {
    //     std::cout << "White" << std::endl;
    // } else {
    //     std::cout << "Black" << std::endl;
    // }
    //
    // printMove(action);
    // std::cout << "action: " << action << std::endl;
    // std::cout << "source: " << sourceSquare << std::endl;
    // std::cout << "plane: " << plane << std::endl;
    // std::cout << "offset: " << offset << std::endl;
    // std::cout << "target: " << targetSquare << std::endl;


    const PieceType promotion = Lookup::getPromotion(plane);

    bool resetHalfMoveClock = updateGameState<isWhite>(state, sourceSquare, targetSquare, promotion);

    updateMoveCount<isWhite>(state, resetHalfMoveClock);

    state.status.silentMove();
}

void ChessGameEnv::step(Action action) {
    state.addHistory(PastGameState(state));

    if (state.status.isWhite) makeMove<true>(state, action);
    else makeMove<false>(state, action);
}

void fillObservationWithBoard(std::vector<bool>& obs, const PastGameState& pastState, int startOffset, bool isWhite, bool is2FoldRep) {
    constexpr int pawnOffset = 0;
    constexpr int rookOffset = 1;
    constexpr int knightOffset = 2;
    constexpr int bishopOffset = 3;
    constexpr int queenOffset = 4;
    constexpr int kingOffset = 5;
    constexpr int whiteOffset = 0;
    constexpr int blackOffset = 6;
    constexpr int repetitionOffset = 12;

    Bitboard w_pawn = pastState.w_pawn;
    Bitboard w_rook = pastState.w_pawn;
    Bitboard w_knight = pastState.w_pawn;
    Bitboard w_bishop = pastState.w_pawn;
    Bitboard w_queen = pastState.w_pawn;
    Bitboard w_king = pastState.w_pawn;

    Bitboard b_pawn = pastState.b_pawn;
    Bitboard b_rook = pastState.b_pawn;
    Bitboard b_knight = pastState.b_pawn;
    Bitboard b_bishop = pastState.b_pawn;
    Bitboard b_queen = pastState.b_pawn;
    Bitboard b_king = pastState.b_pawn;

    Bitboard enpassant = pastState.enpassant_board;
    
    if (isWhite) {
        // black pawn can be taken enpassant on 6th rank we move it to 8th rank
        b_pawn |= enpassant << 16;
    } else {
        // white pawn can be taken enpassant on 3rd rank we move it to 8th rank
        w_pawn |= enpassant << 40;
    }

    Bitloop(w_pawn) {
        const uint64_t sourceSquare = SquareOf(w_pawn);
        obs[startOffset+(whiteOffset+pawnOffset)*PLANE_SIZE+sourceSquare] = true;
    }
    Bitloop(w_rook) {
        const uint64_t sourceSquare = SquareOf(w_rook);
        obs[startOffset+(whiteOffset+rookOffset)*PLANE_SIZE+sourceSquare] = true;
    }
    Bitloop(w_knight) {
        const uint64_t sourceSquare = SquareOf(w_knight);
        obs[startOffset+(whiteOffset+knightOffset)*PLANE_SIZE+sourceSquare] = true;
    }
    Bitloop(w_bishop) {
        const uint64_t sourceSquare = SquareOf(w_bishop);
        obs[startOffset+(whiteOffset+bishopOffset)*PLANE_SIZE+sourceSquare] = true;
    }
    Bitloop(w_queen) {
        const uint64_t sourceSquare = SquareOf(w_queen);
        obs[startOffset+(whiteOffset+queenOffset)*PLANE_SIZE+sourceSquare] = true;
    }
    Bitloop(w_king) {
        const uint64_t sourceSquare = SquareOf(w_king);
        obs[startOffset+(whiteOffset+kingOffset)*PLANE_SIZE+sourceSquare] = true;
    }

    Bitloop(b_pawn) {
        const uint64_t sourceSquare = SquareOf(b_pawn);
        obs[startOffset+(blackOffset+pawnOffset)*PLANE_SIZE+sourceSquare] = true;
    }
    Bitloop(b_rook) {
        const uint64_t sourceSquare = SquareOf(b_rook);
        obs[startOffset+(blackOffset+rookOffset)*PLANE_SIZE+sourceSquare] = true;
    }
    Bitloop(b_knight) {
        const uint64_t sourceSquare = SquareOf(b_knight);
        obs[startOffset+(blackOffset+knightOffset)*PLANE_SIZE+sourceSquare] = true;
    }
    Bitloop(b_bishop) {
        const uint64_t sourceSquare = SquareOf(b_bishop);
        obs[startOffset+(blackOffset+bishopOffset)*PLANE_SIZE+sourceSquare] = true;
    }
    Bitloop(b_queen) {
        const uint64_t sourceSquare = SquareOf(b_queen);
        obs[startOffset+(blackOffset+queenOffset)*PLANE_SIZE+sourceSquare] = true;
    }
    Bitloop(b_king) {
        const uint64_t sourceSquare = SquareOf(b_king);
        obs[startOffset+(blackOffset+kingOffset)*PLANE_SIZE+sourceSquare] = true;
    }
    std::fill_n(obs.begin() + startOffset + PLANE_SIZE * repetitionOffset, PLANE_SIZE, is2FoldRep);
}

void addEdgeFinder(std::vector<bool>& obs, int startOffset) {
    Bitboard border = RANK_1 | RANK_8 | FILE_A | FILE_H;
    Bitloop(border) {
        const uint64_t offset = SquareOf(border);
        obs[startOffset+offset] = true;
    }
}

bool occursMoreThan(const std::map<uint64_t, int>& map, uint64_t hash, int threshold) {
    auto it = map.find(hash);
    if (it != map.end() && it->second > threshold)  {
        return true;
    }
    return false;
}

std::vector<bool> generateObservation(const GameState& state) {
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
    std::fill_n(obs.begin() + sideToMoveOffset, PLANE_SIZE, state.status.isWhite);

    // 50 move clock
    const int moveClockIndex = state.halfMoveClock;
    obs[moveClockOffset + moveClockIndex] = true;

    // edge finder
    addEdgeFinder(obs, edgeFinderOffset);

    const PastGameState curState = PastGameState(state);

    // check if board existed before
    bool is2FoldRep = occursMoreThan(state.positionHashes, state.getPositionHash(), 0);
    fillObservationWithBoard(obs, curState, currentBoardOffset, state.status.isWhite, is2FoldRep);

    // past boards
    for (int i = 0; i < numPastBoards; i++) {
        const bool isWhite = (i % 2 == 0) ? state.status.isWhite : !state.status.isWhite;
        const PastGameState oldState = state.stateHistory[i];
        const int startOffset = currentBoardOffset + (boardSize * (i + 1));

        const uint64_t posHash = oldState.positionHash;
        bool isRep = occursMoreThan(state.positionHashes, posHash, 1);

        fillObservationWithBoard(obs, oldState, startOffset, isWhite, isRep);
    }
    return obs;
}

template<bool isWhite>
uint64_t getMoveIndex(const Move move) {
    const uint64_t sourceSquare = move & 0b111111;
    const uint64_t targetSquare = (move >> 6) & 0b111111;
    const uint64_t flags = (move >> 12) & 0b1111;

    // check for non queen promotion
    if (flags >= 0b1000 && ((flags & 0b0011) != 0b0011)) {
        const uint64_t moveOffset = (targetSquare - sourceSquare);
        const uint64_t promoOffset = flags & 0b0011;
        uint64_t plane;
        if constexpr (isWhite) {
            // we map 7, 8, 9 to 64, 67, 70 and their promotions
            plane = ((moveOffset - 7) * 3) + 64 + promoOffset;
        } else {
            // we map -9, -8, -7 to 64, 67, 70 and their promotions
            plane = ((moveOffset + 9) * 3) + 64 + promoOffset;
        }
        return plane + NUM_ACTION_PLANES * sourceSquare;
    }

    // this puts the starting location at square 0
    const uint64_t normalizedOffset = (targetSquare - sourceSquare) + 64;

    // table lookup what move that is
    uint64_t plane;
    if constexpr (isWhite) {
        plane = Lookup::offsetToPlaneWhite[normalizedOffset];
    } else {
        plane = Lookup::offsetToPlaneBlack[normalizedOffset];
    }
    uint64_t temp = plane + NUM_ACTION_PLANES * sourceSquare;
    return temp;
}

template<bool isWhite>
std::vector<bool> generateLegalActionMask(const GameState& state) {
    Moves moves = Movegen::getLegalMoves(state);
    std::vector<bool> legalActionMask(ACTION_SPACE_SIZE);
    for (const Move& move : moves) {
        const uint64_t moveIndex = getMoveIndex<isWhite>(move);
        legalActionMask[moveIndex] = true;
    }
    return legalActionMask;
}

template<bool isWhite>
bool isCheckMate(const GameState& state) {
    const Bitboard enemySeenSquares = Movegen::getSeenSquares<!isWhite>(state);
    const Bitboard ownKing = getKing<isWhite>(state);

    // king is in check
    if (enemySeenSquares & ownKing) {
        // there are no legal moves
        const Moves legalMoves = Movegen::getLegalMoves(state);
        if (legalMoves.size() == 0) {
            return true;
        }

    }
    return false;
}

template<bool isWhite>
bool isStaleMate(const GameState& state) {
    const Bitboard enemySeenSquares = Movegen::getSeenSquares<!isWhite>(state);
    const Bitboard ownKing = getKing<isWhite>(state);

    // king is in check
    if (enemySeenSquares & ownKing) {
        return false;
    }

    // there are no legal moves
    const Moves legalMoves = Movegen::getLegalMoves(state);
    if (legalMoves.size() == 0) {
        return true;
    }
    return false;
}

bool isDrawBy50Moves(const GameState& state) {
    if (state.halfMoveClock >= 100) {
        return true;
    }
    return false;
}

template<bool isWhite>
bool isDrawBy3FoldRepetition(const GameState& state) {
    const uint64_t posHash = state.getPositionHash();
    return occursMoreThan(state.positionHashes, posHash, 2);
}

template<bool isWhite>
TerminationInfo checkForTermination(const GameState& state) {
    if (isCheckMate<isWhite>(state)) {
        if constexpr (isWhite) {
            return TerminationInfo {-1, 1, true};
        } else {
            return TerminationInfo {1, -1, true};
        }
    }

    if (isStaleMate<isWhite>(state) || isDrawBy50Moves(state) || isDrawBy3FoldRepetition<isWhite>(state)) {
        return TerminationInfo {
            0, 0, true
        };

    }

    if (state.fullMoveCount == (MAX_GAME_LENGTH+1)) {
        return TerminationInfo {
            0, 0, true
        };
    }
    return TerminationInfo {
        0, 0, false
    };
}

template<bool isWhite>
ChessObservation observeTemplate(const GameState& state) {
    const TerminationInfo term = checkForTermination<isWhite>(state);
    return ChessObservation{
        generateObservation(state),
        generateLegalActionMask<isWhite>(state),
        term.whiteReward,
        term.blackReward,
        term.isTerminated
    };
}

ChessObservation ChessGameEnv::observe() {
    if (state.status.isWhite) return observeTemplate<true>(state);
    else return observeTemplate<false>(state);
}
