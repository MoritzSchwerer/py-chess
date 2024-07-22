#include <vector>
#include <iostream>

#include "src/game_env.hpp"

uint64_t firstLegalAction(const std::vector<bool>& actions) {
    for (int i = 0; i < actions.size(); i++) {
        if (actions[i]) return i; 
    }
    return actions.size();
}

int testFirstLegalAction() {
    ChessGameEnv game;
    while (true) {
        const ChessObservation obs = game.observe();
        const uint64_t action = firstLegalAction(obs.actionMask);
        if (action == obs.actionMask.size()) {
            break;
        }
        game.step(action);

    }
    return 0;
}

int main() {
    if (testFirstLegalAction() == 0) {
        std::cout << "success" << std::endl;
    } else {
        std::cout << "failed" << std::endl;
    }
    return 0;
}
