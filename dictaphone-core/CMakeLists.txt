cmake_minimum_required(VERSION 3.5)

project(dictaphone-core)

set(CMAKE_CXX_STANDARD 20)

file (GLOB FILES
        main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*
)
add_executable(${PROJECT_NAME} ${FILES} )


target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE libdictaphonecommon)


install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)