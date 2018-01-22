cmake_minimum_required(VERSION 2.8)
 
PROJECT(sHDR)

#Set output paths
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(TEST_EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/testbin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(LIBRARY_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/include)
set(CMAKE_MODULE_PATH "${sHDR_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

#load modules
#-----------------------------------------------------------------------------
include(zlib)
include(find) #also Finds SMILI
include(macros)
include(version)

#Add compiler flags
#GCC
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fwrapv")
endif(CMAKE_COMPILER_IS_GNUCC)
#MSVC
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /bigobj")
endif(MSVC)

# Enable/Disable Build the testing
OPTION(BUILD_SHARED_LIBS "Build the Libraries as shared libraries" ON)
OPTION(BUILD_PROJECTS "Build the projects." ON)
OPTION(BUILD_APPLICATIONS "Build the applications." ON)
OPTION(BUILD_TESTS "Build the tests" OFF)

#dependent options, triggered if another option is set
DEPENDENT_OPTION(USE_ITK "Build all the ITK components" ON "ITK_LIBRARIES" OFF)
DEPENDENT_OPTION(USE_VTK "Build all the VTK components" ON "VTK_USE_FILE_INCLUDED" OFF)

#Check ITK/VTK statuses
IF(USE_ITK AND NOT USE_VTK)
  add_definitions(-DITK_ONLY)
  message("Disabling non-ITK components. Enable USE_VTK to enable VTK components.")
ENDIF(USE_ITK AND NOT USE_VTK)
IF(USE_VTK AND NOT USE_ITK)
  add_definitions(-DVTK_ONLY)
  message("Disabling non-VTK components. Enable USE_ITK to enable ITK components.")
ENDIF(USE_VTK AND NOT USE_ITK)
IF(USE_VTK)
  message("VTK Version: ${VTK_VERSION}")
ENDIF(USE_VTK)
IF(USE_ITK)
  message("ITK Version: ${ITK_VERSION_MAJOR}.${ITK_VERSION_MINOR}")
ENDIF(USE_ITK)

#advanced options


#Important variables for configurations
set(sHDR_INCLUDE_DIRS ${LIBRARY_INCLUDE_PATH} ${LIBRARY_INCLUDE_PATH}/itk-ext)

IF(APPLE)
    #Release RPATH handling for MAC OSX
    # use, i.e. don't skip the full RPATH for the build tree
    SET(CMAKE_MACOSX_RPATH OFF)
    MESSAGE("MAC OSX RPATH STUFF")
    cmake_policy(SET CMP0042 NEW)
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

    # when building, don't use the install RPATH already
    # (but later on when installing)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
    #SET(CMAKE_SKIP_RPATH TRUE)
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

    # add the automatically determined parts of the RPATH
    # which point to directories outside the build tree to the install RPATH
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    # the RPATH to be used when installing, but only if it's not a system directory
    LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
    IF("${isSystemDir}" STREQUAL "-1")
       SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    ENDIF("${isSystemDir}" STREQUAL "-1")
ENDIF(APPLE)

# Recurse into the "apps" subdirectory. This does not actually
# cause another cmake executable to run.
#~ add_subdirectory (src)

IF(BUILD_PROJECTS)
    add_subdirectory (projects)
ENDIF(BUILD_PROJECTS)

IF(BUILD_APPLICATIONS)
#~     add_subdirectory (apps)
ENDIF(BUILD_APPLICATIONS)

IF(BUILD_TESTS)
    add_subdirectory (tests)
ENDIF(BUILD_TESTS)
