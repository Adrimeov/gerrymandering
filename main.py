import argparse
import sys
import sample_reader
from cpp_project import CppLib
import K_means_experiment as K_mean
if __name__ == "__main__":
    if len(sys.argv) <= 1:
        exit('Erreur: Pas assez d\'arguments.')

    arguments = ["--path", "--nb_districts", "--print"]
    parser = argparse.ArgumentParser()

    for argument in arguments:
        parser.add_argument(argument)

    args = parser.parse_args()
    args_as_dict = vars(args)
    parameters = {a: vars(args)[a] for a in vars(args) if vars(args)[a] != ""}

    samples_path = parameters["path"]
    nb_districts = int(parameters["nb_districts"])
    print_bool = bool(parameters["print"])
    max_improving = 100;
    x, y, municipalities = sample_reader.read_samples(samples_path)
    print(x)
    print(y)
    centers = K_mean.compute_k_means_center(y, x, nb_districts)
    # CppLib.test_initialize(municipalities, x, y, nb_districts, centers)
    valid = False
    while not valid:
        valid =CppLib.Valid_State_Local_Search(municipalities, y, x, nb_districts,max_improving, centers, print_bool)

