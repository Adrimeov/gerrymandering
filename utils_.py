import numpy as np
import sys
np.set_printoptions(threshold=sys.maxsize)
import pymetis



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


def generate_adjacency_matrix(nb_row, nb_column):
    adjacency_list = [[] for i in range(nb_row * nb_column)]
    for row_idx in range(nb_row):
        for col_idx in range(nb_column):
            coordinate_to_idx = (row_idx * nb_column + col_idx)
            coordinate_to_idx_list = coordinate_to_idx
            if row_idx == 0:
                if col_idx == 0:
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx + 1)
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx + nb_column)
                elif col_idx == nb_column - 1:
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx - 1)
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx + nb_column)
                else:
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx - 1)
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx + 1)
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx + nb_column)
            elif row_idx == nb_row - 1:
                if col_idx == 0:
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx - nb_column)
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx + 1)
                elif col_idx == nb_column - 1:
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx - 1)
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx - nb_column)
                else:
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx - nb_column)
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx - 1)
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx + 1)
            else:
                if col_idx == 0:
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx - nb_column)
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx + 1)
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx + nb_column)
                elif col_idx == nb_column -1:
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx - nb_column)
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx - 1)
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx + nb_column)
                else:
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx - nb_column)
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx - 1)
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx + 1)
                    adjacency_list[coordinate_to_idx_list].append(coordinate_to_idx + nb_column)

    return adjacency_list


def validate_solution(districts, n, k):
    min_distance = np.ceil(n/(2*k))
    min_nb_district = np.floor(n/k)
    max_nb_district = np.ceil(n/k)
    counter = 1
    for district_idx in range(len(districts)):
        if len(districts[district_idx]) > max_nb_district or len(districts[district_idx]) < min_nb_district:
            print(len(districts[district_idx]))
            print(district_idx)
            print("not valid too much city.")
            return 0
        for municipality_1_idx in range(len(districts[district_idx]) - 1):
            for municipality_2_idx in range(len(districts[district_idx])):
                distance_x = abs(districts[district_idx][municipality_1_idx][0] - districts[district_idx][municipality_2_idx][0])
                distance_y = abs(districts[district_idx][municipality_1_idx][1] - districts[district_idx][municipality_2_idx][1])
                if distance_x + distance_y > min_distance:
                    print("Actual distance: ", distance_x + distance_y, " Distance max accpeted: ", min_distance)
                    print(district_idx)
                    print("not valid; bust distance max.")
                    return 0

    print("Valid!")


def reconstruct_matrix_from_label(labels, nb_labels, nb_rows, nb_columns, print_=False):
    districts = [[] for i in range(nb_labels)]
    matrix = np.zeros((nb_rows, nb_columns))
    for label_idx in range(len(labels)):
        row = label_idx // nb_columns
        column = label_idx % nb_columns
        matrix[row][column] = labels[label_idx]
        districts[labels[label_idx]].append((row, column))
    if print_:
        print(matrix)
    return districts


if __name__ == "__main__":
    nb_rows = 100
    nb_columns = 10
    nb_district = 12
    a = generate_adjacency_matrix(nb_rows, nb_columns)
    b = np.arange(nb_columns*nb_rows).reshape(nb_rows, nb_columns)

    b, c = pymetis.part_graph(nb_district, adjacency=a)
    district = reconstruct_matrix_from_label(c, nb_district, nb_rows, nb_columns, print_=True)
    validate_solution(district, nb_rows*nb_columns,nb_district)
















