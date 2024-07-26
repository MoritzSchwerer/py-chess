#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <fstream>
#include <vector>
#include <string>

#include "types.hpp"
#include "json.hpp"
#include "move_gen.hpp"
#include "game_state.hpp"
#include "parse_san.hpp"

namespace {

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


class TestCaseGenerator final : public Catch::Generators::IGenerator<TestCase> {
    TestCases testCases;
    int currentIndex;
public:

    TestCaseGenerator() : testCases(parseTestCases()), currentIndex(0) {}

    TestCase const& get() const override;

    bool next() override {
        currentIndex++;
        return currentIndex < testCases.size();
    }
};

TestCase const& TestCaseGenerator::get() const {
    return testCases[currentIndex];
}

Catch::Generators::GeneratorWrapper<TestCase> loadTestCases() {
    return Catch::Generators::GeneratorWrapper<TestCase>(
        std::make_unique<TestCaseGenerator>()
    );
}

}

TEST_CASE("Move generation tests") {
    TestCase tc = GENERATE(loadTestCases());

    GameState state = parseFen(tc.fen);
    Moves genLegalMoves = Movegen::getLegalMoves(state);
    REQUIRE(genLegalMoves.size() == tc.moves.size());
}

