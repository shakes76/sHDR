cmake_minimum_required(VERSION 2.8)

# Include the directory itself as a path to include directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

PROJECT(milx-Qt-HDR)

#find libraries
include(find)

#Win32 requirement for VTK classes in DLLs
add_definitions (-DvtkRendering_EXPORTS)

#Add compiler flags
add_definitions (-DMILXQT_DLL -DMILXQT_MAKEDLL)
if (MILXQT_64)
	add_definitions (-DMILXQT_64)
endif (MILXQT_64)

#--------------------------------------------------------------------------------
# this command finds Qt4 libraries and sets all required variables
# note that it's Qt4, not QT4 or qt4
#detect Qt version (depends on VTK-Qt)
#~ IF(VTK_MAJOR_VERSION GREATER 5 AND VTK_MINOR_VERSION GREATER 0) #need 6.1 and above
#~   include(vtkQt)
#~ ENDIF(VTK_MAJOR_VERSION GREATER 5 AND VTK_MINOR_VERSION GREATER 0)
IF(VTK_QT_VERSION VERSION_GREATER "4")
  FIND_PACKAGE(Qt5Widgets REQUIRED QUIET)
  include_directories(${Qt5Widgets_INCLUDE_DIRS})
  add_definitions(${Qt5Widgets_DEFINITIONS})
  qt5_wrap_cpp(QVTKLibMocSrcs ${QVTKMocHeaders})
  set(QT_LIBRARIES ${Qt5Widgets_LIBRARIES})
ELSE(VTK_QT_VERSION VERSION_GREATER "4")
  FIND_PACKAGE(Qt4 REQUIRED QtCore QtGui QtWebkit)
  INCLUDE(${QT_USE_FILE})
ENDIF(VTK_QT_VERSION VERSION_GREATER "4")

# Add Headers into List
file(GLOB MILXQT_HDR_HEADERS ${HDR_INCLUDE_PATH}/*.h)
file(GLOB MILXQT_HDR_HEADERS_TEMPLATES ${HDR_INCLUDE_PATH}/*.txx)
file(GLOB MILXQT_HDR_HEADERS_MOC ${HDR_INCLUDE_PATH}/*.h)
file(GLOB MILXQT_HEADERS_MOC ${MILXQT_INCLUDE_DIRS}/*.h)
file(GLOB MILXQT_HDR_SRC_UIS ${PROJECT_SOURCE_DIR}/ui/*.ui)
file(GLOB MILXQT_HDR_SRC_UI_HEADERS_MOC ${PROJECT_SOURCE_DIR}/ui/*.h)
file(GLOB MILXQT_HDR_SRC_UI_CPP ${PROJECT_SOURCE_DIR}/ui/*.cpp)
file(GLOB MILXQT_HDR_SRC_RCCS ${PROJECT_SOURCE_DIR}/*.qrc)

# Add App Sources
set(HDR_SOURCES milxQtHDRMain milxQtHDRImage)

include_directories (${MILXQT_INCLUDE_DIRS} ${HDR_INCLUDE_PATH} ${ITK_INCLUDE_DIRS} ${LIBRARY_INCLUDE_PATH}/itk-ext ${SMILI_INCLUDE_DIR}/itkVTKGlue ${SMILI_INCLUDE_DIR} ${SMILI_INCLUDE_DIR}/vtk-ext ${SMILI_INCLUDE_DIR}/itk-ext ${SMILI_INCLUDE_DIR}/utils)
link_directories(${MILXQT_INSTALL_BINARY_DIR})
message("${SMILI_INCLUDE_DIR}")

IF(VTK_QT_VERSION VERSION_GREATER "4")
  # generate rules for building source files from the resources
  QT5_ADD_RESOURCES(HDR_RCCS ${MILXQT_HDR_SRC_RCCS})
  # build ui_XXX files from the XML-style .ui files
  QT5_WRAP_UI(HDR_UIS ${MILXQT_HDR_SRC_UIS})
  # this moc's the above variable and appends to the cxx sources
  QT5_WRAP_CPP(HDR_MOCS ${MILXQT_HDR_HEADERS_MOC})
  QT5_WRAP_CPP(MILXQT_MOCS ${MILXQT_HEADERS_MOC})
  QT5_WRAP_CPP(HDR_UI_MOCS ${MILXQT_HDR_SRC_UI_HEADERS_MOC})
ELSE(VTK_QT_VERSION VERSION_GREATER "4")
  # generate rules for building source files from the resources
  QT4_ADD_RESOURCES(HDR_RCCS ${MILXQT_HDR_SRC_RCCS})
  # build ui_XXX files from the XML-style .ui files
  QT4_WRAP_UI(HDR_UIS ${MILXQT_HDR_SRC_UIS})
  # this moc's the above variable and appends to the cxx sources
  QT4_WRAP_CPP(HDR_MOCS ${MILXQT_HDR_HEADERS_MOC})
  QT4_WRAP_CPP(MILXQT_MOCS ${MILXQT_HEADERS_MOC})
  QT4_WRAP_CPP(HDR_UI_MOCS ${MILXQT_HDR_SRC_UI_HEADERS_MOC})
ENDIF(VTK_QT_VERSION VERSION_GREATER "4")

add_library (milx-Qt-HDR SHARED ${MILXQT_HEADERS} ${MILXQT_HDR_HEADERS} ${HDR_HEADERS} ${HDR_SOURCES} ${HDR_MOCS} ${HDR_UI_MOCS} ${HDR_UIS} ${HDR_SRC_UI_CPP} ${HDR_RCCS} ${MILXQT_MOCS})

# Set the build version (VERSION) and the API version (SOVERSION)
SET_TARGET_PROPERTIES(milx-Qt-HDR
                      PROPERTIES
                      VERSION 1.0.0
                      SOVERSION 1)

#Needed to prevent imporoper linking when VTK 5 is installed side-by-side with VTK 6
IF("${VTK_MAJOR_VERSION}" LESS 6) #If not VTK 6 or above
  set(VTK_QT_LIBRARIES ${VTK_LIBRARIES} QVTK)
ELSE("${VTK_MAJOR_VERSION}" LESS 6) #If VTK 6 or above
  set(VTK_QT_LIBRARIES ${VTK_LIBRARIES})
ENDIF("${VTK_MAJOR_VERSION}" LESS 6)

TARGET_LINK_LIBRARIES(milx-Qt-HDR ${SMILI_LIBRARIES} ${VTK_LIBRARIES} ${VTK_QT_LIBRARIES} ${QT_LIBRARIES} ${ITK_LIBRARIES} ${ITK_REVIEW_LIBRARIES} ${ZLIB_LIBRARIES})
