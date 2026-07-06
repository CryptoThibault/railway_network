cmake_minimum_required(VERSION 3.20)

project(network LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------
# Executable
# -------------------------
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

add_executable(network ${SOURCES})

# -------------------------
# Includes
# -------------------------
target_include_directories(network PRIVATE
    inc
    lib/include
    lib/template
)

# -------------------------
# Compiler flags
# -------------------------
target_compile_options(network PRIVATE
    -Wall
    -Wextra
    -Werror
)

# -------------------------
# OUTPUT AT PROJECT ROOT
# -------------------------
set_target_properties(network PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# -------------------------
# External library
# -------------------------
set(LIB_NAME libftpp.a)
set(LIB_PATH ${CMAKE_SOURCE_DIR}/lib/${LIB_NAME})

add_custom_command(
    OUTPUT ${LIB_PATH}
    COMMAND ${CMAKE_MAKE_PROGRAM}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/lib
    COMMENT "[libftpp] Building external library"
)

add_custom_target(ftpp_lib DEPENDS ${LIB_PATH})

add_dependencies(network ftpp_lib)

# -------------------------
# Link library
# -------------------------
target_link_libraries(network PRIVATE ${LIB_PATH})

# -------------------------
# -no-pie
# -------------------------
target_link_options(network PRIVATE -no-pie)