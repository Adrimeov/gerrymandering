#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

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

PYBIND11_MODULE(Municipality, m) {

    py::class_<Municipality>(m, "Municipality")
            .def(py::init<int, int, int>())
            .def("get_x", &Municipality::get_x)
            .def("get_y", &Municipality::get_y)
            .def("get_votes", &Municipality::get_votes)
            .def("set_x", &Municipality::set_x)
            .def("set_y", &Municipality::set_y)
            .def("set_votes", &Municipality::set_votes);
}
