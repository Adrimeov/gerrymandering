#include <iostream>
#include "vector"
#include "list"
#include "cassert"
#include <cmath>
#include <queue>
#include <chrono>
#include <random>
#include <tuple>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace std;

struct Municipality {
    int x;
    int y;
    int coadjacency_index;
    int votes;
    Municipality(const Municipality &municipality_to_copy){
        x = municipality_to_copy.x;
        y = municipality_to_copy.y;
        coadjacency_index =  municipality_to_copy.coadjacency_index;
        votes = municipality_to_copy.votes;
    }
    Municipality(int _x, int _y, int _votes): x(_x), y(_y), votes(_votes), coadjacency_index(0) {};

    int get_x() { return x; }
    int get_y() { return y; }
    int get_votes() { return votes; }

    void set_x(int value) { x = value; }
    void set_y(int value) { y = value; }
    void set_votes(int value) { votes = value; }
};
struct District {
    float distance_cost;
    int vote_cost;
    float _center_x;
    float _center_y;
    float outlier_distance = 0;
    vector<Municipality> municipalities;

    District(const District &district_to_copy){
        distance_cost = district_to_copy.distance_cost;
        vote_cost = district_to_copy.vote_cost;
        municipalities = vector<Municipality>(district_to_copy.municipalities);
        _center_x = district_to_copy._center_x;
        _center_y = district_to_copy._center_y;

    }
    District(): distance_cost(0), vote_cost(0) {
        municipalities = vector<Municipality>();
    };
};

struct State {
    float distance_cost;
    int vote_cost;
    vector<District> districts;
    // TODO: L'enlever de la??
    vector<Municipality> municipalities;
    int **coadjacency_matrix;
    int nb_rows;
    int nb_cols;
    int nb_municipalities;
    int nb_districts;

    State(const State &state_to_copy) {
        distance_cost = state_to_copy.distance_cost;
        vote_cost = state_to_copy.vote_cost;
        districts = vector<District>(state_to_copy.districts);
        municipalities = vector<Municipality>(state_to_copy.municipalities);
        coadjacency_matrix = state_to_copy.coadjacency_matrix;
        nb_municipalities = state_to_copy.nb_municipalities;
        nb_rows = state_to_copy.nb_rows;
        nb_cols = state_to_copy.nb_cols;
        nb_districts = state_to_copy.nb_districts;
    }

    State(const vector<Municipality> &municipalities_, int rows, int cols, int nb_district, vector<vector<float>> centers): nb_rows(rows), nb_cols(cols), distance_cost(0), vote_cost(0) {
        nb_municipalities = municipalities_.size();
        municipalities = municipalities_;
        districts = vector<District>(nb_district);
        nb_districts = nb_district;
        // TODO: maybe shuffle les municipalites

        initialize_state(centers);
        Setup_Coadjacency();
        recalculate_centers();
        initialize_state_cost();
        initialize_outlier_distance();
    };

    void recalculate_centers(){

        for(auto & district : this-> districts){
            int mun_index = -1;
            int distance_tot =  numeric_limits<int>::max();
            for(int i = 0; i < district.municipalities.size() -1; i++){
                int distance_tempo = 0;
                for(int j = 0; j < district.municipalities.size(); j++){
                    int distance_x = abs(district.municipalities[i].x - district.municipalities[j].x);
                    int distance_y = abs(district.municipalities[i].y - district.municipalities[j].y);
                    distance_tempo+= distance_x + distance_y;
                }
                if(distance_tot > distance_tempo){
                    distance_tot = distance_tempo;
                    mun_index = i;
                }
            }
            district._center_x = district.municipalities[mun_index].x;
            district._center_y = district.municipalities[mun_index].y;
        }
    }
    void initialize_outlier_distance(){
        for(auto & district : this->districts){
            float district_worst_distance = 0;
            for( auto & municipality: district.municipalities){
                float dist_x = abs(municipality.x - district._center_x);
                float dist_y = abs(municipality.y - district._center_y);
                float distance = dist_y + dist_x;
                if(distance > district_worst_distance)
                    district_worst_distance = distance;
            }
            district.outlier_distance = district_worst_distance;
        }
    }


