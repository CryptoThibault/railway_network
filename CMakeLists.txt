cmake_minimum_required(VERSION 3.20)

project(railway_network LANGUAGES CXX)

find_program(MAKE_EXECUTABLE NAMES gmake make REQUIRED)
find_package(X11 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(XFT REQUIRED IMPORTED_TARGET xft)

add_custom_target(network ALL
    COMMAND "${MAKE_EXECUTABLE}" -C "${PROJECT_SOURCE_DIR}" all
    USES_TERMINAL
    VERBATIM
)

add_executable(network_window gui/main.cpp gui/window.cpp gui/simulation_process.cpp gui/telemetry.cpp)
target_compile_definitions(network_window PRIVATE NETWORK_PROJECT_ROOT="${PROJECT_SOURCE_DIR}")
target_compile_features(network_window PRIVATE cxx_std_20)
set_target_properties(network_window PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(network_window PRIVATE -Wall -Wextra -Werror)
target_link_libraries(network_window PRIVATE X11::X11 PkgConfig::XFT)
add_dependencies(network_window network)

include(CTest)
if(BUILD_TESTING)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_test(NAME simulation COMMAND "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/tests/simulation.py" "${PROJECT_SOURCE_DIR}")
    set_tests_properties(simulation PROPERTIES WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}" TIMEOUT 20)
    add_executable(simulation_process_test tests/simulation_process.cpp gui/simulation_process.cpp gui/telemetry.cpp)
    target_include_directories(simulation_process_test PRIVATE gui)
    target_compile_features(simulation_process_test PRIVATE cxx_std_20)
    target_compile_options(simulation_process_test PRIVATE -Wall -Wextra -Werror)
    add_dependencies(simulation_process_test network)
    add_test(NAME simulation_process COMMAND simulation_process_test "${PROJECT_SOURCE_DIR}")
    set_tests_properties(simulation_process PROPERTIES TIMEOUT 15)
endif()
