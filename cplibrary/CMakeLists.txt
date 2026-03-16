cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(cpl VERSION 1.0.0 LANGUAGES C)

add_library(cpl INTERFACE)

target_include_directories(cpl INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

install(FILES cpl.h DESTINATION include/cpl)

install(TARGETS cpl
    EXPORT cplTargets
)

install(EXPORT cplTargets
    FILE cplTargets.cmake
    NAMESPACE cpl::
    DESTINATION lib/cmake/cpl
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cplConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cplConfig.cmake"
    INSTALL_DESTINATION lib/cmake/cpl
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cplConfig.cmake"
    DESTINATION lib/cmake/cpl
)

configure_file(cpl.pc.in cpl.pc @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cpl.pc"
    DESTINATION lib/pkgconfig
)