    void initialize_state(vector<vector<float>> centers){
        vector<int> available_distric;
        vector<int> district_count = initialize_district_count();
        for(int i = 0; i < this->nb_districts; i ++){
            available_distric.push_back(i);
        }

        for(int mun_index = 0; mun_index < this->nb_municipalities; mun_index++){
            float dist_max = numeric_limits<int>::max();
            int district_index = -1;
            for(int district_idx = 0; district_idx < this->nb_districts; district_idx++){
                int dist_x = abs(this->municipalities[mun_index].x - centers[available_distric[district_idx]][0]);
                int dist_y = abs(this->municipalities[mun_index].y - centers[available_distric[district_idx]][1]);
                if(dist_y + dist_x < dist_max){
                    dist_max = dist_x + dist_y;
                    district_index = district_idx;
                }

            }
            this->districts[available_distric[district_index]].municipalities.push_back(this->municipalities[mun_index]);
            district_count[available_distric[district_index]] --;
            if(district_count[available_distric[district_index]] == 0)
                available_distric.erase(available_distric.begin() + district_index);
        }


    }
    vector<int> initialize_district_count(){
        float nb_municipality_min = floor(this->nb_municipalities / this->nb_districts);
        float nb_municipality_max = ceil(this->nb_municipalities / this->nb_districts);
        int reminder = this->nb_municipalities - (this->nb_districts * nb_municipality_min);
        vector<int> distribution(this->nb_districts, nb_municipality_min);
        for(int i = 0; i < this->nb_districts; i++){
            if(reminder <= 0)
                break;
            if(distribution[i] < nb_municipality_max) {
                distribution[i] += 1;
                reminder -= 1;
            }
        }
        return distribution;
    }

    void initialize_state_1(vector<vector<float>> centers){
        vector<int> available_distric;
        int min_nb_mun_per_district = floor((float)this->nb_municipalities / this->nb_districts);
        cout<< "Min number on mun per district: "<<min_nb_mun_per_district<<endl;
        for(int i = 0; i < this->nb_districts; i ++){
            available_distric.push_back(i);
        }

        for(int i = 0; i < this->municipalities.size(); i++){
            if(available_distric.empty()) {
                this->districts[i % this->nb_districts].municipalities.push_back(this->municipalities[i]);
                continue;
            }

            int closer = numeric_limits<int>::max();
            int index = 0;
            for(int j = 0; j < available_distric.size(); j++){
                int dist_x = abs(this->municipalities[i].x - centers[available_distric[j]][0]);
                int dist_y = abs(this->municipalities[i].y - centers[available_distric[j]][1]);
                int dist_tot = dist_x + dist_y;
                if(dist_tot < closer){
                    closer = dist_tot;
                    index = j;
                }
            }
            this->districts[available_distric[index]].municipalities.push_back(this->municipalities[i]);
            if(this->districts[available_distric[index]].municipalities.size() == min_nb_mun_per_district)
                available_distric.erase(available_distric.begin() + index);

        }
    }

    void initialize_state_cost() {
//        for(int i = 0; i < this->nb_districts; i++){
//            this->districts[i]._center_x = centers[i][0];
//            this->districts[i]._center_y = centers[i][1];
//        }

        for(auto & district : this->districts){
            district.distance_cost = 0;
            float municipality_size = (float)district.municipalities.size();

            float distance_tot = 0;
            for(int i = 0; i < municipality_size; i++){
                float distance_x = abs(district.municipalities[i].x - district._center_x);
                float distance_y = abs(district.municipalities[i].y - district._center_y);
                distance_tot += pow(distance_x + distance_y, 2) / municipality_size;
            }
            district.distance_cost = distance_tot / municipality_size;
            this->distance_cost += district.distance_cost;
        }
    }

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
    int matrice[state.nb_rows][state.nb_cols];
    int index = 1;
    for(auto itr = state.districts.begin(); itr != state.districts.end(); itr++, index++) {
        for(auto itr_2 = itr->municipalities.begin(); itr_2 != itr->municipalities.end(); itr_2++)
            matrice[itr_2->x][itr_2->y] = index;
    }
    for(int i = 0; i < state.nb_rows; i++) {
        for (int j = 0; j < state.nb_cols; j++)
            cout << matrice[i][j] << " ";
        cout << endl;
    }

}

State swap_municipalities(State current_state, int dist_idx_1, int dist_idx_2, int mun_idx_1, int mun_idx_2){
    State new_state = State(current_state);
    // TODO essayer de swap les ref, pas les objets.
    Municipality municipality_tempo = new_state.districts[dist_idx_1].municipalities[mun_idx_1];
    new_state.districts[dist_idx_1].municipalities[mun_idx_1] = new_state.districts[dist_idx_2].municipalities[mun_idx_2];
    new_state.districts[dist_idx_2].municipalities[mun_idx_2] = municipality_tempo;
    return new_state;
}
void update_outlier_distance(State &state, int district_idx){
    float district_new_outlier_distance = 0;
    for(auto & municipality: state.districts[district_idx].municipalities){
        float distance_x = abs(municipality.x - state.districts[district_idx]._center_x);
        float distance_y = abs(municipality.y - state.districts[district_idx]._center_y);
        if(distance_x + distance_y > district_new_outlier_distance)
            district_new_outlier_distance = distance_x + distance_y;
    }
    state.districts[district_idx].outlier_distance = district_new_outlier_distance;
}

