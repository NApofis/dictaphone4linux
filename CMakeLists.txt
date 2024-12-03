cmake_minimum_required(VERSION 3.5)

project(dictaphone)
set(CMAKE_CXX_STANDARD 20)

set(PROGRAM_ROOT_DIRECTORY "/opt/dictaphone4linux")
file (GLOB TESTS_LIST
        ${CMAKE_CURRENT_SOURCE_DIR}/list_tests/*.cpp
)

add_executable(tests ${TESTS_LIST})
add_subdirectory(./external/googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_BINARY_DIR})

set( BUILD_TZ_LIB ON CACHE INTERNAL "build/install of TZ library")
set( USE_SYSTEM_TZ_DB ON CACHE INTERNAL "Use the operating system's timezone database" )
add_subdirectory(./external/date)

target_link_libraries(tests
        gtest
        gtest_main
 )
add_subdirectory(dictaphone-common)
add_subdirectory(dictaphone-cli)
add_subdirectory(dictaphone-core)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_CONTACT tihonovn.a@yandex.ru)
set(CPACK_PACKAGE_VERSION ${RELEASE_VERSION})
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libasound2-dev, libpulse-dev")
set(CPACK_PACKAGE_DIRECTORY "${PROGRAM_ROOT_DIRECTORY}")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}-package"
        CACHE STRING "The resulting package name"
)
include(CPack)

#enable_testing()

