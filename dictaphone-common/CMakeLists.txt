cmake_minimum_required(VERSION 3.5)
project(dictaphone4linux-common)

add_library(${PROJECT_NAME} STATIC)
add_library(libdictaphonecommon ALIAS ${PROJECT_NAME})

file (GLOB_RECURSE ${PROJECT_NAME}_SRC_FILES src/*.cpp)
target_sources(${PROJECT_NAME}
        PRIVATE
        ${${PROJECT_NAME}_SRC_FILES}
)

include(ExternalProject)

ExternalProject_Add(portaudio
        GIT_REPOSITORY    https://github.com/PortAudio/portaudio.git
        GIT_TAG           "master"
        SOURCE_DIR        ""
        BUILD_IN_SOURCE   1
        CONFIGURE_COMMAND cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=./build
        BUILD_COMMAND     make
        INSTALL_COMMAND   make install
        TEST_COMMAND      ""
        UPDATE_COMMAND    ""
        UPDATE_DISCONNECTED 1
        EXCLUDE_FROM_ALL TRUE
)

ExternalProject_Get_Property( portaudio SOURCE_DIR BINARY_DIR )
set( PORTAUDIO_INCLUDE_DIR "${SOURCE_DIR}/build/include" )
set( PORTAUDIO_STATIC_LIB ${BINARY_DIR}/build/lib/libportaudio.a )

add_dependencies( ${PROJECT_NAME} portaudio)

find_package(ALSA)
find_package(PulseAudio)

target_link_libraries(${PROJECT_NAME}
        ${PORTAUDIO_STATIC_LIB}
        ${ALSA_LIBRARIES}
        ${PULSEAUDIO_LIBRARY}
)

target_include_directories(${PROJECT_NAME}
        PUBLIC
            include
            ${PORTAUDIO_INCLUDE_DIR}
        PRIVATE
            ${ALSA_INCLUDE_DIRS}
            ${PULSEAUDIO_INCLUDE_DIR}
)