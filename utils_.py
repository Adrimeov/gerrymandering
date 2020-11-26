import numpy as np
import sys
np.set_printoptions(threshold=sys.maxsize)


def priority_matrix(dim_x, dim_y, municipalities_coordinates, max_distance):
    nb_municipalities = len(municipalities_coordinates)
    municipalities_priority = np.zeros((dim_x, dim_y))
    for i in range(len(municipalities_coordinates)):
        for j in range(len(municipalities_coordinates)):
            distance_x = np.abs(municipalities_coordinates[i][0] - municipalities_coordinates[j][0])
            distance_y = np.abs(municipalities_coordinates[i][1] - municipalities_coordinates[j][1])
            distance = distance_x + distance_y
            if distance <= max_distance:
                municipalities_priority[municipalities_coordinates[i][0], municipalities_coordinates[i][1]] += 1
    return municipalities_priority


def custom_convert_matrix_to_list(matrix):

    priority_coordinates_list = []
    for i in range(matrix.shape[0]):
        for j in range(matrix.shape[1]):
            priority_coordinates_list.append((matrix[i][j], i, j))

    return priority_coordinates_list


def spiral_matrix_print(arr):
    # Defining the boundaries of the matrix.
    row = arr.shape[0]
    col = arr.shape[1]
    top = 0
    bottom = row - 1
    left = 0
    right = col - 1
    indices = []
    # Defining the direction in which the array is to be traversed.
    dir = 0

    while (top <= bottom and left <= right):
        if dir == 0:
            for i in range(left, right + 1):  # moving left->right
                indices.append((top, i))
            # Since we have traversed the whole first
            # row, move down to the next row.
            top += 1
            dir = 1

        elif dir == 1:
            for i in range(top, bottom + 1):  # moving top->bottom
                indices.append((i, right))

            # Since we have traversed the whole last
            # column, move down to the previous column.
            right -= 1
            dir = 2

        elif dir == 2:
            for i in range(right, left - 1, -1):  # moving right->left
                indices.append((bottom, i))

            # Since we have traversed the whole last
            # row, move down to the previous row.
            bottom -= 1
            dir = 3

        elif dir == 3:
            for i in range(bottom, top - 1, -1):  # moving bottom->top
                indices.append((i, left))
            # Since we have traversed the whole first
            # column, move down to the next column.
            left += 1
            dir = 0

    return indices


if __name__ == "__main__":
    test_arr = np.zeros((4,5))
    indices = spiral_matrix_print(test_arr)
    print(indices)