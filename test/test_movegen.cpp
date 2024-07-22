#include <fstream>
#include <vector>
#include <string>
#include <iostream>

#include "json.hpp"
#include "types.hpp"
#include "utils.hpp"
#include "parsesan.hpp"


struct TestCase {
    std::string desc;

    std::string fen;
    std::vector<std::string> sans;

    GameState state;
    Moves moves;
};

typedef std::vector<TestCase> TestCases;

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
        if (tc.state.isWhite) {
            tc.moves = sansToMoves<true>(tc.sans, tc.state);
        } else {
            tc.moves = sansToMoves<false>(tc.sans, tc.state);
        }
        testCases.push_back(tc);
    }
    return testCases;
}

int main() {
    std::vector<std::string> files {
        "examples.json",
    };
    TestCases testCases;
    for (const std::string &f : files) readTestCaseFile(f, testCases);

    for (const TestCase &tc : testCases) {
        std::cout << tc.desc << std::endl;
    }
    return 0;
}
