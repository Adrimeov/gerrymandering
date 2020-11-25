import numpy as np
from sklearn.cluster import KMeans

x = np.arange(10)
y = np.arange(15)
nb_municipalite = len(x) * len(y)
nb_circonscription = 4
min_municipalite_in_circonscription = np.floor(nb_municipalite / nb_circonscription)
max_municipalite_in_circonscription = np.ceil(nb_municipalite / nb_circonscription)
distance = np.ceil(nb_municipalite / (2 * nb_circonscription))

XX, YY = np.meshgrid(x, y)
out = np.column_stack((XX.ravel(), YY.ravel()))
kmeans = KMeans(n_clusters=nb_circonscription, random_state=0).fit(out)



set_ = [[] for i in range(nb_circonscription)]

for i in range(len(kmeans.labels_)):
    set_[kmeans.labels_[i]].append(out[i])

for i in set_:
    print(len(i))

print(min_municipalite_in_circonscription, "   ", max_municipalite_in_circonscription)


counter = 0
for i in range(len(set_)):
    for j in range(len(set_[i]) - 1):
        for k in range(1, len(set_[i])):
            distance_x = np.abs(set_[i][j][0] - set_[i][k][0])
            distance_y = np.abs(set_[i][j][1] - set_[i][k][1])
            distance_total = distance_x + distance_y
            if distance_total > distance:
                print(distance_total - distance)
