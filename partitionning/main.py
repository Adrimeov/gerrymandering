import numpy as np
import sys

from time import time
from enum import Enum
from math import ceil, floor, gcd
from cpp_project import CppLib

np.set_printoptions(threshold=sys.maxsize)

MAX_MUN_DISTICTS_RATIO = 20
SMALL_DIMENSION = 10


class Direction(Enum):
    X = 1
    Y = 2


def split_districts(sub_matrix, x_range, y_range, nb_districts, solver=None, districts=[], label_start=1):
    x_length = x_range[1] - x_range[0] + 1
    y_length = y_range[1] - y_range[0] + 1
    n = x_length * y_length

    k_min = n // nb_districts
    k_max = ceil(n / nb_districts)

    direction = get_split_axis(x_length, y_length)

    ranges = get_new_ranges(x_range, y_range, direction, nb_districts)

    # No possible split found, we solve
    if len(ranges) == 0:
        districts += solve_sub_matrix(sub_matrix, x_range, y_range, direction, n, nb_districts, k_min, k_max, label_start, solver)
        return

    for i, sub_range in enumerate(ranges):
        split_nb_districts, split_x_range, split_y_range = sub_range
        split_districts(sub_matrix, split_x_range, split_y_range, split_nb_districts, districts=districts, label_start=label_start + i*split_nb_districts)


def solve_sub_matrix(sub_matrix, x_range, y_range, direction, n, nb_districts, k_min, k_max, label_start, solver=None):
    try:
        # Solve with iteration
        if verify_sub_matrix(x_range, y_range, direction, n, nb_districts):
            return iterated_solve(sub_matrix, x_range, y_range, direction, n, k_min, k_max, label_start)
        else:
            raise ValueError("Sub_matrix cannot be solved manually")
    except:
        # TODO: make sure the provided solver gives a valid solution
        rows, cols, nb_districts = prepare_solver_parameters(x_range, y_range, nb_districts)
        return solver(rows, cols, nb_districts)


def prepare_solver_parameters(x_range, y_range, nb_districts):
    rows = x_range[1] - x_range[0] + 1
    cols = y_range[1] - y_range[0] - 1
    return rows, cols, nb_districts


def iterated_solve(sub_matrix, x_range, y_range, direction, n, k_min, k_max, label_start):
    left_municipalities = n
    nb_max_mun = 0

    while left_municipalities % k_min != 0:
        left_municipalities -= k_max
        nb_max_mun += 1

    assert left_municipalities % k_min == 0

    nb_min_mun = left_municipalities // k_min

    if direction == Direction.X:
        outer_range = x_range
        inner_range = y_range
    else:
        outer_range = y_range
        inner_range = x_range

    municipalities_placed = 0
    district_label = label_start

    districts = []
    municipalities = []

    for i in range(outer_range[0], outer_range[1] + 1):
        for j in range(inner_range[0], inner_range[1] + 1):
            if Direction == Direction.X:
                sub_matrix[i, j] = district_label
                municipalities.append(CppLib.Municipality(i, j, 0))
            else:
                sub_matrix[j, i] = district_label
                municipalities.append(CppLib.Municipality(j, i, 0))

            municipalities_placed += 1

            if nb_max_mun > 0 and municipalities_placed % k_max == 0:
                nb_max_mun -= 1
                district_label += 1
                municipalities_placed = 0
                districts.append(municipalities)
                municipalities = []
            elif nb_max_mun == 0 and municipalities_placed % k_min == 0:
                nb_min_mun -= 1
                district_label += 1
                municipalities_placed = 0
                districts.append(municipalities)
                municipalities = []

    return districts


def verify_sub_matrix(x_range, y_range, direction, n, nb_districts):
    is_valid = True

    max_distance = ceil(n / (2 * nb_districts))

    if direction == Direction.X:
        outer_range = x_range
        inner_range = y_range
    else:
        outer_range = y_range
        inner_range = x_range

    district_height = ceil((outer_range[1] - outer_range[0] + 1) / nb_districts)

    for district in range(1, nb_districts + 1):
        top_index = (outer_range[0] + (district - 1) * (district_height - 1), inner_range[1])
        bottom_index = (outer_range[0] + district * (district_height - 1), inner_range[0])
        distance = abs(top_index[1] - bottom_index[1]) + abs(top_index[0] - bottom_index[0])

        if distance > max_distance:
            return False

    return is_valid


