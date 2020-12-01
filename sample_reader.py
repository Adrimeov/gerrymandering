import numpy as np
from cpp_project import CppLib

DATA_DIR = "./exemplaires"


def read_samples(path):
    municipalities = np.array([])

    with open(path, "r") as file:
        y, x = [int(i) for i in file.readline().split()]

        for i, line in enumerate(file):
            line = line.replace("\n", "")
            row = np.array([int(i) for i in line.split()])

            row_municipalities = [CppLib.Municipality(i, j, votes)
                                  for j, votes in enumerate(row)]

            municipalities = np.concatenate((municipalities, row_municipalities))

    return x, y, municipalities


if __name__ == "__main__":
    filename = f"{DATA_DIR}/6_7_0.txt"
    x, y, municipalities = read_samples(filename)
    counter = 0
    for i in range(len(municipalities)):
        print(municipalities[i].get_x(), " : ", municipalities[i].get_y())
        counter +=1
    print(counter)
    # assert x == 6
    # assert y == 7
    # assert municipalities.size == x * y
    #
    # municipality_1 = municipalities[0]
    # municipality_2 = municipalities[-1]
    #
    # assert municipality_1.get_x() == 0
    # assert municipality_1.get_y() == 0
    #
    # a = municipality_2.get_y()
    #
    # assert municipality_2.get_x() == x - 1
    # assert municipality_2.get_y() == y - 1
