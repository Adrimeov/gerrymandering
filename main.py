import argparse
import sys
import sample_reader
import K_means_experiment as K_mean
import numpy as np

from cpp_project import CppLib
from partitionning.main import initialize_districts
from partitionning.local_search_solver import solve_local_search

# (timeout 120s ./tp.sh -e ./exemplaires/10_10_0.txt -c 10) | python3 ./check_sol.py -e ./exemplaires/10_10_0.txt -c 10


def launch_algo(path, nb_districts, print_):
    nb_districts = int(nb_districts)
    rows, cols, municipalities, vote_map = sample_reader.read_samples(path)
    districts = initialize_districts(rows, cols, nb_districts, solve_local_search)

    assert len(districts) == nb_districts

    initialise_votes(districts, vote_map)
    print("nice")
    # TODO: launch votes local search


def initialise_votes(districts, vote_map):
    for district in districts:
        for mun in district:
            x = mun.get_x()
            y = mun.get_y()

            votes = int(vote_map[x, y])

            mun.set_votes(votes)

if __name__ == "__main__":
    if len(sys.argv) <= 1:
        exit('Erreur: Pas assez d\'arguments.')

    arguments = ["--path", "--nb_districts", "--print_"]
    parser = argparse.ArgumentParser()

    for argument in arguments:
        parser.add_argument(argument)

    args = parser.parse_args()
    args_as_dict = vars(args)
    parameters = {a: vars(args)[a] for a in vars(args) if vars(args)[a] != ""}
    launch_algo(**parameters)