def get_max_split(axis_size, nb_districts):
    max_split = gcd(axis_size, nb_districts)

    if max_split < 10:
        return max_split

    max_split = 10

    while axis_size % max_split != 0 or nb_districts % max_split != 0:
        max_split -= 1

    return max_split


def get_split_axis(x_length, y_length):
    if x_length >= y_length:
        return Direction.X
    else:
        return Direction.Y


def get_asymetric_ranges(x_range, y_range, split_direction, nb_districts, axis_size):
    ranges = []

    nb_districts_1 = ceil(nb_districts / 2)
    nb_districts_2 = floor(nb_districts / 2)

    axis_size_1 = axis_size * (nb_districts_1 / nb_districts)
    axis_size_2 = axis_size * (nb_districts_2 / nb_districts)

    while not float.is_integer(axis_size_1) or not float.is_integer(axis_size_2):
        nb_districts_1 += 1
        nb_districts_2 -= 1
        axis_size_1 = axis_size * (nb_districts_1 / nb_districts)
        axis_size_2 = axis_size * (nb_districts_2 / nb_districts)

    assert axis_size_1 + axis_size_2 == axis_size

    axis_size_1 = int(axis_size_1)

    if nb_districts_1 == nb_districts:
        return []

    if split_direction == Direction.X:
        split_x_range_1 = (x_range[0], x_range[0] + axis_size_1 - 1)
        split_x_range_2 = (x_range[0] + axis_size_1, x_range[1])
        split_y_range_1 = y_range
        split_y_range_2 = y_range
    else:
        split_x_range_1 = x_range
        split_x_range_2 = x_range
        split_y_range_1 = (y_range[0], y_range[0] + axis_size_1 - 1)
        split_y_range_2 = (y_range[0] + axis_size_1, y_range[1])

    ranges.append((nb_districts_1, split_x_range_1, split_y_range_1))
    ranges.append((nb_districts_2, split_x_range_2, split_y_range_2))

    return ranges


def get_new_ranges(x_range, y_range, split_direction, nb_districts):
    if split_direction == Direction.X:
        axis_size = x_range[1] - x_range[0] + 1
    else:
        axis_size = y_range[1] - y_range[0] + 1

    ranges = []

    if axis_size <= SMALL_DIMENSION and nb_districts % 2 != 0:
        ranges = get_asymetric_ranges(x_range, y_range, split_direction, nb_districts, axis_size)

    if len(ranges) > 0:
        return ranges

    splits = get_max_split(axis_size, nb_districts)

    if splits == 1:
        return []

    new_axis_size = axis_size // splits
    split_nb_districts = nb_districts // splits

    ranges = []

    for i in range(splits):
        if split_direction == Direction.X:
            x_start = x_range[0] + i * new_axis_size
            split_x_range = (x_start, x_start + new_axis_size - 1)
            split_y_range = y_range
        else:
            y_start = y_range[0] + i * new_axis_size
            split_y_range = (y_start, y_start + new_axis_size - 1)
            split_x_range = x_range
        ranges.append((split_nb_districts, split_x_range, split_y_range))

    return ranges


def show_municipalities(municipalities):
    for row in municipalities:
        for mun in row:
            print(f"{int(mun)} ", end="")
        print("")


def initialize_districts(rows, cols, nb_districts, solver):
    row_range = (0, rows - 1)
    col_range = (0, cols - 1)
    n = rows * cols

    assert n / nb_districts <= MAX_MUN_DISTICTS_RATIO
    matrix = np.zeros((rows, cols))
    split_districts(matrix, row_range, col_range, nb_districts, solver)


def experiment_municipalities_show(sub_matrix, districts):
    for i, district in enumerate(districts):
        for mun in district:
            x = mun.get_x()
            y = mun.get_y()
            sub_matrix[x, y] = i + 1


if __name__ == "__main__":
    rows = (0, 9)
    cols = (0, 9)
    nb_districts = 5

    n = rows[1] * cols[1]

    assert n / nb_districts <= MAX_MUN_DISTICTS_RATIO

    municipalities = np.zeros((rows[1] + 1, cols[1] + 1))

    districts = []

    start = time()
    split_districts(municipalities, rows, cols, nb_districts,  districts=districts, solver=None)
    print(f"Total time: {time() - start}")

    show_municipalities(municipalities)
