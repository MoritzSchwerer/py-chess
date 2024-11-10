#include <iostream>

#include "game_env.hpp"

int main() {
    ChessGameEnv env(
        "2q3r1/r1ppn3/b4kpn/BQbP1p1p/5pP1/1P2P2P/P1P1K1BR/2R2NN1 w - - 3 30");
    // auto obs = env.observe();
    env.showBoard();
    std::cout << "Test" << std::endl;
    return 0;
}