float update_new_cost_after_swap(State &state, int district_idx_1, int district_idx_2, int mun_idx_1, int mun_idx_2){
    assert(district_idx_1 != district_idx_2);
    int indexes[2]{district_idx_1, district_idx_2};

    for(int i = 0; i < 2; i++) {
        state.distance_cost -= state.districts[indexes[i]].distance_cost;
        float distance_tot = 0;
        float municipality_size = (float)state.districts[indexes[i]].municipalities.size();

        for(int j = 0; j < municipality_size; j ++){
            float distance_x = abs(state.districts[indexes[i]].municipalities[j].x - state.districts[indexes[i]]._center_x);
            float distance_y = abs(state.districts[indexes[i]].municipalities[j].y - state.districts[indexes[i]]._center_y);
            distance_tot += pow(distance_x + distance_y, 2) / municipality_size;
        }
        state.districts[indexes[i]].distance_cost = distance_tot / municipality_size;
        state.distance_cost += state.districts[indexes[i]].distance_cost;
    }
    update_outlier_distance(state, district_idx_1);
    update_outlier_distance(state, district_idx_2);
    return state.distance_cost;
}

int update_new_cost_after_swap_1(State &state, int district_idx_1, int district_idx_2, int mun_idx_1, int mun_idx_2){
    assert(district_idx_1 != district_idx_2);

    District *district_1 = &state.districts[district_idx_1];
    District *district_2 = &state.districts[district_idx_2];

    state.distance_cost -= district_1->distance_cost;
    state.distance_cost -= district_2->distance_cost;

    district_1->distance_cost -= abs(district_1->_center_x - district_2->municipalities[mun_idx_2].x);
    district_1->distance_cost -= abs(district_1->_center_y - district_2->municipalities[mun_idx_2].y);
    district_2->distance_cost -= abs(district_2->_center_x - district_1->municipalities[mun_idx_1].x);
    district_2->distance_cost -= abs(district_2->_center_y - district_1->municipalities[mun_idx_1].y);

    district_1->distance_cost += abs(district_1->_center_x - district_1->municipalities[mun_idx_1].x);
    district_1->distance_cost += abs(district_1->_center_y - district_1->municipalities[mun_idx_1].y);
    district_2->distance_cost += abs(district_2->_center_x - district_2->municipalities[mun_idx_2].x);
    district_2->distance_cost += abs(district_2->_center_y - district_2->municipalities[mun_idx_2].y);

    state.distance_cost += district_1->distance_cost;
    state.distance_cost += district_2->distance_cost;

    int indexes[2]{district_idx_1, district_idx_2};

    for(int i = 0; i < 2; i++) {
        float new_center_x = 0;
        float new_center_y = 0;
        float nb_mun = state.districts[indexes[i]].municipalities.size();

        for (int j = 0; j < nb_mun; j++) {
            new_center_x += state.districts[indexes[i]].municipalities[j].x;
            new_center_y += state.districts[indexes[i]].municipalities[j].y;
        }

        state.districts[indexes[i]]._center_x = new_center_x / nb_mun;
        state.districts[indexes[i]]._center_y = new_center_y / nb_mun;
    }

    return state.distance_cost;
}

tuple<int, int> find_district_swap(const State &state, int iteration_count) {

    float wildcard_probability = 0.5;

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    uniform_real_distribution<float> proba(0.0, 1.0);
    default_random_engine generator(seed);

    if (wildcard_probability > proba(generator))
    {
        uniform_int_distribution<int> district_distribution(0,state.nb_districts - 1);
        int chosen_district = district_distribution(generator);
        uniform_int_distribution<int> municipalities_distribution(0,state.districts[chosen_district].municipalities.size() - 1);
        int chosen_municipality = municipalities_distribution(generator);
        return make_tuple(chosen_district, chosen_municipality);
    }

    float worst_cost = 0;
    float worst_district_index = 0;
    for (int i = 0; i < state.districts.size(); i++) {
        if (state.districts[i].outlier_distance > worst_cost) {
            worst_cost = state.districts[i].outlier_distance;
            worst_district_index = i;
        }
    }
    worst_cost = 0;
    float worst_mun_index = 0;
    for(int i = 0; i < state.districts[worst_district_index].municipalities.size(); i++){
        float distance_x = abs(state.districts[worst_district_index].municipalities[i].x - state.districts[worst_district_index]._center_x);
        float distance_y = abs(state.districts[worst_district_index].municipalities[i].y - state.districts[worst_district_index]._center_y);
        if(worst_cost < (distance_y + distance_x)) {
            worst_cost = distance_x + distance_y;
            worst_mun_index = i;
        }

    }
    Municipality mun = state.districts[worst_district_index].municipalities[worst_mun_index];
    return make_tuple(worst_district_index, worst_mun_index);
}

