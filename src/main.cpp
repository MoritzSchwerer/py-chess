#include <iostream>
#include <vector>

#include "game_env.hpp"
#include "utils.hpp"

int getFirstIndex(std::vector<bool> &actionMask) {
  for (int i = 0; i < actionMask.size(); i++) {
    if (actionMask[i])
      return i;
  }
  return actionMask.size();
}

int main() {
  ChessGameEnv env;
  ChessObservation obs = env.observe();
  env.step(getFirstIndex(obs.actionMask));
  return 0;
}
