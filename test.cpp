#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "movegen.hpp"
#include "utils.hpp"

TEST_CASE("SeenSquares: Initial board white") {
    GameState state;
    Bitboard seenSquares = getSeenSquares<true>(state);
    REQUIRE(seenSquares == 0x0000000000FFFF7e);
}
TEST_CASE("SeenSquares: Initial board black") {
    GameState state;
    Bitboard seenSquares = getSeenSquares<false>(state);
    REQUIRE(seenSquares == 0x7eFFFF0000000000);
}

TEST_CASE("SquareOf empty returns 64") {
    const Bitboard board = 0ull;
    const uint64_t square = _tzcnt_u64(board);
    const Bitboard mask = board >> square;
    REQUIRE(square == 64);
    REQUIRE(mask == 0ull);
}

TEST_CASE("BroadcastBit all 0s") {
    const Bitboard board = broadcastBit(0ull);
    REQUIRE(board == 0ull);
}

TEST_CASE("BroadcastBit all 1s") {
    const Bitboard board = broadcastBit(1ull);
    REQUIRE(board == 0xffffffffffffffff);
}

TEST_CASE("BroadcastSingleToMask single one") {
    const Bitboard board = 0b000000000000000000010000000000000;
    const Bitboard broadcasted = broadcastSingleToMask(board);
    REQUIRE(broadcasted == 0xffffffffffffffff);
}

TEST_CASE("BroadcastSingleToMask multiple ones") {
    const Bitboard board = 0b000100010000000000010000000100000;
    const Bitboard broadcasted = broadcastSingleToMask(board);
    REQUIRE(broadcasted == 0xffffffffffffffff);
}

TEST_CASE("BroadcastSingleToMask all zeros") {
    const Bitboard board = 0b000000000000000000000000000000000;
    const Bitboard broadcasted = broadcastSingleToMask(board);
    REQUIRE(broadcasted == 0ull);
}

// TODO: test legal pawn moves



// takes king and pawn possition in fen notation
#define GENERATE_PAWN_CHECKMASK_TEST(pawn, king, isCheck) \
TEST_CASE("Pawn on " + std::string(pawn) + " " + (isCheck ? "attacks" : "does not attack") + " king on " + std::string(king), "[checkmask][pawn]") { \
    GameState state = GameStateEmpty(); \
    int kingSquare = fenPosToIndex(king); \
    int pawnSquare = fenPosToIndex(pawn); \
    state.w_king = 1ull << kingSquare; \
    state.b_pawn = 1ull << pawnSquare; \
    const Bitboard checkMask = getCheckMask<true>(state); \
    const Bitboard result = isCheck ? state.b_pawn : 0xffffffffffffffff; \
    REQUIRE(checkMask == result); \
}

// should all be checks
GENERATE_PAWN_CHECKMASK_TEST("e4", "d3", true);
GENERATE_PAWN_CHECKMASK_TEST("c4", "d3", true);
GENERATE_PAWN_CHECKMASK_TEST("h4", "g3", true);
GENERATE_PAWN_CHECKMASK_TEST("e2", "d1", true);
GENERATE_PAWN_CHECKMASK_TEST("c2", "d1", true);
GENERATE_PAWN_CHECKMASK_TEST("h2", "g1", true);
GENERATE_PAWN_CHECKMASK_TEST("c7", "b6", true);

// should not be checks
GENERATE_PAWN_CHECKMASK_TEST("g4", "g3", false);
GENERATE_PAWN_CHECKMASK_TEST("h4", "a3", false);
GENERATE_PAWN_CHECKMASK_TEST("c4", "g3", false);
GENERATE_PAWN_CHECKMASK_TEST("c2", "d3", false);



#define GENERATE_KNIGHT_CHECKMASK_TEST(knight, king, isCheck) \
TEST_CASE("Knight on " + std::string(knight) + " " + (isCheck ? "attacks" : "does not attack") + " king on " + std::string(king), "[checkmask][knight]") { \
    GameState state = GameStateEmpty(); \
    int kingSquare = fenPosToIndex(king); \
    int knightSquare = fenPosToIndex(knight); \
    state.w_king = 1ull << kingSquare; \
    state.b_knight = 1ull << knightSquare; \
    const Bitboard checkMask = getCheckMask<true>(state); \
    const Bitboard result = isCheck ? state.b_knight : 0xffffffffffffffff; \
    REQUIRE(checkMask == result); \
}

// TODO: generate more
// should all be checks
GENERATE_KNIGHT_CHECKMASK_TEST("e2", "f4", true);



