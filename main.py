import argparse
import sys
import sample_reader
from cpp_project import CppLib
import K_means_experiment as K_mean
import numpy as np
# / Users / adrimeov / Desktop / exemplaires_tests /
# (timeout 120s ./tp.sh -e ./exemplaires/10_10_0.txt -c 10) | python3 ./check_sol.py -e ./exemplaires/10_10_0.txt -c 10
# 36_60_m-300.txt nb mun = 8
# 25_20_0.txt nb_mun = 25
def launch_algo(path=None, nb_districts=None, print_=False):
    max_improving = 100
    x, y, municipalities = sample_reader.read_samples(path)
    centers = K_mean.compute_k_means_center(x, y, int(nb_districts))
    # centers = np.round(np.array(centers))
    CppLib.Valid_State_Local_Search(municipalities, x, y, int(nb_districts), max_improving, centers, bool(print_))


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

