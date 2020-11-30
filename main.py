import argparse
import sys
import sample_reader
from cpp_project import CppLib

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

    x, y, municipalities = sample_reader.read_samples(samples_path)
    state = CppLib.Valid_State_Local_Search(municipalities, y, x, nb_districts)

