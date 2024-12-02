#include <chrono>
#include <vector>

#include "game_env.hpp"

std::vector<int> getActionsFromMask(const std::vector<bool>& mask) {
    std::vector<int> result;
    const int size = mask.size();
    for (int i = 0; i < size; ++i) {
        if (mask[i]) {
            result.push_back(i);
        }
    }
    return result;
}

uint64_t perft(int depth, ChessGameEnv& env) {
    if (depth == 0)
        return 1;  // Base case: at depth 0, it's just the current position

    uint64_t nodes = 0;

    ChessObservation obs = env.observe();

    // Check if game has ended (checkmate, stalemate, etc.)
    if (obs.isTerminated && !(obs.blackReward || obs.whiteReward)) {
        return 1;  // This is a terminal node, count it
    }

    const std::vector<bool> actionMask = obs.actionMask;
    const std::vector<int> moves = getActionsFromMask(actionMask);
    for (const auto& move : moves) {
        ChessGameEnv new_env = env;
        new_env.step(move);
        // Recursive call to explore further moves
        nodes += perft(depth - 1, new_env);
    }
    return nodes;
}

int testAll() {
    ChessGameEnv env;

    // Incrementally test depths 1 to 5
    for (int depth = 1; depth <= 5; ++depth) {
        auto start_time = std::chrono::high_resolution_clock::now();

        uint64_t nodes = perft(depth, env);

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_time = end_time - start_time;

        std::cout << "Depth: " << depth << " | Nodes: " << nodes
                  << " | Time: " << elapsed_time.count() << " seconds"
                  << std::endl;
    }

    return 0;
}
int testSingle(int depth) {
    ChessGameEnv env;

    auto start_time = std::chrono::high_resolution_clock::now();

    uint64_t nodes = perft(depth, env);

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_time = end_time - start_time;

    std::cout << "Depth: " << depth << " | Nodes: " << nodes
              << " | Time: " << elapsed_time.count() << " seconds" << std::endl;

    return 0;
}

int runSingle(int depth) {
    ChessGameEnv env;
    volatile uint64_t nodes = perft(depth, env);
    return 0;
}

int main() {
    // runSingle(5);
    // testSingle(7);
    testAll();
}
