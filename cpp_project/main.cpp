#include <iostream>
#include "vector"
#include "list"
#include "cassert"
#include <cmath>
#include <queue>
#include <chrono>
#include <random>
#include <tuple>

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
};

struct District {
    int distance_max;
    int vote_cost;
    vector<Municipality> municipalities;

    District(const District &district_to_copy){
        distance_max = district_to_copy.distance_max;
        vote_cost = district_to_copy.vote_cost;
        municipalities = vector<Municipality>(district_to_copy.municipalities);
    }
    District(): distance_max(0), vote_cost(0) {
        municipalities = vector<Municipality>();
    };
};

struct State {
    int distance_cost;
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

    State(const vector<Municipality> &municipalities_, int rows, int cols, int nb_district): nb_rows(rows), nb_cols(cols), distance_cost(0), vote_cost(0) {
        nb_municipalities = municipalities_.size();
        municipalities = municipalities_;
        districts = vector<District>(nb_district);
        nb_districts = nb_district;
        // TODO: maybe shuffle les municipalites

        queue<int> fifo;
        for(int i = 0; i < nb_district; i++)
            fifo.push(i);

        for(auto itr : municipalities) {
            int district = fifo.front();
            fifo.pop();
            fifo.push(district);
            districts[district].municipalities.push_back(itr);
        }
        Setup_Coadjacency();
        initialize_state_cost();
    };

