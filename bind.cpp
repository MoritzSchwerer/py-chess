#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "src/game_env.hpp" // assume this is the header file for the ChessGameEnv struct

namespace py = pybind11;

PYBIND11_MODULE(chess_env, m) {

    py::class_<ChessGameEnv> chess_env(m, "ChessGameEnv");

    chess_env.def(py::init<>()); // default constructor

    chess_env.def("step", &ChessGameEnv::step, py::arg("action"));
    chess_env.def("observe", &ChessGameEnv::observe);
    chess_env.def("copy", [](const ChessGameEnv& env) {
        return ChessGameEnv(env);
    });

    py::class_<ChessObservation> chess_observation(m, "ChessObservation");

    chess_observation.def_readonly("whiteReward", &ChessObservation::whiteReward);
    chess_observation.def_readonly("blackReward", &ChessObservation::blackReward);
    chess_observation.def_readonly("isTerminated", &ChessObservation::isTerminated);

    chess_observation.def_property_readonly("observation", 
                                    [&m](const ChessObservation& co) {
                                        py::array_t<bool> arr(co.observation.size());
                                        std::copy(co.observation.begin(), co.observation.end(), arr.mutable_data());
                                        return arr;
                                    });

    chess_observation.def_property_readonly("actionMask", 
                                    [&m](const ChessObservation& co) {
                                        py::array_t<bool> arr(co.actionMask.size());
                                        std::copy(co.actionMask.begin(), co.actionMask.end(), arr.mutable_data());
                                        return arr;
                                    });
}
