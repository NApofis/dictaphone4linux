cmake_minimum_required(VERSION 3.5)

project(dictaphone4linux)
set(CMAKE_CXX_STANDARD 20)

set(PROGRAM_ROOT_DIRECTORY "/opt/dictaphone4linux")
configure_file(dictaphone-common/include/const.h.in dictaphone-common/const.h)
configure_file(${PROJECT_SOURCE_DIR}/deb/postinst.in postinst)

set( BUILD_TZ_LIB ON CACHE INTERNAL "build/install of TZ library")
set( USE_SYSTEM_TZ_DB ON CACHE INTERNAL "Use the operating system's timezone database" )
add_subdirectory(./external/date)

add_subdirectory(dictaphone-common)
add_subdirectory(dictaphone-cli)
add_subdirectory(dictaphone-core)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_CONTACT tihonovn.a@yandex.ru)
set(CPACK_PACKAGE_VERSION ${RELEASE_VERSION})
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libasound2-dev, libpulse-dev")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/postinst")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}-package"
        CACHE STRING "The resulting package name"
)
include(CPack)

#enable_testing()

