import numpy as np
from sklearn.cluster import KMeans
import utils_


def validate_distance(coordinate_1, coordinate_2, max_distance):
    delta_x = np.abs(coordinate_1[0] - coordinate_2[0])
    delta_y = np.abs(coordinate_1[1] - coordinate_2[1])
    return (delta_x + delta_y) <= max_distance


x = np.arange(6)
y = np.arange(6)
nb_of_municipalites = len(x) * len(y)
nb_circonscription = 7
min_municipalite_in_circonscription = int(np.floor(nb_of_municipalites / nb_circonscription))
max_municipalite_in_circonscription = int(np.ceil(nb_of_municipalites / nb_circonscription))
max_distance = np.ceil(nb_of_municipalites / (2 * nb_circonscription))
map = np.zeros((len(x), len(y)))

XX, YY = np.meshgrid(x, y)
municipalities_coordinate = np.column_stack((XX.ravel(), YY.ravel()))
municipalities_priority_matrix = utils_.priority_matrix(len(x), len(y), municipalities_coordinate, max_distance)
municipalities_priority_and_gps = sorted(utils_.custom_convert_matrix_to_list(municipalities_priority_matrix), key= lambda x: x[0])
kmeans = KMeans(n_clusters=nb_circonscription, random_state=0).fit(municipalities_coordinate)
circonscriptions = [[False] for i in range(nb_circonscription)]
map = np.zeros((len(x), len(y)))
number_min_municipalities_to_place = nb_circonscription * min_municipalite_in_circonscription













# break_condition = False
# while not break_condition:
#     nb_of_placed_municipalities = 0
#     for i in range(len(circonscriptions)):
#         for j in range(len(municipalities_priority_and_gps)):
#             center_coordinate = kmeans.cluster_centers_[i]
#             municipality_coordinate = municipalities_priority_and_gps[j]
#             if validate_distance(center_coordinate, municipality_coordinate, max_distance) and not circonscriptions[i][0]:
#                 map[municipality_coordinate[1], municipality_coordinate[2]] = i + 1
#                 municipalities_priority_and_gps.pop(j)
#                 nb_of_placed_municipalities += 1
#                 if len(circonscriptions[i]) == min_municipalite_in_circonscription:
#                     circonscriptions[i][0] = True
#                 break
#     break_condition = nb_of_placed_municipalities < nb_circonscription













# print(clustering.labels_.reshape((6,6)))



# kmeans = KMeans(n_clusters=nb_circonscription, random_state=0).fit(municipalities_coordinate)
# centers = kmeans.cluster_centers_
# map = np.zeros((len(x), len(y)))
# circonscriptions = [[] for i in range(nb_circonscription)]
# clock_wise_iterator = utils_.spiral_matrix_print(map)
# print(clock_wise_iterator.reverse())


# for iterator in clock_wise_iterator:
#     closest_center = (-1, np.inf)
#     for center_index, center in enumerate(centers):
#         distance_ = np.sqrt(np.power(iterator[0] - center[0], 2) + np.power(iterator[1] - center[1], 2))
#
#         if len(circonscriptions[center_index]) == min_municipalite_in_circonscription:
#             continue
#
#         if distance_ < closest_center[1]:
#             closest_center = (center_index, distance_)
#
#     map[iterator[0], iterator[1]] = closest_center[0] + 1
#     circonscriptions[closest_center[0]].append(iterator)
#
# print(map)




# for i in range(len(kmeans.labels_)):
#     set_[kmeans.labels_[i]].append(out[i])
#


# print(circonscriptions)

# for circonscription in circonscriptions:
#     for j in range(len(circonscription) - 1):
#         for k in range(1, len(circonscription)):
#             distance_x = np.abs(circonscription[j][0] - circonscription[k][0])
#             distance_y = np.abs(circonscription[j][1] - circonscription[k][1])
#             distance_total = distance_x + distance_y
#             if distance_total > distance:
#                 print(distance_total - distance)