State Search_new_state(const State &current_state, int district_index, int municipality_index) {
    State best_state(current_state);
    // TODO: tester avec et sans
    best_state.distance_cost = numeric_limits<int>::max();
//    best_state.distance_cost = current_state.distance_cost;
    for(int i = 0; i < current_state.nb_districts; i++) {
        if(i == district_index) {
            // Skipping swaps with municipalities from same district
            continue;
        }

        for (int j = 0; j < current_state.districts[i].municipalities.size(); j++) {
            State candidate = swap_municipalities(current_state, district_index, i, municipality_index, j);
            float candidate_cost = update_new_cost_after_swap(candidate, district_index, i, municipality_index, j);
            if (candidate_cost < best_state.distance_cost)
                best_state = candidate;
        }
    }
    return best_state;
}
void print_solution(const State &state) {
    for(int i = 0; i < state.nb_districts; i ++ ){
        for(int j = 0; j < state.districts[i].municipalities.size(); j ++){
            cout << state.districts[i].municipalities[j].y <<" " << state.districts[i].municipalities[j].x<<" ";
        }
        cout<<endl;
    }
    cout<<endl<<endl;;

}

bool validate_state(const State &state) {
    float distance_max = ceil((float)state.nb_municipalities / (2*(float)state.nb_districts));
    bool valid = true;
    int district_num = 0;
    for(auto & district : state.districts){

        for(int i = 0; i < district.municipalities.size() - 1; i++) {
            for(int j = i + 1; j < district.municipalities.size(); j++) {
                  int distance_x = abs(district.municipalities[i].x - district.municipalities[j].x);
                  int distance_y = abs(district.municipalities[i].y - district.municipalities[j].y);
                  int distance = distance_x + distance_y;
//                int x = district.municipalities[i].x * state.nb_cols + district.municipalities[i].y;
//                int y = district.municipalities[j].x * state.nb_cols + district.municipalities[j].y;
//                int distance = state.coadjacency_matrix[x][y];

                if(distance > distance_max) {
                    cout << " Distance: " << distance;
                    cout <<" Outlier district: "<< district_num + 1;
                    valid = false;
                }

            }
        }
        district_num+=1;
    }
    if(valid)
        cout << "Valid!" << endl;
    cout<<""<<endl;
    return valid;
}

float validation_threshold(const State &state){
    float distance_max = ceil(state.nb_municipalities / (2*state.nb_districts));
    float min_nb_mun_per_district = floor(state.nb_municipalities / state.nb_districts);
    float division = pow(distance_max, 2) / min_nb_mun_per_district;
    return division * state.nb_districts;
}

bool Valid_State_Local_Search(const vector<Municipality> &municipalities_, int rows, int cols, int nb_district, int max_non_improving_iterations, vector<vector<float>> centers,  bool print_) {
    State current_state(municipalities_, rows,cols, nb_district, centers);
    State best_state(current_state);
    ShowState(current_state);
    float treshold = 60.0;
    int non_improving_iterations = 0;
    int iteration_counter = 0;

    while (non_improving_iterations < max_non_improving_iterations){
        non_improving_iterations++;
        tuple<int, int> random_indexes = find_district_swap(current_state, ++iteration_counter);
        current_state = Search_new_state(current_state, get<0>(random_indexes), get<1>(random_indexes));

        if (current_state.distance_cost < best_state.distance_cost) {
            best_state = current_state;
            non_improving_iterations = 0;
            cout << best_state.distance_cost << endl;
            if(best_state.distance_cost <= treshold) {
                cout << "validation in progress.." << endl;
                if (validate_state(best_state)) {
                    ShowState(best_state);
                    return true;
                }
            }
        }
    }

    ShowState(best_state);
    return validate_state(best_state);
}
void test_initialize(const vector<Municipality> &municipalities_, int rows, int cols, int nb_district, vector<vector<float>> centers){
    State state_test(municipalities_, rows, cols, nb_district, centers);
    ShowState(state_test);
}



PYBIND11_MODULE(CppLib, m) {

    py::class_<Municipality>(m, "Municipality")
            .def(py::init<int, int, int>())
            .def("get_x", &Municipality::get_x)
            .def("get_y", &Municipality::get_y)
            .def("get_votes", &Municipality::get_votes)
            .def("set_x", &Municipality::set_x)
            .def("set_y", &Municipality::set_y)
            .def("set_votes", &Municipality::set_votes);

    m.def("Valid_State_Local_Search", &Valid_State_Local_Search, "Builds a valid state using local search.");
    m.def("test_initialize", &test_initialize, "Tests centers");
}
