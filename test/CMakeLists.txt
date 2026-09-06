include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.4
)
FetchContent_MakeAvailable(Catch2)

add_executable(tcl_tests
  unit/smoke_test.cpp
  unit/scoring_test.cpp
  unit/lexer_test.cpp
)
target_link_libraries(tcl_tests PRIVATE tcl_core Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(tcl_tests)
