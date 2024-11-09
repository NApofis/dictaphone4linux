cmake_minimum_required(VERSION 3.2)

project(dictaphone)

add_executable(${PROJECT_NAME} main.cpp)

file (GLOB TESTS_LIST
        ${CMAKE_CURRENT_SOURCE_DIR}/list_tests/*.cpp
)

add_executable(tests ${TESTS_LIST})
add_subdirectory(./googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_BINARY_DIR})

target_link_libraries(tests
        gtest
        gtest_main
 )

