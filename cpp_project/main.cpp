#include <iostream>
#include "vector"
#include "list"
#include "cassert"
#include <cmath>
#include <chrono>
#include <tuple>
#include <random>

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
    float distance_cost;
    int vote_cost;
    float _center_x;
    float _center_y;
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
        vote_cost = 0;
        distance_cost = 0;

        initialize_state(centers);
        Setup_Coadjacency();
        initialize_state_cost(centers);
        initialize_vote_cost();
    };

    void initialize_state(vector<vector<float>> centers){
        vector<int> available_distric;
        int min_nb_mun_per_district = floor((float)this->nb_municipalities / this->nb_districts);
        cout<<min_nb_mun_per_district<<endl;
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

    void initialize_state_cost(vector<vector<float>> centers) {
        for(int i = 0; i < this->nb_districts; i++){
            this->districts[i]._center_x = centers[i][0];
            this->districts[i]._center_y = centers[i][1];
        }

        for(auto & district : this->districts){
            district.distance_cost = 0;
            float municipality_size = (float)district.municipalities.size();

            float distance_tot = 0;
            for(int i = 0; i < municipality_size; i++){
                float distance_x = abs(district.municipalities[i].x - district._center_x);
                float distance_y = abs(district.municipalities[i].y - district._center_y);
                distance_tot += distance_x + distance_y;
            }
            district.distance_cost = distance_tot / municipality_size;
            this->distance_cost += district.distance_cost;
        }
    }

    void initialize_vote_cost() {
        vote_cost = 0;
        int vote_per_mun = 100;

        for(auto &district: districts) {
            int votes_per_district = vote_per_mun * district.municipalities.size();
            int green_votes = 0;

            for(int i = 0; i < district.municipalities.size(); i++)
                green_votes += district.municipalities[i].votes;

            int votes_to_win = votes_per_district / 2 + 1;

            if (green_votes > votes_to_win) {
                district.vote_cost = green_votes - votes_to_win;
            } else {
                district.vote_cost = green_votes;
            }

            vote_cost += district.vote_cost;
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
            distance_tot += distance_x + distance_y;
        }
        state.districts[indexes[i]].distance_cost = distance_tot / municipality_size;
        state.distance_cost += state.districts[indexes[i]].distance_cost;
    }
    return state.distance_cost;
}

