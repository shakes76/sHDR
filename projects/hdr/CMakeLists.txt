cmake_minimum_required(VERSION 2.6)

PROJECT(itkHighDynamicRangeImage)

SET(HDR_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/include)

OPTION(BUILD_GUI "Build the graphical user interfaces." OFF)

IF(BUILD_GUI)
    add_subdirectory (src)
ENDIF(BUILD_GUI)
add_subdirectory (apps)
IF(BUILD_TESTS)
    add_subdirectory (tests)
ENDIF(BUILD_TESTS)