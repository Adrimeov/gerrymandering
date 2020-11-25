import numpy as np
from sklearn.cluster import KMeans
import utils_

x = np.arange(6)
y = np.arange(6)
nb_of_municipalites = len(x) * len(y)
nb_circonscription = 7
min_municipalite_in_circonscription = int(np.floor(nb_of_municipalites / nb_circonscription))
max_municipalite_in_circonscription = int(np.ceil(nb_of_municipalites / nb_circonscription))
distance = np.ceil(nb_of_municipalites / (2 * nb_circonscription))

XX, YY = np.meshgrid(x, y)
municipalities_coordinate = np.column_stack((XX.ravel(), YY.ravel()))
kmeans = KMeans(n_clusters=nb_circonscription, random_state=0).fit(municipalities_coordinate)
centers = kmeans.cluster_centers_
map = np.zeros((len(x), len(y)))
circonscriptions = [[] for i in range(nb_circonscription)]
clock_wise_iterator = utils_.spiral_matrix_print(map)
print(clock_wise_iterator.reverse())


for iterator in clock_wise_iterator:
    closest_center = (-1, np.inf)
    for center_index, center in enumerate(centers):
        distance_ = np.sqrt(np.power(iterator[0] - center[0], 2) + np.power(iterator[1] - center[1], 2))

        if len(circonscriptions[center_index]) == min_municipalite_in_circonscription:
            continue

        if distance_ < closest_center[1]:
            closest_center = (center_index, distance_)

    map[iterator[0], iterator[1]] = closest_center[0] + 1
    circonscriptions[closest_center[0]].append(iterator)

print(map)




# for i in range(len(kmeans.labels_)):
#     set_[kmeans.labels_[i]].append(out[i])
#


print(circonscriptions)

for circonscription in circonscriptions:
    for j in range(len(circonscription) - 1):
        for k in range(1, len(circonscription)):
            distance_x = np.abs(circonscription[j][0] - circonscription[k][0])
            distance_y = np.abs(circonscription[j][1] - circonscription[k][1])
            distance_total = distance_x + distance_y
            if distance_total > distance:
                print(distance_total - distance)
