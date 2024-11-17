cmake_minimum_required(VERSION 3.5)

project(dictaphone-cli)

set(CMAKE_CXX_STANDARD 20)

file (GLOB FILES
        main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
add_executable(${PROJECT_NAME} ${FILES}
        main.cpp)

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG main # Important: Specify a version or a commit hash here.
)
FetchContent_MakeAvailable(ftxui)


target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        libdictaphonecommon
)
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        libdictaphonecommon
        ftxui::screen
        ftxui::dom
        ftxui::component
        date::date-tz
)


install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)