#include <iostream>
#include "vector"
#include "list"
#include "cassert"
#include <cmath>
#include <queue>
#include <chrono>
#include <random>
#include <tuple>
#include "signal.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace std;

bool ACTIVE = true;

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

    State(const vector<vector<Municipality>> &districts, int rows, int cols, int nb_district, vector<vector<float>> centers): nb_rows(rows), nb_cols(cols), distance_cost(0), vote_cost(0) {
        municipalities = vector<Municipality>();
        this->districts = vector<District>(nb_district);
        nb_districts = nb_district;
        vote_cost = 0;
        distance_cost = 0;

        for (int i = 0; i < districts.size(); i++) {
            this->districts[i].municipalities = districts[i];
            this->districts[i]._center_x = centers[i][0];
            this->districts[i]._center_y = centers[i][1];
            municipalities.insert(municipalities.begin(), districts[i].begin(), districts[i].end());
        }

        nb_municipalities = municipalities.size();
        Setup_Coadjacency();
        initialize_state_cost(centers);
        initialize_vote_cost();
    }

    void initialize_state(vector<vector<float>> centers){
        vector<int> available_distric;
        int min_nb_mun_per_district = floor((float)this->nb_municipalities / this->nb_districts);
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

            district.vote_cost = abs(green_votes - votes_to_win);

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
        state.districts[index].vote_cost = abs(green_votes - votes_to_win);
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
            cout << state.districts[i].municipalities[j].x << " " << state.districts[i].municipalities[j].y <<" ";
        }
        cout << endl;
    }
    cout << endl << endl;;
}

void print_number_green_states(const State &state) {
    int green_districts = 0;
    for(const auto &district : state.districts) {
        int votes_to_win =(district.municipalities.size() * 100) / 2 + 1;
        int votes = 0;

        for(const auto &mun: district.municipalities)
            votes += mun.votes;

        if (votes >= votes_to_win)
            green_districts += 1;
    }

    cout << green_districts << endl;
}

tuple<int, int> find_district_swap(const State &state, int iteration_count) {
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

    return make_tuple(worst_district_index, worst_mun_index);
}

tuple<int, int> find_vote_swap(const State &state) {
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    uniform_int_distribution<int> random_district_dist(0.0, state.nb_districts - 1);
    default_random_engine generator(seed);
    int district_index = random_district_dist(generator);

    uniform_int_distribution<int> random_mun_dist(0.0, state.districts[district_index].municipalities.size() - 1);
    int mun_index = random_mun_dist(generator);
    return make_tuple(district_index, mun_index);
}

State Search_new_state(const State &current_state, int district_index, int municipality_index) {
    State best_state(current_state);
    best_state.distance_cost = numeric_limits<int>::max();

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
                int distance_x = abs(district.municipalities[i].x - district.municipalities[j].x);
                int distance_y = abs(district.municipalities[i].y - district.municipalities[j].y);
                int distance = distance_x + distance_y;

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

vector<vector<Municipality>> build_n_return_solution(const State &final_state){
    vector<vector<Municipality>> final_solution;

    for(int i = 0; i < final_state.nb_districts; i++){
        vector<Municipality> new_vector;
        final_solution.push_back(new_vector);

        for(int j = 0; j < final_state.districts[i].municipalities.size(); j++){
            final_solution[i].push_back(final_state.districts[i].municipalities[j]);
        }
    }

    return final_solution;
}

vector<vector<Municipality>> Valid_State_Local_Search(const vector<Municipality> &municipalities_, int rows, int cols, int nb_district, int max_non_improving_iterations, vector<vector<float>> centers,  bool print_) {
    State current_state(municipalities_, rows,cols, nb_district, centers);
    State best_state(current_state);

    int non_improving_iterations = 0;
    int iteration_counter = 0;

    while (non_improving_iterations < max_non_improving_iterations){
        non_improving_iterations++;
        tuple<int, int> random_indexes = find_district_swap(current_state, ++iteration_counter);
        current_state = Search_new_state(current_state, get<0>(random_indexes), get<1>(random_indexes));

        if (current_state.distance_cost < best_state.distance_cost) {
            best_state = current_state;
            non_improving_iterations = 0;

            if(validate_state(best_state))
                return build_n_return_solution(best_state);
        }
    }

    if(validate_state(best_state))
        return build_n_return_solution(best_state);

    return vector<vector<Municipality>>();
}

State Valid_State_Search(const State &initial_state, int max_non_improving_iterations,  bool print_) {
    State current_state(initial_state);
    State best_state(current_state);
    int non_improving_iterations = 0;
    int iteration_counter = 0;

    while (non_improving_iterations < max_non_improving_iterations){
        non_improving_iterations++;
        tuple<int, int> random_indexes = find_district_swap(current_state, ++iteration_counter);
        current_state = Search_new_state(current_state, get<0>(random_indexes), get<1>(random_indexes));
        if (current_state.distance_cost < best_state.distance_cost) {
            best_state = current_state;
            non_improving_iterations = 0;

            if(validate_state(best_state)) {
                return best_state;
            }
        }
    }

    return best_state;
}

struct Solver {

    State Votes_Local_Search(const State &initial_state, int max_non_improving_iterations, bool show_solution) {
        State current_state(initial_state);
        State best_state(current_state);

        int best_score = numeric_limits<int>::max();
        bool active = true;

        while (active) {
            tuple<int, int> random_indexes = find_vote_swap(current_state);
            current_state = Search_new_vote_state(current_state, get<0>(random_indexes), get<1>(random_indexes));

            if (current_state.vote_cost < best_state.vote_cost) {
                best_state = current_state;

                if (best_state.vote_cost < best_score) {
                    if (show_solution)
                        print_solution(best_state);
                    else
                        print_number_green_states(best_state);

                    best_score = best_state.vote_cost;
                }
            }

            if (PyErr_CheckSignals() != 0) {
                active = false;
            }

        }

        return best_state;
    }

    void Optimise_Votes(const vector<vector<Municipality>> &municipalities_, int rows, int cols, int nb_district,
                        int max_non_improving_iterations, const vector<vector<float>> &centers,  bool print_) {

        State initial_state(municipalities_, rows,cols, nb_district, centers);

        if (!validate_state(initial_state)) {
            return;
        }

        initial_state.initialize_vote_cost();

        if (print_)
            print_solution(initial_state);
        else
            print_number_green_states(initial_state);

        State final_state = Votes_Local_Search(initial_state, max_non_improving_iterations, print_);

        if (print_)
            print_solution(initial_state);
        else
            print_number_green_states(initial_state);
    }
};


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

    py::class_<Solver>(m, "Solver")
            .def(py::init())
            .def("Optimise_Votes", &Solver::Optimise_Votes, "votes local search");
}
