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
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_CONTACT tihonovn.a@yandex.ru)
set(CPACK_PACKAGE_VERSION ${RELEASE_VERSION})
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}-package"
        CACHE STRING "The resulting package name"
)
include(CPack)

#enable_testing()

