cmake_minimum_required(VERSION 3.10)
project(chess_env)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable PIC for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mbmi -mbmi2 -std=c++2b")

# Find Python and pybind11
find_package(Python 3 REQUIRED COMPONENTS Interpreter Development)
set(pybind11_DIR $ENV{CONDA_PREFIX}/lib/python3.10/site-packages/pybind11/share/cmake/pybind11)
find_package(pybind11 REQUIRED CONFIG)

# Create a shared library for Python bindings
add_library(${PROJECT_NAME} SHARED
    bindings/bind_perf.cpp
    src/game_state.cpp
)

# Add include directories to the library target
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)


# Link against the static library and pybind11
target_link_libraries(${PROJECT_NAME} pybind11::pybind11 Python::Python)
target_include_directories(${PROJECT_NAME} PRIVATE ${Python_INCLUDE_DIRS})

# Set output name of the shared library to match Python's naming convention (e.g., _project_name.so)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "_")

# Add an executable for main.cpp
add_executable(app src/main.cpp)

# Link the executable to the chess_env library
target_link_libraries(app ${PROJECT_NAME})


# Optionally add tests if you have them
enable_testing()

add_executable(test_core test/test_core.cpp)
add_executable(test_adapter test/test_move_adapter.cpp)
# add_executable(test_movegen test/test_movegen.cpp)
add_executable(test_golden test/test_golden.cpp)

target_link_libraries(test_core ${PROJECT_NAME})
target_link_libraries(test_adapter ${PROJECT_NAME})
# target_link_libraries(test_movegen ${PROJECT_NAME})
target_link_libraries(test_golden ${PROJECT_NAME})

target_compile_definitions(test_golden PRIVATE DATA_DIR="${CMAKE_SOURCE_DIR}/test/data")

add_test(NAME test_core COMMAND test_core)
add_test(NAME test_adapter COMMAND test_adapter)
# add_test(NAME test_movegen COMMAND test_movegen)
add_test(NAME test_golden COMMAND test_golden)


add_custom_target(tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    COMMENT "Running all tests"
)
