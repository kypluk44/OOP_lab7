cmake_minimum_required(VERSION 3.16)

project(task7_var20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(npc_lib
    src/npc.cpp
    src/ork.cpp
    src/squirrel.cpp
    src/druid.cpp
    src/factory.cpp
    src/observers.cpp
    src/battle.cpp
)

target_include_directories(npc_lib PUBLIC include)

add_executable(task7 main.cpp)
target_link_libraries(task7 PRIVATE npc_lib)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(npc_tests tests/npc_test.cpp)
target_link_libraries(npc_tests PRIVATE npc_lib GTest::gtest_main)

add_test(NAME npc_tests COMMAND npc_tests)
