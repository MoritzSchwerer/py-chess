#include "game_env.hpp"

int main() {
    ChessGameEnv game;
    game.step(8*numActionPlanes + 1);
    game.step(63*numActionPlanes + 29);
    game.step(24*numActionPlanes + 0);
    game.step(49*numActionPlanes + 29);
    game.step(32*numActionPlanes + 7);

    return 0;
}
