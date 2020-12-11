import argparse
import sys
import sample_reader
import signal

from cpp_project import CppLib
from partitionning.main import initialize_districts
from partitionning.local_search_solver import solve_local_search

# (timeout 120s ./tp.sh -e ./exemplaires/10_10_0.txt -c 10) | python3 ./check_sol.py -e ./exemplaires/10_10_0.txt -c 10



def launch_algo(path=None, nb_districts=None, print_=True):
    nb_districts = int(nb_districts)
    rows, cols, municipalities, vote_map = sample_reader.read_samples(path)
    districts = initialize_districts(rows, cols, nb_districts, solve_local_search)

    assert len(districts) == nb_districts

    centers = []

    for i, district in enumerate(districts):
        center_x = 0
        center_y = 0
        for mun in district:
            x = mun.get_x()
            y = mun.get_y()
            votes = int(vote_map[x, y])
            mun.set_votes(votes)
            center_x += x
            center_y += y
        center_x /= len(district)
        center_y /= len(district)
        centers.append((center_x, center_y))

    solver = CppLib.Solver()

    try :
        solver.Optimise_Votes(districts, rows, cols, nb_districts, 500, centers, bool(print_))
    except:
        print("")


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

