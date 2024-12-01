#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include "game_env.hpp"

constexpr int NUM_TEST_CASES = 1000;

using Actions = std::vector<uint16_t>;
using GameActions = std::vector<Actions>;

enum class Termination : uint8_t { WHITE_WIN, BLACK_WIN, DRAW, TIMEOUT };

struct TestCase {
    Termination termination;
    Actions actionsTaken;
    GameActions perStepActions;
};

Actions actionMaskToIndices(std::vector<bool>& mask) {
    Actions actions;
    actions.reserve(mask.size());
    for (int i = 0; i < mask.size(); i++) {
        if (mask[i]) {
            actions.push_back(i);
        }
    }
    return actions;
}

TestCase generateSingleGame() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint16_t> actionDist(0, 4671);

    ChessGameEnv env;
    TestCase testCase;

    while (true) {
        ChessObservation obs = env.observe();

        // if terminated return correct termination
        if (obs.isTerminated) {
            if (obs.whiteReward == 1) {
                testCase.termination = Termination::WHITE_WIN;
            } else if (obs.blackReward == 1) {
                testCase.termination = Termination::BLACK_WIN;
            } else {
                testCase.termination = Termination::DRAW;
            }
            return testCase;
        }

        std::vector<bool> actionMask = obs.actionMask;
        Actions actions = actionMaskToIndices(actionMask);

        // pick a random index from the vector
        std::uniform_int_distribution<size_t> indexDist(0, actions.size() - 1);
        size_t randomIndex = indexDist(gen);
        uint16_t chosenAction = actions[randomIndex];

        testCase.actionsTaken.push_back(chosenAction);
        testCase.perStepActions.push_back(actions);

        env.step(chosenAction);
    }
    return testCase;
}

void saveTestCases(const std::string& filename,
                   const std::vector<TestCase>& testCases) {
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Failed to open file for writing: " +
                                 filename);
    }

    uint32_t numTestCases = static_cast<uint32_t>(testCases.size());
    outFile.write(reinterpret_cast<const char*>(&numTestCases),
                  sizeof(numTestCases));

    for (const auto& testCase : testCases) {
        // Write termination
        outFile.write(reinterpret_cast<const char*>(&testCase.termination),
                      sizeof(testCase.termination));

        // Write actionsTaken
        uint32_t actionsTakenSize =
            static_cast<uint32_t>(testCase.actionsTaken.size());
        outFile.write(reinterpret_cast<const char*>(&actionsTakenSize),
                      sizeof(actionsTakenSize));
        outFile.write(
            reinterpret_cast<const char*>(testCase.actionsTaken.data()),
            actionsTakenSize * sizeof(uint16_t));

        // Write perStepActions
        uint32_t perStepSize =
            static_cast<uint32_t>(testCase.perStepActions.size());
        outFile.write(reinterpret_cast<const char*>(&perStepSize),
                      sizeof(perStepSize));
        for (const auto& actions : testCase.perStepActions) {
            uint32_t legalActionsSize = static_cast<uint32_t>(actions.size());
            outFile.write(reinterpret_cast<const char*>(&legalActionsSize),
                          sizeof(legalActionsSize));
            outFile.write(reinterpret_cast<const char*>(actions.data()),
                          legalActionsSize * sizeof(uint16_t));
        }
    }
    outFile.close();
}

std::vector<TestCase> loadTestCases(const std::string& filename) {
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::vector<TestCase> testCases;

    uint32_t numTestCases = 0;
    inFile.read(reinterpret_cast<char*>(&numTestCases), sizeof(numTestCases));

    for (uint32_t i = 0; i < numTestCases; ++i) {
        TestCase testCase;

        // Read termination
        uint8_t termination;
        inFile.read(reinterpret_cast<char*>(&termination), sizeof(termination));
        testCase.termination = static_cast<Termination>(termination);

        // Read actionsTaken
        uint32_t actionsTakenSize;
        inFile.read(reinterpret_cast<char*>(&actionsTakenSize),
                    sizeof(actionsTakenSize));
        testCase.actionsTaken.resize(actionsTakenSize);
        inFile.read(reinterpret_cast<char*>(testCase.actionsTaken.data()),
                    actionsTakenSize * sizeof(uint16_t));

        // Read perStepActions
        uint32_t perStepSize;
        inFile.read(reinterpret_cast<char*>(&perStepSize), sizeof(perStepSize));
        testCase.perStepActions.resize(perStepSize);

        for (uint32_t j = 0; j < perStepSize; ++j) {
            uint32_t legalActionsSize;
            inFile.read(reinterpret_cast<char*>(&legalActionsSize),
                        sizeof(legalActionsSize));
            testCase.perStepActions[j].resize(legalActionsSize);
            inFile.read(
                reinterpret_cast<char*>(testCase.perStepActions[j].data()),
                legalActionsSize * sizeof(uint16_t));
        }

        testCases.push_back(std::move(testCase));
    }

    inFile.close();
    return testCases;
}

int main() {
    std::vector<TestCase> testCases;
    testCases.reserve(NUM_TEST_CASES);

    for (int i = 0; i < NUM_TEST_CASES; i++) {
        TestCase tc = generateSingleGame();
        testCases.push_back(tc);
        if ((i + 1) % 100 == 0) {
            std::cout << "Generated " << (i + 1) << "/" << NUM_TEST_CASES
                      << " test cases." << std::endl;
        }
    }

    saveTestCases(DATA_DIR "/testCases.bin", testCases);
    std::cout << "Saved the test cases." << std::endl;

    std::vector<TestCase> testCasesLoaded;
    testCasesLoaded = loadTestCases(DATA_DIR "/testCases.bin");
    std::cout << "Loaded the test cases successfully!" << std::endl;
}
