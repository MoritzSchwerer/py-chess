#include <array>
#include <vector>

#include "src/game_env.hpp"

// TODO: do testing with random legal actions
int main() {
    ChessGameEnv game;
    game.observe();
    game.step(8*numActionPlanes + 1);
    game.observe();
    game.step(63*numActionPlanes + 29);
    game.observe();
    game.step(24*numActionPlanes + 0);
    game.observe();
    game.step(49*numActionPlanes + 29);
    game.observe();
    game.step(32*numActionPlanes + 7);
    ChessObservation obs = game.observe();
    std::cout << obs.whiteReward << std::endl;
    std::cout << obs.blackReward << std::endl;
    std::cout << obs.isTerminated << std::endl;
    return 0;
}
