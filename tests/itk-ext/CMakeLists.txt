cmake_minimum_required(VERSION 2.6)

PROJECT(itk-ext-Tests)

INCLUDE_DIRECTORIES(${ITK_INCLUDE_DIRS} ${LIBRARY_INCLUDE_PATH}/itk-ext)

link_directories(${ITK_LIBRARY_DIRS})

#~ ADD_DEFINITIONS(-DSMILI_DLL)

# Send binaries to testbin dir
SET(EXECUTABLE_OUTPUT_PATH ${TEST_EXECUTABLE_OUTPUT_PATH})

ADD_EXECUTABLE(itkFastBilateralImageFilterTest MACOSX_BUNDLE itkFastBilateralImageFilterTest.cxx)
TARGET_LINK_LIBRARIES(itkFastBilateralImageFilterTest ${ITK_LIBRARIES} ${ZLIB_LIBRARIES})

ADD_EXECUTABLE(itkMinimumImageFunctionTest MACOSX_BUNDLE itkMinimumImageFunctionTest.cxx)
TARGET_LINK_LIBRARIES(itkMinimumImageFunctionTest ${ITK_LIBRARIES} ${ZLIB_LIBRARIES})

#~ ADD_EXECUTABLE(itkHighDynamicRangeImageFilterTest MACOSX_BUNDLE itkHighDynamicRangeImageFilterTest.cxx)
#~ TARGET_LINK_LIBRARIES(itkHighDynamicRangeImageFilterTest ${ITK_LIBRARIES} ${ZLIB_LIBRARIES})
