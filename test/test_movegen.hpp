#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <catch2/catch.hpp>

#include "types.hpp"
#include "json.hpp"
#include "moves.hpp"
#include "move_gen.hpp"
#include "game_state.hpp"
#include "parse_san.hpp"


struct TestCase {
    std::string desc;

    std::string fen;
    std::vector<std::string> sans;

    GameState state;
    Moves moves;
};

using TestCases = std::vector<TestCase>;

TestCases readTestCaseFile(const std::string &f, TestCases &testCases) {
    std::ifstream file(f);
    std::string json((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    nlohmann::json j = nlohmann::json::parse(json);

    for (const auto& testCase : j["testCases"]) {
        TestCase tc;
        tc.fen = testCase["start"]["fen"];
        tc.state = parseFen(tc.fen);

        tc.desc = testCase["start"]["description"];
        for (const auto& expected : testCase["expected"]) {
            tc.sans.push_back(expected["move"]);
        }
        if (tc.state.status.isWhite) {
            tc.moves = sansToMoves<true>(tc.sans, tc.state);
        } else {
            tc.moves = sansToMoves<false>(tc.sans, tc.state);
        }
        testCases.push_back(tc);
    }
    return testCases;
}

TestCases parseTestCases() {
    std::vector<std::string> files {
        "test/data/castling.json",
        "test/data/famous.json",
        "test/data/pawns.json",
        "test/data/promotions.json",
        "test/data/taxing.json",
        "test/data/standard.json",
    };
    TestCases testCases;
    for (const std::string &f : files) readTestCaseFile(f, testCases);
    return testCases;
}

void runTestCase(const TestCase& tc) {
    GameState state = parseFen(tc.fen);
    Moves genLegalMoves = Movegen::getLegalMoves(state);

    Moves actLegalMoves = tc.moves;


    if (genLegalMoves.size() != actLegalMoves.size()) {
        std::cout << "=================================" << std::endl;
        std::cout << tc.fen << std::endl;
        std::cout << "Generated legal moves: " << genLegalMoves.size() << std::endl;
        std::cout << "Actually  legal moves: " << actLegalMoves.size() << std::endl;
        Bitboard mask = 0ull;
        if (state.status.isWhite) mask = Movegen::getSeenSquares<false>(state);
        else mask = Movegen::getSeenSquares<true>(state);
        std::cout << (state.status.isWhite ? "White" : "Black") << std::endl;
        printBoard(state, mask);
        std::cout << "Castle white: " << state.status.wQueenC << " " << state.status.wKingC << std::endl;
        std::cout << "Castle black: " << state.status.bQueenC << " " << state.status.bKingC << std::endl;
        std::cout << "My moves: " << std::endl;
        for (const Move& m : genLegalMoves) {
            printMove(m);
        }
        std::cout << "" << std::endl;
        std::cout << "Actual moves: " << std::endl;
        for (const Move& m : actLegalMoves) {
            printMove(m);
        }
        // for (const std::string& san : tc.sans) {
        //     std::cout << san << std::endl;
        // }
    }
}
