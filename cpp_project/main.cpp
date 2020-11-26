#include <iostream>
#include "vector"
#include "list"
#include "cassert"
#include "math.h"

using namespace std;

struct Municipality {
    int x;
    int y;
    int coadjacency_index;
    int votes;

    Municipality(int _x, int _y, int _votes): x(_x), y(_y), votes(_votes), coadjacency_index(0) {};
};

struct District {
    int distance_max;
    int vote_cost;
    vector<Municipality> municipalities;

    District(): distance_max(0), vote_cost(0) {
        municipalities = vector<Municipality>();
    };
};

struct State {

    float distance_cost;
    float vote_cost;
    list<District> districts;
    // TODO: L'enlever de la??
    vector<Municipality> municipalities;
    int **coadjacency_matrix;
    int nb_rows;
    int nb_cols;
    int nb_municipalities;

    State(const vector<Municipality> &municipalities_, int rows, int cols, int nb_district): nb_rows(rows), nb_cols(cols), distance_cost(0), vote_cost(0) {
        nb_municipalities = municipalities_.size();
        municipalities = municipalities_;
        districts = list<District>(nb_district);
        // TODO: maybe shuffle les municipalites
        for(auto itr : municipalities) {
            District district = districts.front();
            districts.pop_front();
            district.municipalities.push_back(itr);
            districts.push_back(district);
        }
        Setup_Coadjacency();
    };

    void Setup_Coadjacency() {
        coadjacency_matrix = new int*[nb_municipalities];

        for (int i = 0; i < nb_municipalities; i++)
            coadjacency_matrix[i] = new int[nb_municipalities];

        for(int i = 0; i < nb_municipalities - 1; i++) {
            for(int j = i; j < nb_municipalities; j++) {
                int distance_x = abs(municipalities[i].x - municipalities[j].x);
                int distance_y = abs(municipalities[i].y - municipalities[j].y);
                coadjacency_matrix[i][j] = distance_x + distance_y;
                coadjacency_matrix[j][i] = distance_x + distance_y;
            }
        }
    }
};


void ShowState(const State &state) {
    for(auto itr: state.districts) {

    }
}


int main() {

    int nb_row = 3;
    int nb_col = 3;
    int nb_municipalities = nb_col * nb_row;
    int nb_district = 2;
    int min_municipalities_per_district = floor((float)nb_municipalities / (float)nb_district);
    int max_municipalities_per_district = ceil((float)nb_municipalities / (float)nb_district);

    vector<Municipality> municipalities_1;

    for (int i = 0; i < nb_row; i++) {
        for (int j = 0; j < nb_col; j++) {
            municipalities_1.emplace_back(i, j, 100);
        }
    }

    assert(municipalities_1.size() == nb_col * nb_row);
    assert(municipalities_1[0].x == 0);
    assert(municipalities_1[1].y == 1);
    assert(municipalities_1[0].votes == 100);

    State test_state_1 = State(municipalities_1, nb_row, nb_col, nb_district);
    assert(test_state_1.nb_municipalities == nb_col * nb_row);

//    for (int i = 0; i < test_state_1.nb_municipalities; i++) {
//        for (int j = 0; j < test_state_1.nb_municipalities; j++)
//            cout << test_state_1.coadjacency_matrix[i][j] << " ";
//        cout << endl;
//    }

    for(const auto &itr: test_state_1.districts){
        assert(itr.municipalities.size() == min_municipalities_per_district || itr.municipalities.size() == max_municipalities_per_district);
    }


    return 0;
}
