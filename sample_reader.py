import numpy as np

DATA_DIR = "./exemplaires"


def read_samples(path):
    municipalities = np.array([])

    with open(path, "r") as file:
        x, y = [int(i) for i in file.readline().split()]

        for line in file:
            line = line.replace("\n", "")
            row = np.array([int(i) for i in line.split()])
            municipalities = np.concatenate((municipalities, row))

    return x, y, municipalities


if __name__ == "__main__":

    filename = f"{DATA_DIR}/test_samples.txt"

    x, y, samples = read_samples(filename)

    assert x == 6
    assert y == 7
    assert samples.size == x * y

