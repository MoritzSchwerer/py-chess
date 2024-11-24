#define CATCH_CONFIG_MAIN
#include <bitset>
#include <catch2/catch.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "game_env.hpp"
#include "lookup.hpp"
#include "utils.hpp"

struct TestCase {
    std::vector<int> actions_taken;
    bool isStaleCheck;
    bool canClaimDraw;
    bool isInsufficientMaterial;
    int line;
};

struct TestResult {
    int line;

    std::vector<int> actions_taken;

    bool isStaleCheck;
    bool canClaimDraw;
    bool isInsufficientMaterial;

    GameState state;
};

// Helper function to convert bool mask to vector of ints
std::vector<int> actionMaskToIndices(const std::vector<bool>& mask) {
    std::vector<int> result;
    for (int i = 0; i < mask.size(); ++i) {
        if (mask[i]) {
            result.push_back(i);
        }
    }
    return result;
}

// Helper function to split a comma-separated string into a vector of integers
std::vector<int> parseActionList(const std::string& action_list_str) {
    std::vector<int> actions;
    std::stringstream ss(action_list_str);
    std::string action;
    while (std::getline(ss, action, ',')) {
        actions.push_back(std::stoi(action));
    }
    return actions;
}

std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

// Helper function to parse a line into a TestCase struct
bool parseTestCase(const std::string& line, TestCase& test_case) {
    auto separator_pos = line.find(';');
    if (separator_pos == std::string::npos) {
        return false;
    }

    test_case.actions_taken = parseActionList(line.substr(0, separator_pos));
    std::string bool_values_str = line.substr(separator_pos + 1);
    auto bool_tokens = splitString(bool_values_str, ',');

    if (bool_tokens.size() != 3) {  // Ensure there are exactly 3 boolean values
        return false;
    }

    test_case.isStaleCheck = (bool_tokens[0] == "1");
    test_case.canClaimDraw = (bool_tokens[1] == "1");
    test_case.isInsufficientMaterial = (bool_tokens[2] == "1");

    return true;
}

// Generator function to load test cases from the file
std::vector<TestCase> loadTestCases(const std::string& file_path,
                                    int max_lines) {
    std::ifstream data_file(file_path);
    REQUIRE(data_file.is_open());

    std::vector<TestCase> test_cases;
    std::string line;
    int line_number = 0;

    while (std::getline(data_file, line) && line_number < max_lines) {
        line_number++;
        TestCase test_case;
        test_case.line = line_number;
        if (parseTestCase(line, test_case)) {
            test_cases.push_back(test_case);
        } else {
            FAIL("Parsing failed for line " + std::to_string(line_number));
        }
    }

    return test_cases;
}

// Overload operator<< for std::vector<int>
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vect) {
    os << "[";
    for (size_t i = 0; i < vect.size(); ++i) {
        os << vect[i];
        if (i != vect.size() - 1) {
            os << ", ";
        }
    }
    os << "] (" << std::to_string(vect.size()) << ")";
    return os;
}

std::vector<int> getMissingActions(const std::vector<int>& expected,
                                   const std::vector<int>& actual) {
    std::vector<int> missing;
    for (int i = 0; i < expected.size(); ++i) {
        bool foundMatch = false;
        for (int j = 0; j < actual.size(); ++j) {
            if (expected[i] == actual[j]) {
                foundMatch = true;
                break;
            }
        }
        if (!foundMatch) {
            missing.push_back(expected[i]);
        }
    }
    return missing;
}

std::vector<int> getExtraActions(const std::vector<int>& expected,
                                 const std::vector<int>& actual) {
    std::vector<int> extra;
    for (int i = 0; i < actual.size(); ++i) {
        bool foundMatch = false;
        for (int j = 0; j < expected.size(); ++j) {
            if (actual[i] == expected[j]) {
                foundMatch = true;
                break;
            }
        }
        if (!foundMatch) {
            extra.push_back(actual[i]);
        }
    }
    return extra;
}

std::string getActionInfo(const int action, bool isWhite) {
    const char files[] = "abcdefgh";
    std::string result = "";
    const uint16_t sourceFile = action / (NUM_ACTION_PLANES * 8);
    const uint16_t sourceRank = (action / NUM_ACTION_PLANES) % 8;
    // const uint16_t tempSourceSquare = sourceRank * 8 + sourceFile;
    const uint16_t plane = action % NUM_ACTION_PLANES;

    uint16_t sourceSquare =
        convertToColorSquare<true>(sourceFile + sourceRank * 8);
    int16_t offset = Lookup::getOffsetFromPlane<true>(plane);
    if (!isWhite) {
        offset = Lookup::getOffsetFromPlane<false>(plane);
        sourceSquare = convertToColorSquare<false>(sourceFile + sourceRank * 8);
    }
    uint16_t sf = sourceSquare % 8;
    uint16_t sr = sourceSquare / 8;

    // const int16_t offset = Lookup::getOffsetFromPlane<isWhite>(plane);
    result += files[sf] + std::to_string(sr) + " " + std::to_string(offset);
    return result;
}

std::string actionVectorToString(const std::vector<int>& vect, bool isWhite) {
    std::string result = "[";
    for (size_t i = 0; i < vect.size(); ++i) {
        // result += std::to_string(vect[i]) + " (" + getActionInfo(vect[i],
        // isWhite) + ")";
        result += "(" + getActionInfo(vect[i], isWhite) + ")";
        if (i != vect.size() - 1) {
            result += ", ";
        }
    }
    result += "] (" + std::to_string(vect.size()) + ")";
    return result;
}

void checkTestResult(const TestResult& tr) {}

TEST_CASE("Golden master tests for chess library actions", "[chess]") {
    // Load the test cases using the generator
    const auto test_cases =
        loadTestCases(DATA_DIR "/chess_early_termination_cases.txt", 1000);

    // Use Catch2's GENERATE feature to iterate over each test case
    const auto& test_case = GENERATE_REF(from_range(test_cases));

    SECTION("Test case line " + std::to_string(test_case.line)) {
        ChessGameEnv env;

        // Apply the sequence of actions to set up the game state
        for (int action : test_case.actions_taken) {
            env.step(action);
        }

        // Get the list of available actions from the library at this state
        auto observation = env.observe();
        // std::vector<bool> actionMask = observation.actionMask;
        // std::vector<int> actions = actionMaskToIndices(actionMask);
        bool isTerminated = observation.isTerminated;

        // if (isTerminated !=
        //     (test_case.isInsufficientMaterial || test_case.canClaimDraw ||
        //      test_case.isStaleCheck)) {
        if (!isTerminated && test_case.isInsufficientMaterial) {
            env.showBoard();
            std::cout << "Insufficient Material: "
                      << test_case.isInsufficientMaterial << "\n"
                      << "Can Claim Draw: " << test_case.canClaimDraw << "\n"
                      << "Stale/Checkmate: " << test_case.isStaleCheck
                      << std::endl;
            std::cout << "FEN: " << generateFEN(env.getState()) << std::endl;
            FAIL();
        }
        SUCCEED();
    }
}
