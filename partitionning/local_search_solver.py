import numpy as np
import K_means_experiment as k_means

from cpp_project import CppLib


def solve_local_search(rows, cols, nb_districts):
    municipalities = prepare_municipalities(rows, cols)

    centers = k_means.compute_k_means_center(rows, cols, int(nb_districts))
    centers = np.round(np.array(centers))
    districts = CppLib.Valid_State_Local_Search(municipalities, rows, cols, int(nb_districts), 100, centers, False)

    return districts


def prepare_municipalities(rows, cols):
    municipalities = []

    for i in range(rows):
        for j in range(cols):
            municipality = CppLib.Municipality(i, j, 100)

            municipalities.append(municipality)

    return municipalities


if __name__ == "__main__":
    rows = 10
    cols = 10
    nb_districts = 5

    muns = prepare_municipalities(rows, cols)

    print(f"nb muns {len(muns)}")