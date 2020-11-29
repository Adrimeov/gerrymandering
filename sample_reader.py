import numpy as np
from cpp_project import Municipality

DATA_DIR = "./exemplaires"


def read_samples(path):
    municipalities = np.array([])

    with open(path, "r") as file:
        x, y = [int(i) for i in file.readline().split()]

        for i, line in enumerate(file):
            line = line.replace("\n", "")
            row = np.array([int(i) for i in line.split()])

            row_municipalities = [Municipality.Municipality(j, i, votes)
                                  for j, votes in enumerate(row)]

            municipalities = np.concatenate((municipalities, row_municipalities))

    return x, y, municipalities


if __name__ == "__main__":
    filename = f"{DATA_DIR}/test_samples.txt"
    x, y, municipalities = read_samples(filename)

    assert x == 6
    assert y == 7
    assert municipalities.size == x * y

    municipality_1 = municipalities[0]
    municipality_2 = municipalities[-1]

    assert municipality_1.get_x() == 0
    assert municipality_1.get_y() == 0

    a = municipality_2.get_y()

    assert municipality_2.get_x() == x - 1
    assert municipality_2.get_y() == y - 1
