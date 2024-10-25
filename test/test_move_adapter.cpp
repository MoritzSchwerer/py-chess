#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "types.hpp"
#include "moves.hpp"
#include "game_env.hpp"
#include "utils.hpp"

namespace {

uint64_t promotionToFlags(PieceType type) {
    switch(type) {
        case PieceType::Knight: return 0b1000;
        case PieceType::Bishop: return 0b1001;
        case PieceType::Rook:   return 0b1010;
        case PieceType::Queen:  return 0b1011;
        default: return 0b0000;
    }
}

class MoveGenerator final : public Catch::Generators::IGenerator<Move> {
    Moves moves;
    int currentIndex;
public:

    MoveGenerator(const GameState& state) : moves(Movegen::getLegalMoves(state)), currentIndex(0) {}

    Move const& get() const override;

    bool next() override {
        currentIndex++;
        return currentIndex < moves.size();
    }
};

Move const& MoveGenerator::get() const {
    return moves[currentIndex];
}

template<bool isWhite>
Catch::Generators::GeneratorWrapper<Move> loadQueenMoves(Bitboard position) {
    GameState state = GameStateEmpty();
    if (isWhite) {
        state.w_queen = position;
    } else {
        state.b_queen = position;
    }
    return Catch::Generators::GeneratorWrapper<Move>(
        std::make_unique<MoveGenerator>(state)
    );
}

template<bool isWhite>
Catch::Generators::GeneratorWrapper<Move> loadKnightMoves(Bitboard position) {
    GameState state = GameStateEmpty();
    if (isWhite) {
        state.w_knight = position;
    } else {
        state.b_knight = position;
    }
    return Catch::Generators::GeneratorWrapper<Move>(
        std::make_unique<MoveGenerator>(state)
    );
}

template<bool isWhite>
Catch::Generators::GeneratorWrapper<Move> loadBishopMoves(Bitboard position) {
    GameState state = GameStateEmpty();
    if (isWhite) {
        state.w_bishop = position;
    } else {
        state.b_bishop = position;
    }
    return Catch::Generators::GeneratorWrapper<Move>(
        std::make_unique<MoveGenerator>(state)
    );
}

template<bool isWhite>
Catch::Generators::GeneratorWrapper<Move> loadRookMoves(Bitboard position) {
    GameState state = GameStateEmpty();
    if (isWhite) {
        state.w_rook = position;
    } else {
        state.b_rook = position;
    }
    return Catch::Generators::GeneratorWrapper<Move>(
        std::make_unique<MoveGenerator>(state)
    );
}

template<bool isWhite>
Catch::Generators::GeneratorWrapper<Move> loadPawnMoves(Bitboard position) {
    GameState state = GameStateEmpty();
    if (isWhite) {
        state.w_pawn = position;
    } else {
        state.b_pawn = position;
    }
    return Catch::Generators::GeneratorWrapper<Move>(
        std::make_unique<MoveGenerator>(state)
    );
}


}


TEST_CASE("Move conversion: white queen moves middle of board") {
    const Move move = GENERATE(loadQueenMoves<true>(0x10000000));

    const uint64_t sourceSquare = move & 0b111111;
    const uint64_t targetSquare = (move >> 6) & 0b111111;
    const uint64_t flags = 0b1011;

    const Action action = getMoveIndex<true>(move);
    const ActionInfo ai = parseAction<true>(action);

    const uint64_t flagsC = promotionToFlags(ai.promotion);

    REQUIRE(ai.sourceSquare == sourceSquare);
    REQUIRE(ai.targetSquare == targetSquare);
    REQUIRE(flagsC == flags);
}

TEST_CASE("Move conversion: white knight moves middle of board") {
    const Move move = GENERATE(loadKnightMoves<true>(0x10000000));

    const uint64_t sourceSquare = move & 0b111111;
    const uint64_t targetSquare = (move >> 6) & 0b111111;
    const uint64_t flags = 0b1011;

    const Action action = getMoveIndex<true>(move);
    const ActionInfo ai = parseAction<true>(action);

    const uint64_t flagsC = promotionToFlags(ai.promotion);

    REQUIRE(ai.sourceSquare == sourceSquare);
    REQUIRE(ai.targetSquare == targetSquare);
    REQUIRE(flagsC == flags);
}

TEST_CASE("Move conversion: white bishop moves middle of board") {
    const Move move = GENERATE(loadBishopMoves<true>(0x10000000));

    const uint64_t sourceSquare = move & 0b111111;
    const uint64_t targetSquare = (move >> 6) & 0b111111;
    const uint64_t flags = 0b1011;

    const Action action = getMoveIndex<true>(move);
    const ActionInfo ai = parseAction<true>(action);

    const uint64_t flagsC = promotionToFlags(ai.promotion);

    REQUIRE(ai.sourceSquare == sourceSquare);
    REQUIRE(ai.targetSquare == targetSquare);
    REQUIRE(flagsC == flags);
}

TEST_CASE("Move conversion: white rook moves middle of board") {
    const Move move = GENERATE(loadRookMoves<true>(0x10000000));

    const uint64_t sourceSquare = move & 0b111111;
    const uint64_t targetSquare = (move >> 6) & 0b111111;
    const uint64_t flags = 0b1011;

    const Action action = getMoveIndex<true>(move);
    const ActionInfo ai = parseAction<true>(action);

    const uint64_t flagsC = promotionToFlags(ai.promotion);

    REQUIRE(ai.sourceSquare == sourceSquare);
    REQUIRE(ai.targetSquare == targetSquare);
    REQUIRE(flagsC == flags);
}

TEST_CASE("Move conversion: white pawn moves middle of board") {
    const Move move = GENERATE(loadPawnMoves<true>(0x10000000));

    const uint64_t sourceSquare = move & 0b111111;
    const uint64_t targetSquare = (move >> 6) & 0b111111;
    const uint64_t flags = 0b1011;

    const Action action = getMoveIndex<true>(move);
    const ActionInfo ai = parseAction<true>(action);

    const uint64_t flagsC = promotionToFlags(ai.promotion);

    REQUIRE(ai.sourceSquare == sourceSquare);
    REQUIRE(ai.targetSquare == targetSquare);
    REQUIRE(flagsC == flags);
}

TEST_CASE("Move conversion: white pawn moves promotion") {
    const Move move = GENERATE(loadPawnMoves<true>(0x00FF000000000000));

    const uint64_t sourceSquare = move & 0b111111;
    const uint64_t targetSquare = (move >> 6) & 0b111111;
    const uint64_t flags = (move >> 12) & 0b1111;

    const Action action = getMoveIndex<true>(move);
    const ActionInfo ai = parseAction<true>(action);

    const uint64_t flagsC = promotionToFlags(ai.promotion);
    const Move convertedM = Movegen::create_move(ai.sourceSquare, ai.targetSquare, 0ull);

    REQUIRE(ai.sourceSquare == sourceSquare);
    REQUIRE(ai.targetSquare == targetSquare);
    REQUIRE(flagsC == flags);
}
