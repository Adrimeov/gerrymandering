import numpy as np
import sys

from time import time
from enum import Enum
from math import ceil, floor, gcd

np.set_printoptions(threshold=sys.maxsize)

MAX_MUN_DISTICTS_RATIO = 20


class Direction(Enum):
    X = 1
    Y = 2


def split_districts(sub_matrix, x_range, y_range, nb_districts, label_start=1):
    x_length = x_range[1] - x_range[0] + 1
    y_length = y_range[1] - y_range[0] + 1
    n = x_length * y_length

    k_min = n // nb_districts
    k_max = ceil(n / nb_districts)

    direction = get_split_axis(x_length, y_length)

    # TODO: find better way to determine if sub_matrix is solvable
    if nb_districts <= 3:
        solve_sub_matrix(sub_matrix, x_range, y_range, direction, n, k_min, k_max, label_start)
        return

    # nb_districts_l, x_range_l, y_range_l, nb_districts_r, x_range_r, y_range_r =
    # get_new_ranges(x_range, y_range, direction, nb_districts)
    #
    # split_districts(sub_matrix, x_range_l, y_range_l, nb_districts_l, label_start)
    # split_districts(sub_matrix, x_range_r, y_range_r, nb_districts_r, label_start + nb_districts_r)

    ranges = get_new_ranges(x_range, y_range, direction, nb_districts)

    for i, sub_range in enumerate(ranges):
        split_nb_districts, split_x_range, split_y_range = sub_range
        split_districts(sub_matrix, split_x_range, split_y_range, split_nb_districts, label_start + i*split_nb_districts)


def solve_sub_matrix(sub_matrix, x_range, y_range, direction, n, k_min, k_max, label_start):
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

    for i in range(outer_range[0], outer_range[1] + 1):
        for j in range(inner_range[0], inner_range[1] + 1):
            if Direction == Direction.X:
                sub_matrix[i, j] = district_label
            else:
                sub_matrix[j, i] = district_label

            municipalities_placed += 1

            if nb_max_mun > 0 and municipalities_placed % k_max == 0:
                nb_max_mun -= 1
                district_label += 1
                municipalities_placed = 0
            elif nb_max_mun == 0 and municipalities_placed % k_min == 0:
                nb_min_mun -= 1
                district_label += 1
                municipalities_placed = 0


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


def get_new_ranges(x_range, y_range, split_direction, nb_districts):
    if split_direction == Direction.X:
        axis_size = x_range[1] - x_range[0] + 1
    else:
        axis_size = y_range[1] - y_range[0] + 1

    # nb_districts_l = ceil(nb_districts / 2)
    # nb_districts_r = nb_districts // 2
    #
    # ratio = nb_districts_r / nb_districts_l

    # TODO: take care of odd sizes
    splits = get_max_split(axis_size, nb_districts)

    if splits == 1:
        return


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


if __name__ == "__main__":
    rows = (0, 5)
    cols = (0, 49)
    nb_districts = 40

    n = rows[1] * cols[1]

    assert n / nb_districts <= MAX_MUN_DISTICTS_RATIO

    municipalities = np.zeros((rows[1] + 1, cols[1] + 1))

    start = time()
    split_districts(municipalities, rows, cols, nb_districts)
    print(f"Total time: {time() - start}")

    show_municipalities(municipalities)
