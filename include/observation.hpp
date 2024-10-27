#pragma once

#include <stdint.h>
#include <vector>

struct ChessObservation {
    std::vector<bool> observation;
    std::vector<bool> actionMask;
    int32_t whiteReward;
    int32_t blackReward;
    bool isTerminated;

    ChessObservation() = default;
    ChessObservation(ChessObservation&& other) noexcept
        : observation(std::move(other.observation)),
          actionMask(std::move(other.actionMask)),
          whiteReward(other.whiteReward),
          blackReward(other.blackReward),
          isTerminated(other.isTerminated) {}

    ChessObservation(std::vector<bool>&& observation,
                     std::vector<bool>&& actionMask, int whiteReward,
                     int blackReward, bool isTerminated)
        : observation(std::move(observation)),
          actionMask(std::move(actionMask)),
          whiteReward(whiteReward),
          blackReward(blackReward),
          isTerminated(isTerminated) {}
};
