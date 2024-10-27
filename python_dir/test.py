import sys
import random
import numpy as np

from _chess_env import ChessGameEnv
from pettingzoo.classic import chess_v6
from pettingzoo.classic.chess.chess_utils import *

# TODO: I error on castling moves because I target the rook and pettingzoo
# targets the square the king will be on


def main():
    my_env = ChessGameEnv()
    correct_env = chess_v6.env()

    correct_env.reset()

    actions_taken = []

    for i in range(500):
        player = "player_0" if i % 2 == 0 else "player_1"

        my_obs = my_env.observe()
        c_obs = correct_env.observe(player)

        orig_actions = my_obs.actionMask.nonzero()[0]
        co_actions = c_obs["action_mask"].nonzero()[0]

        print(f"Move number: {i+1}")
        print(orig_actions)
        print(co_actions)
        # TODO: currently termination is not checked for correctness
        if my_obs.isTerminated:
            print("Terminated")
            sys.exit(0)
        if len(orig_actions) != len(co_actions):
            print("ERRORED: ")
            my_env.showBoard()

            try:
                indices = (orig_actions != co_actions).nonzero()[0]
            except:
                unique1 = np.setdiff1d(orig_actions, co_actions)
                unique2 = np.setdiff1d(co_actions, orig_actions)
                print(f"To many in my implementation: {unique1}")
                print(f"Not found by my implementation: {unique2}")

                print(f"Player to move: {'White' if player == 'player_0' else 'Black'}")
                if unique2.size > 0:
                    for u in unique2:
                        source_file = u // (73 * 8)
                        source_rank = (u // 73) % 8
                        plane = u % 73
                        print(f"{u}: Source={source_file},{source_rank}, Move={plane}")
                print("Actions taken to get to here: ")
                print(actions_taken)
            sys.exit(1)

        action_idx = random.choice(range(len(orig_actions)))
        my_env.step(orig_actions[action_idx])
        correct_env.step(co_actions[action_idx])
        actions_taken.append(co_actions[action_idx])


if __name__ == "__main__":
    main()
