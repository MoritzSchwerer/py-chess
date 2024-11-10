import random
from _chess_env import ChessGameEnv
from pettingzoo.classic import chess_v6
from tqdm import tqdm

DATA_FILE = "chess_failure_cases.txt"  # Define the data file for failing test cases


def initialize_environments():
    """Initialize the custom and correct chess environments."""
    my_env = ChessGameEnv()
    correct_env = chess_v6.env()
    correct_env.reset()
    return my_env, correct_env


def get_player(i):
    """Determine the current player based on the move number."""
    return "player_0" if i % 2 == 0 else "player_1"


def fetch_observations(my_env, correct_env, player):
    """Fetch observations from both the custom and reference environments."""
    my_obs = my_env.observe()
    # c_obs = correct_env.observe(player)
    c_obs, _, termination, truncation, _ = correct_env.last()
    orig_actions = my_obs.actionMask.nonzero()[0]
    co_actions = c_obs["action_mask"].nonzero()[0]
    return my_obs, orig_actions, co_actions


def check_termination(my_obs):
    """Check if the game has terminated in the custom environment."""
    return my_obs.isTerminated


def compare_actions(orig_actions, co_actions, actions_taken):
    """Compare available actions between custom and reference environments."""
    if len(orig_actions) != len(co_actions):
        save_failure_case(actions_taken, co_actions)
        return False  # Return False to indicate a failure was logged
    return True


def save_failure_case(actions_taken, co_actions):
    """Save a failure case to the data file for later testing."""
    with open(DATA_FILE, "a") as f:
        # Write the actions leading up to the failure
        f.write(",".join(map(str, actions_taken)) + ";")

        # Write expected actions from the correct environment
        f.write(",".join(map(str, co_actions)) + "\n")


def execute_actions(my_env, correct_env, orig_actions, co_actions, actions_taken):
    """Execute a random action on both environments."""
    res = True
    action_idx = random.choice(range(len(orig_actions)))
    try:
        my_env.step(orig_actions[action_idx])
    except:
        res = False
    correct_env.step(co_actions[action_idx])
    actions_taken.append(co_actions[action_idx])
    return res


def play_single_game(max_moves=500):
    """Play a single game up to max_moves or until termination, logging any discrepancies."""
    my_env, correct_env = initialize_environments()
    actions_taken = []

    for move_count in range(max_moves):
        player = get_player(move_count)
        my_obs, orig_actions, co_actions = fetch_observations(
            my_env, correct_env, player
        )
        _, _, termination, truncation, _ = correct_env.last()

        if check_termination(my_obs) or termination or truncation:
            # print(
            #     f"Termination: {move_count}, my_res: {check_termination(my_obs)}, correct: {termination or truncation}"
            # )
            return  # Game ended normally

        if not compare_actions(orig_actions, co_actions, actions_taken):
            # print(f"wrong actions: {move_count}")
            return  # Stop this game on discrepancy, already logged

        if not execute_actions(
            my_env, correct_env, orig_actions, co_actions, actions_taken
        ):
            # print(f"error: {move_count}")
            return  # got an error in cpp


def main(n_games=100, max_moves=500):
    """Run multiple games, logging any failure cases."""
    for game_number in tqdm(range(n_games)):
        # print(f"Starting game {game_number + 1}")
        play_single_game(max_moves)


if __name__ == "__main__":
    main(n_games=100000)  # Specify the number of games to run
