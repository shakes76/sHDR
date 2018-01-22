cmake_minimum_required(VERSION 2.6)

INCLUDE_DIRECTORIES(${HDR_INCLUDE_PATH} ${ITK_INCLUDE_DIRS} ${LIBRARY_INCLUDE_PATH}/itk-ext ${SMILI_INCLUDE_DIR}/itk-ext ${SMILI_INCLUDE_DIR} ${SMILI_INCLUDE_DIR}/vtk-ext ${SMILI_INCLUDE_DIR}/itkVTKGlue)

link_directories(${ITK_LIBRARY_DIRS})

#~ ADD_DEFINITIONS(-DSMILI_DLL)

# Send binaries to testbin dir
SET(EXECUTABLE_OUTPUT_PATH ${TEST_EXECUTABLE_OUTPUT_PATH})

ADD_EXECUTABLE(itkHighDynamicRangeImagePreprocessingTest MACOSX_BUNDLE itkHighDynamicRangeImagePreprocessingTest.cxx)
TARGET_LINK_LIBRARIES(itkHighDynamicRangeImagePreprocessingTest ${ITK_LIBRARIES} ${ZLIB_LIBRARIES})

ADD_EXECUTABLE(itkHighDynamicRangeImageMLICTest MACOSX_BUNDLE itkHighDynamicRangeImageMLICTest.cxx)
TARGET_LINK_LIBRARIES(itkHighDynamicRangeImageMLICTest ${ITK_LIBRARIES} ${ZLIB_LIBRARIES} ${SMILI_LIBRARIES} ${VTK_LIBRARIES})

ADD_EXECUTABLE(itkHighDynamicRangeImageMSDETest MACOSX_BUNDLE itkHighDynamicRangeImageMSDETest.cxx)
TARGET_LINK_LIBRARIES(itkHighDynamicRangeImageMSDETest ${ITK_LIBRARIES} ${ZLIB_LIBRARIES} ${SMILI_LIBRARIES} ${VTK_LIBRARIES})

#~ ADD_EXECUTABLE(itkHighDynamicRangeImageFilterTest MACOSX_BUNDLE itkHighDynamicRangeImageFilterTest.cxx)
#~ TARGET_LINK_LIBRARIES(itkHighDynamicRangeImageFilterTest ${ITK_LIBRARIES} ${ZLIB_LIBRARIES})
