import numpy as np
from sklearn.cluster import KMeans

x = np.arange(10)
y = np.arange(15)
nb_municipalite = len(x) * len(y)
nb_circonscription = 4
min_municipalite_in_circonscription = int(np.floor(nb_municipalite / nb_circonscription))
max_municipalite_in_circonscription = int(np.ceil(nb_municipalite / nb_circonscription))
distance = np.ceil(nb_municipalite / (2 * nb_circonscription))

XX, YY = np.meshgrid(x, y)
out = np.column_stack((XX.ravel(), YY.ravel()))
kmeans = KMeans(n_clusters=nb_circonscription, random_state=0).fit(out)

# set_ = [[] for i in range(nb_circonscription)]

centers = kmeans.cluster_centers_
municipalities = np.asarray(out)
circonscriptions = []

for center in centers:
    municipalities = sorted(municipalities, key=lambda point: abs(point[0] - center[0]) + abs(point[1] - center[1]))
    candidates = municipalities[:min_municipalite_in_circonscription]
    municipalities = municipalities[min_municipalite_in_circonscription:]
    circonscriptions.append(candidates)

for municipality in municipalities:
    closest_center = np.inf
    closest_center_index = -1
    for i, center in enumerate(centers):

        if len(circonscriptions[i]) >= max_municipalite_in_circonscription:
            continue

        distance = abs(municipality[0] - center[0]) + abs(municipality[1] - center[1])
        if distance < closest_center:
            closest_center = distance
            closest_center_index = i
    circonscriptions[closest_center_index].append(municipality)

for circonscription in circonscriptions:
    print(len(circonscription))


# for i in range(len(kmeans.labels_)):
#     set_[kmeans.labels_[i]].append(out[i])
#
# for i in set_:
#     print(len(i))
#
# print(min_municipalite_in_circonscription, "   ", max_municipalite_in_circonscription)
#
#
# counter = 0

for circonscription in circonscriptions:
    for j in range(len(circonscription) - 1):
        for k in range(1, len(circonscription)):
            distance_x = np.abs(circonscription[j][0] - circonscription[k][0])
            distance_y = np.abs(circonscription[j][1] - circonscription[k][1])
            distance_total = distance_x + distance_y
            if distance_total > distance:
                print(distance_total - distance)
