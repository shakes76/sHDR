cmake_minimum_required(VERSION 2.6)

INCLUDE_DIRECTORIES(${HDR_INCLUDE_PATH} ${ITK_INCLUDE_DIRS} ${LIBRARY_INCLUDE_PATH}/itk-ext ${LIBRARY_INCLUDE_PATH} ${SMILI_INCLUDE_DIR}/itk-ext ${SMILI_INCLUDE_DIR} ${SMILI_INCLUDE_DIR}/vtk-ext ${SMILI_INCLUDE_DIR}/itkVTKGlue ${SMILI_INCLUDE_DIR}/Qt)

link_directories(${ITK_LIBRARY_DIRS})

IF(USE_ITK)
  set(HDR_HEADERS
    #itk extensions
    ${HDR_INCLUDE_PATH}/itkHighDynamicRangeImageFilter.h
    ${HDR_INCLUDE_PATH}/itkHighDynamicRangeImageFilter.hxx
    )
ENDIF(USE_ITK)

# Send binaries to testbin dir
SET(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH})

IF(WIN32)
  MESSAGE("Optimisation flags /O2 and /Ob2 cause the Bilateral Filter to crash. Use /O1 and/or /Ob1 instead.")
ENDIF(WIN32)

ADD_EXECUTABLE(itkHighDynamicRangeImageApp MACOSX_BUNDLE ${HDR_HEADERS} itkHighDynamicRangeImageApp.cxx)
TARGET_LINK_LIBRARIES(itkHighDynamicRangeImageApp ${ITK_LIBRARIES} ${ZLIB_LIBRARIES} ${SMILI_LIBRARIES} ${VTK_LIBRARIES})

#other apps
IF(BUILD_GUI)
    add_subdirectory (sHDR)
ENDIF(BUILD_GUI)
