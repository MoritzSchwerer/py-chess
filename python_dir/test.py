import sys
import random

from chess_env import ChessGameEnv
from pettingzoo.classic import chess_v6
from pettingzoo.classic.chess.chess_utils import *

# TODO: I error on castling moves because I target the rook and pettingzoo
# targets the square the king will be on


def main():
    my_env = ChessGameEnv()
    correct_env = chess_v6.env()

    correct_env.reset()

    for i in range(500):
        player = "player_0" if i % 2 == 0 else "player_1"

        my_obs = my_env.observe()
        c_obs = correct_env.observe(player)

        orig_actions = my_obs.actionMask.nonzero()[0]
        co_actions = c_obs["action_mask"].nonzero()[0]

        print(f"Move number: {i+1}")
        print(orig_actions)
        print(co_actions)
        # assert (
        #     len(orig_actions) == len(co_actions)
        # ), f"Man, lengths don't match up, got: {len(orig_actions)} and {len(co_actions)}"
        # assert np.all(orig_actions == co_actions), "whoops, error"
        if my_obs.isTerminated:
            print("Terminated")
            sys.exit(0)
        if len(orig_actions) != len(co_actions):
            print("ERRORED: ")
            my_env.showBoard()

            indices = (orig_actions != co_actions).nonzero()[0]
            for idx in indices:
                my_action = orig_actions[idx]
                correct_action = co_actions[idx]
                print()
                print("don't agree on index: ", idx)
                print(f"My value: {my_action}")
                print(f"Correct value: {correct_action}")

                source_file = my_action // (73 * 8)
                source_rank = (my_action // 73) % 8
                plane = my_action % 73
                print(f"My stats: {source_file}, {source_rank}, {plane}")

                source_file = correct_action // (73 * 8)
                source_rank = (correct_action // 73) % 8
                plane = correct_action % 73
                print(f"Correct stats: {source_file}, {source_rank}, {plane}")

            sys.exit(1)

        action_idx = random.choice(range(len(orig_actions)))
        my_env.step(orig_actions[action_idx])
        correct_env.step(co_actions[action_idx])


if __name__ == "__main__":
    main()