int update_vote_cost_after_swap(State &state, int district_idx_1, int district_idx_2) {
    assert(district_idx_1 != district_idx_2);
    int indexes[2]{district_idx_1, district_idx_2};
    int vote_per_mun = 100;

    for (int index : indexes) {
        state.vote_cost -= state.districts[index].vote_cost;

        int votes_per_district = vote_per_mun * state.districts[index].municipalities.size();
        int green_votes = 0;

        for(const auto &mun: state.districts[index].municipalities)
            green_votes += mun.votes;

        int votes_to_win = votes_per_district / 2 + 1;

        if (green_votes >= votes_to_win) {
            state.districts[index].vote_cost = green_votes - votes_to_win;
        } else {
            state.districts[index].vote_cost = green_votes;
        }

        int calisse = state.districts[index].vote_cost;


        state.vote_cost += state.districts[index].vote_cost;
    }

    return state.vote_cost;
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

void print_solution(const State &state) {

    for(int i = 0; i < state.nb_districts; i ++ ){
        for(int j = 0; j < state.districts[i].municipalities.size(); j ++){
            cout << state.districts[i].municipalities[j].x <<" " << state.districts[i].municipalities[j].y<< endl;
        }
    }

}

tuple<int, int> find_district_swap(const State &state, int iteration_count) {

    // TODO: trouver une belle formule
//    float wildcard_probability = 0.5;
//
//    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
//    uniform_real_distribution<float> proba(0.0, 1.0);
//    default_random_engine generator(seed);
//
//    if (wildcard_probability > proba(generator))
//    {
//        uniform_int_distribution<int> district_distribution(0,state.nb_districts - 1);
//        int chosen_district = district_distribution(generator);
//        uniform_int_distribution<int> municipalities_distribution(0,state.districts[chosen_district].municipalities.size() - 1);
//        int chosen_municipality = municipalities_distribution(generator);
//        return make_tuple(chosen_district, chosen_municipality);
//    }

    float worst_cost = 0;
    float worst_district_index = 0;
    for (int i = 0; i < state.districts.size(); i++) {
        if (state.districts[i].distance_cost > worst_cost) {
            worst_cost = state.districts[i].distance_cost;
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

tuple<int, int> find_vote_swap(const State &state) {
    float worst_cost = 0;

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    uniform_int_distribution<int> random_district_dist(0.0, state.nb_districts - 1);
    default_random_engine generator(seed);
    int district_index = random_district_dist(generator);

    worst_cost = 0;
    int worst_mun_index = 0;

    for(int i = 0; i < state.districts[district_index].municipalities.size(); i++){
        float distance_x = abs(state.districts[district_index].municipalities[i].x - state.districts[district_index]._center_x);
        float distance_y = abs(state.districts[district_index].municipalities[i].y - state.districts[district_index]._center_y);

        if(worst_cost < (distance_y + distance_x)) {
            worst_cost = distance_x + distance_y;
            worst_mun_index = i;
        }
    }

    return make_tuple(district_index, worst_mun_index);
}

State Search_new_state(const State &current_state, int district_index, int municipality_index) {
    State best_state(current_state);
    // TODO: tester avec et sans
    best_state.distance_cost = numeric_limits<int>::max();
//    best_state.distance_cost = best_state.distance_cost;
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

bool validate_state(const State &state) {
    float distance_max = ceil((float)state.nb_municipalities / (2*(float)state.nb_districts));
    for(auto & district : state.districts){
        for(int i = 0; i < district.municipalities.size() - 1; i++) {
            for(int j = i + 1; j < district.municipalities.size(); j++) {
                int x = district.municipalities[i].x * state.nb_cols + district.municipalities[i].y;
                int y = district.municipalities[j].x * state.nb_cols + district.municipalities[j].y;
                int distance = state.coadjacency_matrix[x][y];
                if(distance > distance_max) {
                    return false;
                }

            }
        }
    }
    return true;
}

State Search_new_vote_state(const State &current_state, int district_index, int municipality_index) {
    State best_state(current_state);
    int best_score = numeric_limits<int>::max();

    for(int i = 0; i < current_state.nb_districts; i++) {
        if(i == district_index) {
            // Skipping swaps with municipalities from same district
            continue;
        }

        for (int j = 0; j < current_state.districts[i].municipalities.size(); j++) {
            State candidate = swap_municipalities(current_state, district_index, i, municipality_index, j);
            int candidate_cost = update_vote_cost_after_swap(candidate, district_index, i);

            if (validate_state(candidate) and candidate_cost < best_score) {
                best_score = candidate_cost;
                best_state = candidate;
            }
        }
    }

    return best_state;
}

void Initialize_Random_Votes(vector<Municipality> &municipalities) {
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    uniform_int_distribution<int> random_votes_dist(0.0, 100);
    default_random_engine generator(seed);

    for (auto &municipality: municipalities)
        municipality.votes = random_votes_dist(generator);
}

float validation_threshold(const State &state){
    return state.nb_municipalities;
}

bool Valid_State_Local_Search(const vector<Municipality> &municipalities_, int rows, int cols, int nb_district, int max_non_improving_iterations, vector<vector<float>> centers,  bool print_) {
    State current_state(municipalities_, rows,cols, nb_district, centers);
    State best_state(current_state);
    float treshold = validation_threshold(best_state);
    int non_improving_iterations = 0;
    int iteration_counter = 0;

    cout<<"_-----------Initial-------------_"<<endl;
    ShowState(current_state);

    while (non_improving_iterations < max_non_improving_iterations){
        non_improving_iterations++;
        tuple<int, int> random_indexes = find_district_swap(current_state, ++iteration_counter);
        current_state = Search_new_state(current_state, get<0>(random_indexes), get<1>(random_indexes));
        if (current_state.distance_cost < best_state.distance_cost) {
            best_state = current_state;
            non_improving_iterations = 0;
            cout << "Treshold: "<< treshold << endl;
            cout << best_state.distance_cost << endl;
            ShowState(best_state);
            if(best_state.distance_cost <= treshold)
                if(validate_state(best_state)) {
                    return true;
                }

        }
        if (non_improving_iterations== max_non_improving_iterations - 1) {
            cout << "last" << endl;
            ShowState(current_state);
        }
    }

    return validate_state(best_state);
}

State Valid_State_Local_Search(const State &initial_state, int max_non_improving_iterations,  bool print_) {
    State current_state(initial_state);
    State best_state(current_state);
    float threshold = validation_threshold(best_state);
    int non_improving_iterations = 0;
    int iteration_counter = 0;

    cout<<"_-----------Initial-------------_"<<endl;
    ShowState(current_state);

    while (non_improving_iterations < max_non_improving_iterations){
        non_improving_iterations++;
        tuple<int, int> random_indexes = find_district_swap(current_state, ++iteration_counter);
        current_state = Search_new_state(current_state, get<0>(random_indexes), get<1>(random_indexes));
        if (current_state.distance_cost < best_state.distance_cost) {
            best_state = current_state;
            non_improving_iterations = 0;
            if(best_state.distance_cost <= threshold)
                if(validate_state(best_state))
                    return best_state;
        }
    }

    return best_state;
}

State Votes_Local_Search(const State &initial_state, int max_non_improving_iterations) {
    State current_state(initial_state);
    State best_state(current_state);
    int non_improving_iterations = 0;

    while (non_improving_iterations < max_non_improving_iterations) {
        non_improving_iterations++;
        tuple<int, int> random_indexes = find_vote_swap(current_state);
        current_state = Search_new_vote_state(current_state, get<0>(random_indexes), get<1>(random_indexes));
        if (current_state.vote_cost < best_state.vote_cost) {
            best_state = current_state;
            non_improving_iterations = 0;
        }
        if (current_state.vote_cost < 0) {
            cout << "hmmm" << endl;
        }
    }

    return best_state;
}

void Local_Search(const vector<Municipality> &municipalities_, int rows, int cols, int nb_district,
                  int max_non_improving_iterations, const vector<vector<float>> &centers,  bool print_) {

    State initial_state(municipalities_, rows,cols, nb_district, centers);

    cout << "-----Initial State-----" << endl;
    ShowState(initial_state);

    initial_state = Valid_State_Local_Search(initial_state, max_non_improving_iterations, print_);

    if (!validate_state(initial_state)) {
        cout << "invalid state" << endl;
        return;
    }

    cout << "-----Valid Districts State-----" << endl;
    ShowState(initial_state);
    initial_state.initialize_vote_cost();
    cout << "Vote cost: " << initial_state.vote_cost << endl;

    State final_state = Votes_Local_Search(initial_state, max_non_improving_iterations);

    cout << "-----Vote Optimised States-----" << endl;
    ShowState(final_state);
    cout << "Vote cost: " << final_state.vote_cost << endl;
}

int main() {
    vector<vector<float>> centers_10_10 { vector<float>{8, 2}, vector<float>{4 , 7}, vector<float>{2, 2},vector<float>{8, 7}
            ,vector<float>{5, 2}, vector<float>{1, 7}};
    int nb_row = 10;
    int nb_col = 10;
    int nb_municipalities = nb_col * nb_row;
    int nb_district = 6;
    int min_municipalities_per_district = floor((float)nb_municipalities / (float)nb_district);
    int max_municipalities_per_district = ceil((float)nb_municipalities / (float)nb_district);

    vector<Municipality> municipalities_1;

    for (int i = 0; i < nb_row; i++) {
        for (int j = 0; j < nb_col; j++) {
            municipalities_1.emplace_back(i, j, 100);
        }
    }

//    assert(municipalities_1.size() == nb_col * nb_row);
//    assert(municipalities_1[0].x == 0);
//    assert(municipalities_1[1].y == 1);
//    assert(municipalities_1[0].votes == 100);

    State test_state_1 = State(municipalities_1, nb_row, nb_col, nb_district, centers_10_10);
//    ShowState(test_state_1);
    vector<vector<float>> threshold_centers { vector<float>{8, 2}, vector<float>{4 , 7}, vector<float>{2, 2},vector<float>{8, 7}};
    State threshold_state_test = State(municipalities_1, 8, 8, 4, threshold_centers);
    float threshold = validation_threshold(test_state_1);
//    assert(test_state_1.nb_municipalities == nb_col * nb_row);

//    for (int i = 0; i < test_state_1.nb_municipalities; i++) {
//        for (int j = 0; j < test_state_1.nb_municipalities; j++)
//            cout << test_state_1.coadjacency_matrix[i][j] << " ";
//        cout << endl;
//    }

//    for(const auto &itr: test_state_1.districts){
//        assert(itr.municipalities.size() == min_municipalities_per_district || itr.municipalities.size() == max_municipalities_per_district);
//    }
//    ShowState(test_state_1);
//    cout << test_state_1.distance_cost << endl;
//    tuple<int, int> to_swap = find_district_swap(test_state_1, 0);
////    cout<< get<0>(to_swap) << ":"<<get<1>(to_swap) << endl;
//    State new_state = swap_municipalities(test_state_1, 0, 1, 0, 0);
//    tuple<int, int> indexes = find_district_swap(new_state, 0);
//    assert(get<0>(indexes) < new_state.nb_districts && get<0>(indexes) >= 0);
//    assert(get<1>(indexes) < new_state.districts[get<0>(indexes)].municipalities.size() && get<1>(indexes) >= 0);
//    ShowState(new_state);

//    State best_state = Valid_State_Local_Search(test_state_1.municipalities, nb_row, nb_col, nb_district, 100, centers, false);
//
//    ShowState(best_state);
//    cout << best_state.distance_cost << endl;

    // test 10 10 avec 6 districts

    Initialize_Random_Votes(municipalities_1);
    Local_Search(municipalities_1, nb_row, nb_col, nb_district,  100, centers_10_10, false);

    //

//    bool found = false;
//    while(!found){
//        cout << "oooooooooooooooooooooooooooooooooooo" << endl;
//        found = Valid_State_Local_Search(test_state.municipalities, nb_row, nb_col, nb_district, 100, centers_10_10, false);
//    }

    return 0;
}