    void initialize_state_cost() {
        for(auto & district : this->districts){
            for(int i = 0; i < district.municipalities.size() - 1; i++) {
                for(int j = i + 1; j < district.municipalities.size(); j++) {
                    int distance = this->coadjacency_matrix[district.municipalities[i].x * this->nb_rows + district.municipalities[i].y][district.municipalities[j].x * this->nb_rows + district.municipalities[j].y];
                    if(distance > district.distance_max)
                        district.distance_max = distance;
                }
            }
            this->distance_cost += district.distance_max;
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

int update_new_cost_after_swap(State &state, int district_idx_1, int district_idx_2){
    assert(district_idx_1 != district_idx_2);
    int swap_districts_idxs[2] {district_idx_1, district_idx_2};
    for(int district_idx = 0; district_idx < 2; district_idx++){
        District *district_to_evaluate = &state.districts[swap_districts_idxs[district_idx]];
        district_to_evaluate->distance_max = 0;
        for(int i = 0; i < district_to_evaluate->municipalities.size() - 1; i++) {
            for(int j = i + 1; j < district_to_evaluate->municipalities.size(); j++) {
                int index_x = district_to_evaluate->municipalities[i].x * state.nb_rows + district_to_evaluate->municipalities[i].y;
                int index_y = district_to_evaluate->municipalities[j].x * state.nb_rows + district_to_evaluate->municipalities[j].y;
                int distance = state.coadjacency_matrix[index_x][index_y];
                if(distance > district_to_evaluate->distance_max)
                    district_to_evaluate->distance_max = distance;
            }
        }
    }
    state.distance_cost = 0;
    for(const auto& district: state.districts)
        state.distance_cost += district.distance_max;

    return state.distance_cost;
}

//int apply_new_cost_after_swap_1(State &state, int district_idx_1, int district_idx_2, int swap_mun_1_idx, int swap_mun_2_idx){
//    assert(district_idx_1 != district_idx_2);
//
//    int swap_districts_idxs[2] {district_idx_1, district_idx_2};
//    int swap_municipalities_idxs[2] {swap_mun_1_idx, swap_mun_2_idx};
//    int delta_distance_cost = 0;
//
//    for(int district_idx = 0; district_idx < 2; district_idx++){
//        District *district_to_evaluate = &state.districts[swap_districts_idxs[district_idx]];
//        int swap_municipality_idx = swap_municipalities_idxs[district_idx];
//        Municipality swap_mun = district_to_evaluate->municipalities[swap_municipality_idx];
//        int largest_distance = 0;
//        for(int municipality_idx = 0; municipality_idx < district_to_evaluate->municipalities.size(); municipality_idx++){
//            Municipality other_mun = district_to_evaluate->municipalities[municipality_idx];
//            int cost_between_mun = state.coadjacency_matrix[swap_mun.x * state.nb_rows + swap_mun.y][other_mun.x * state.nb_rows + other_mun.y];
//            if(cost_between_mun > largest_distance)
//                largest_distance = cost_between_mun;
//        }
//        if(largest_distance > district_to_evaluate->distance_max)
//            district_to_evaluate->distance_max = largest_distance;
//    }
//
//    state.distance_cost = 0;
//    for(const auto& district: state.districts)
//        state.distance_cost += district.distance_max;
//
//    return state.distance_cost;
//}

tuple<int, int> find_random_district_swap(const State &state){
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    uniform_int_distribution<int> district_distribution(0,state.nb_districts - 1);
    default_random_engine generator (seed);
    int chosen_district = district_distribution(generator);
    uniform_int_distribution<int> municipalities_distribution(0,state.districts[chosen_district].municipalities.size() - 1);
    int chosen_municipality = municipalities_distribution(generator);
    return make_tuple(chosen_district, chosen_municipality);
}

State Search_new_state(const State &current_state, int district_index, int municipality_index) {
    State best_state(current_state);
    // TODO: tester avec et sans
    best_state.distance_cost = numeric_limits<int>::max();

    for(int i = 0; i < current_state.nb_districts; i++) {
        if(i == district_index) {
            // Skipping swaps with municipalities from same district
            continue;
        }

        for (int j = 0; j < current_state.districts[i].municipalities.size(); j++) {
            State candidate = swap_municipalities(current_state, district_index, i, municipality_index, j);
            int candidate_cost = update_new_cost_after_swap(candidate, district_index, i);

            if (candidate_cost < best_state.distance_cost)
                best_state = candidate;
        }
    }

    return best_state;
}

State Valid_State_Local_Search(const vector<Municipality> &municipalities_, int rows, int cols, int nb_district) {
    State current_state(municipalities_, rows,cols, nb_district);
    State best_state(current_state);
    int max_non_improving_iterations = 1000;
    int non_improving_iterations = 0;

    while (non_improving_iterations < max_non_improving_iterations){
        non_improving_iterations++;
        tuple<int, int> random_indexes = find_random_district_swap(current_state);
        current_state = Search_new_state(current_state, get<0>(random_indexes), get<1>(random_indexes));

        if (current_state.distance_cost < best_state.distance_cost) {
            best_state = current_state;
            non_improving_iterations = 0;
        }
    }

    return best_state;
}

bool validate_state(const State &state) {
    int distance_max = ceil((float)state.nb_municipalities / (2*(float)state.nb_districts));
    cout << distance_max << endl;
    for(auto & district : state.districts){
        for(int i = 0; i < district.municipalities.size() - 1; i++) {
            for(int j = i + 1; j < district.municipalities.size(); j++) {
                int distance = state.coadjacency_matrix[district.municipalities[i].x * state.nb_rows + district.municipalities[i].y][district.municipalities[j].x * state.nb_rows + district.municipalities[j].y];
                if(distance > distance_max) {
                    cout << distance;
                    cout << ": Invalid !" << endl;
                    return false;
                }
            }
        }
    }
    cout << "Valid!" << endl;
    return true;
}

int main() {

    int nb_row = 10;
    int nb_col = 10;
    int nb_municipalities = nb_col * nb_row;
    int nb_district = 7;
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

    State new_state = swap_municipalities(test_state_1, 0, 1, 0, 0);
    tuple<int, int> indexes = find_random_district_swap(new_state);
    assert(get<0>(indexes) < new_state.nb_districts && get<0>(indexes) >= 0);
    assert(get<1>(indexes) < new_state.districts[get<0>(indexes)].municipalities.size() && get<1>(indexes) >= 0);
//    ShowState(new_state);
    cout << update_new_cost_after_swap(new_state, 0, 1) << endl;
    cout << new_state.distance_cost <<endl;
//    ShowState(new_state);


    // Search test
    State best_state = Valid_State_Local_Search(municipalities_1, nb_row, nb_col, nb_district);

    cout << best_state.distance_cost << endl;
    ShowState(best_state);
    validate_state(best_state);

    return 0;
}
