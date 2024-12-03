cmake_minimum_required(VERSION 3.5)

project(dictaphone4linux-core)
set(CMAKE_CXX_STANDARD 20)

file (GLOB FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
add_executable(${PROJECT_NAME} main.cpp ${FILES})

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
            libdictaphonecommon
            date::date-tz
)


install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)