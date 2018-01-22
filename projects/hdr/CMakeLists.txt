cmake_minimum_required(VERSION 2.6)

PROJECT(itkHighDynamicRangeImage)

SET(HDR_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/include)

add_subdirectory (src)
add_subdirectory (apps)
IF(BUILD_TESTS)
    add_subdirectory (tests)
ENDIF(BUILD_TESTS)