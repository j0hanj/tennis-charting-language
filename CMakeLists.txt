cmake_minimum_required(VERSION 3.20)
project(tennis_charting_language LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

# --- core library (grows over time) --------------------------------------
add_library(tcl_core STATIC
  src/scoring/score.cpp
  src/lexer/lexer.cpp
  src/lexer/render.cpp
  src/ast/ast.cpp
  src/parser/parser.cpp
  src/viz/court.cpp
)
target_include_directories(tcl_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- CLI -----------------------------------------------------------------
add_executable(tcl src/cli/main.cpp)
target_link_libraries(tcl PRIVATE tcl_core)

# --- tests -------------------------------------------------------------
include(CTest)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