#define GENERATE_ROOK_CHECKMASK_TEST(rook, king, expectedCheckMask) \
TEST_CASE("CheckMask<true> rook on " + std::string(rook) + " attacks king on " + std::string(king), "[checkmask][rook]") { \
    GameState state; \
    int kingSquare = fenPosToIndex(king); \
    int rookSquare = fenPosToIndex(rook); \
    state.w_king = 1ull << kingSquare; \
    state.b_pawn = 0x0; \
    state.b_rook = 1ull << rookSquare; \
    const Bitboard checkMask = getCheckMask<true>(state); \
    REQUIRE(checkMask == expectedCheckMask); \
}

GENERATE_ROOK_CHECKMASK_TEST("e6", "e3", 0x00101010000000);



#define GENERATE_BISHOP_CHECKMASK_TEST(bishop, king, expectedCheckMask) \
TEST_CASE("CheckMask<true> bishop on " + std::string(bishop) + " attacks king on " + std::string(king), "[checkmask][bishop]") { \
    GameState state; \
    int kingSquare = fenPosToIndex(king); \
    int bishopSquare = fenPosToIndex(bishop); \
    state.w_king = 1ull << kingSquare; \
    state.b_bishop = 1ull << bishopSquare; \
    const Bitboard checkMask = getCheckMask<true>(state); \
    REQUIRE(checkMask == expectedCheckMask); \
}
GENERATE_BISHOP_CHECKMASK_TEST("g7", "b2", 0x40201008040000);



#define GENERATE_PINMASK_HV_TEST(piece, blocker, expectedPinMask) \
TEST_CASE(std::string("PinMask<true> HV ") + "rook on " + std::string(piece) + ", blocker on " + std::string(blocker), "[pinmask][" + std::string(piece) + "]") { \
    GameState state; \
    int pieceSquare = fenPosToIndex(piece); \
    int blockerSquare = fenPosToIndex(blocker); \
    state.b_rook = 1ull << pieceSquare; \
    state.w_pawn = 1ull << blockerSquare; \
    state.b_queen = 0ull; \
    const Bitboard pinMask = getPinMaskHV<true>(state); \
    REQUIRE(pinMask == expectedPinMask); \
}

GENERATE_PINMASK_HV_TEST("e6", "e5", 0x101010101000);
GENERATE_PINMASK_HV_TEST("e6", "d5", 0ull);
GENERATE_PINMASK_HV_TEST("e2", "d5", 0ull);
GENERATE_PINMASK_HV_TEST("e3", "e2", 0x101000);



#define GENERATE_PINMASK_DG_TEST(piece, blocker, expectedPinMask) \
TEST_CASE(std::string("PinMask<true> DG ") + "rook on " + std::string(piece) + ", blocker on " + std::string(blocker), "[pinmask][" + std::string(piece) + "]") { \
    GameState state; \
    int pieceSquare = fenPosToIndex(piece); \
    int blockerSquare = fenPosToIndex(blocker); \
    state.b_bishop = 1ull << pieceSquare; \
    state.w_pawn = 1ull << blockerSquare; \
    state.b_queen = 0ull; \
    const Bitboard pinMask = getPinMaskDG<true>(state); \
    REQUIRE(pinMask == expectedPinMask); \
}

GENERATE_PINMASK_DG_TEST("h4", "g3", 0x80402000);
GENERATE_PINMASK_DG_TEST("h4", "g5", 0ull);
GENERATE_PINMASK_DG_TEST("a5", "d2", 0x102040800);
GENERATE_PINMASK_DG_TEST("a6", "d2", 0ull);
GENERATE_PINMASK_DG_TEST("e2", "e3", 0ull);


TEST_CASE("A pinned knight can never move") {
    GameState state = parseFen("8/8/8/4r3/8/4N3/8/4K3");
    std::vector<Move> moves;
    getLegalKnightMoves<true>(state, moves);
    REQUIRE(moves.size() == 0);
}

TEST_CASE("The knight has to capture") {
    GameState state = parseFen("8/8/8/4r3/8/5N2/8/4K3");
    std::vector<Move> moves;
    getLegalKnightMoves<true>(state, moves);
    REQUIRE(moves.size() == 1);
    Move move = moves[0];
    REQUIRE((move       & 0b111111) == SquareOf(state.w_knight));
    REQUIRE((move >> 6  & 0b111111) == SquareOf(state.b_rook));
    REQUIRE((move >> 12 & 0b1111  ) == 0b0100);

}
