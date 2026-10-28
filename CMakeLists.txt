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

file(GLOB CORE_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM CORE_SOURCES "${PROJECT_SOURCE_DIR}/src/main.cpp")
add_library(railway_core STATIC ${CORE_SOURCES})
target_include_directories(railway_core PUBLIC inc lib/include lib/template)
target_compile_features(railway_core PUBLIC cxx_std_20)
set_target_properties(railway_core PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(railway_core PRIVATE -Wall -Wextra -Werror)
target_link_libraries(railway_core PUBLIC "${PROJECT_SOURCE_DIR}/lib/libftpp.a")
add_dependencies(railway_core network)

add_executable(network_window gui/main.cpp gui/window.cpp gui/simulation_controller.cpp)
target_compile_definitions(network_window PRIVATE NETWORK_PROJECT_ROOT="${PROJECT_SOURCE_DIR}")
target_compile_features(network_window PRIVATE cxx_std_20)
set_target_properties(network_window PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(network_window PRIVATE -Wall -Wextra -Werror)
target_link_libraries(network_window PRIVATE railway_core X11::X11 PkgConfig::XFT)

include(CTest)
if(BUILD_TESTING)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_test(NAME loading COMMAND "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/tests/loading.py" "${PROJECT_SOURCE_DIR}")
    set_tests_properties(loading PROPERTIES TIMEOUT 20)
    add_test(NAME network_simulation COMMAND "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/tests/network_simulation.py" "${PROJECT_SOURCE_DIR}")
    set_tests_properties(network_simulation PROPERTIES TIMEOUT 30)
    add_test(NAME scenario_data COMMAND "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/tests/scenario_data.py" "${PROJECT_SOURCE_DIR}")
    set_tests_properties(scenario_data PROPERTIES TIMEOUT 10)
    add_test(NAME pathfinder COMMAND "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/tests/pathfinder.py" "${PROJECT_SOURCE_DIR}")
    set_tests_properties(pathfinder PROPERTIES TIMEOUT 30)
    add_test(NAME daily_simulation COMMAND "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/tests/daily_simulation.py" "${PROJECT_SOURCE_DIR}")
    set_tests_properties(daily_simulation PROPERTIES TIMEOUT 60)
    add_executable(simulation_core_test tests/simulation_core.cpp)
    target_compile_options(simulation_core_test PRIVATE -Wall -Wextra -Werror)
    target_link_libraries(simulation_core_test PRIVATE railway_core)
    add_test(NAME simulation_core COMMAND simulation_core_test)
    set_tests_properties(simulation_core PROPERTIES TIMEOUT 10)
    add_test(NAME headers COMMAND "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/tests/headers.py" "${PROJECT_SOURCE_DIR}" "${CMAKE_CXX_COMPILER}")
    set_tests_properties(headers PROPERTIES TIMEOUT 30)
    add_executable(gui_controller_test tests/gui_controller.cpp gui/simulation_controller.cpp)
    target_include_directories(gui_controller_test PRIVATE gui)
    target_compile_options(gui_controller_test PRIVATE -Wall -Wextra -Werror)
    target_link_libraries(gui_controller_test PRIVATE railway_core)
    add_test(NAME gui_controller COMMAND gui_controller_test)
    set_tests_properties(gui_controller PROPERTIES TIMEOUT 20)
    add_executable(gui_smoke_test tests/gui_smoke.cpp)
    target_compile_features(gui_smoke_test PRIVATE cxx_std_20)
    target_compile_options(gui_smoke_test PRIVATE -Wall -Wextra -Werror)
    target_link_libraries(gui_smoke_test PRIVATE X11::X11)
endif()
