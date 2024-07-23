import random
import numpy as np

from chess_env import ChessGameEnv
from pettingzoo.classic import chess_v6
from pettingzoo.classic.chess.chess_utils import *


OFFSETS_WHITE = np.array(
    [
        8,
        16,
        24,
        32,
        40,
        48,
        56,
        9,
        18,
        27,
        36,
        45,
        54,
        63,
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        -7,
        -14,
        -21,
        -28,
        -35,
        -42,
        -49,
        -8,
        -16,
        -24,
        -32,
        -40,
        -48,
        -56,
        -9,
        -18,
        -27,
        -36,
        -45,
        -54,
        -63,
        -1,
        -2,
        -3,
        -4,
        -5,
        -6,
        -7,
        7,
        14,
        21,
        28,
        35,
        42,
        49,
        15,
        17,
        10,
        -6,
        -15,
        -17,
        6,
        -10,
        7,
        7,
        7,
        8,
        8,
        8,
        9,
        9,
        9,
    ]
)
OFFSETS_BLACK = np.array(
    [
        8,
        16,
        24,
        32,
        40,
        48,
        56,
        9,
        18,
        27,
        36,
        45,
        54,
        63,
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        -7,
        -14,
        -21,
        -28,
        -35,
        -42,
        -49,
        -8,
        -16,
        -24,
        -32,
        -40,
        -48,
        -56,
        -9,
        -18,
        -27,
        -36,
        -45,
        -54,
        -63,
        -1,
        -2,
        -3,
        -4,
        -5,
        -6,
        -7,
        7,
        14,
        21,
        28,
        35,
        42,
        49,
        15,
        17,
        10,
        -6,
        -15,
        -17,
        6,
        -10,
        -9,
        -9,
        -9,
        -8,
        -8,
        -8,
        -7,
        -7,
        -7,
    ]
)


def my_plane_to_offset(plane, isWhite: bool):
    if isWhite:
        return OFFSETS_WHITE[plane]
    else:
        return OFFSETS_BLACK[plane]


def get_move_plane(source, dest):
    difference = diff(square_to_coord(source.item()), square_to_coord(dest.item()))

    QUEEN_MOVES = 56
    KNIGHT_MOVES = 8
    QUEEN_OFFSET = 0
    KNIGHT_OFFSET = QUEEN_MOVES
    UNDER_OFFSET = KNIGHT_OFFSET + KNIGHT_MOVES

    if is_knight_move(difference):
        return KNIGHT_OFFSET + get_knight_dir(difference)
    else:
        return QUEEN_OFFSET + get_queen_plane(difference)


def map_action(action, isWhite):
    source = action // 73
    file = source % 8
    rank = source // 8
    plane = action % 73
    offset = my_plane_to_offset(plane, isWhite)

    target = source + offset
    new_plane = get_move_plane(source, target)

    return file * 584 + rank * 73 + new_plane


def main():
    my_env = ChessGameEnv()
    correct_env = chess_v6.env()

    correct_env.reset()

    for i in range(10):
        player = "player_0" if i % 2 == 0 else "player_1"
        isWhite = i % 2 == 0

        my_obs = my_env.observe()
        c_obs = correct_env.observe(player)

        orig_actions = my_obs.actionMask.nonzero()[0]
        co_actions = c_obs["action_mask"].nonzero()[0]

        my_actions = np.array([map_action(a, isWhite) for a in orig_actions])

        sort_indices = my_actions.argsort()
        inv_indices = [0] * len(sort_indices)
        for i in range(len(sort_indices)):
            inv_indices[sort_indices[i]] = i

        my_actions.sort()
        co_actions.sort()

        print(my_actions.shape)
        print(co_actions.shape)
        if not np.all(my_actions == co_actions):
            print(my_actions)
            print(co_actions)
            raise AssertionError("Whoops something went wrong")

        action_idx = random.choice(range(len(my_actions)))
        inv_index = inv_indices[action_idx]
        my_env.step(orig_actions[inv_index])
        correct_env.step(co_actions[action_idx])

    # envs = [ChessGameEnv() for _ in range(128 * 4)]
    # for it in trange(500):
    #     for i, env in enumerate(envs):
    #         for _ in range(1):
    #             tenv = env.copy()
    #             obs = tenv.observe()
    #             if obs.isTerminated:
    #                 continue
    #             action = int(random.choice(obs.actionMask.nonzero()[0]))
    #             tenv.step(action)


if __name__ == "__main__":
    main()
