################################################################################
# Simplified CMakeLists.txt for pure C++ HALCON project
# Removed HDevEngine and resource dependencies
################################################################################

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(match CXX)

# Set HALCONROOT if available
if(DEFINED ENV{HALCONROOT} AND NOT $ENV{HALCONROOT} STREQUAL "")
  message("Using HALCONROOT " $ENV{HALCONROOT})
  STRING(REGEX REPLACE "\\\\" "/" HALCONROOT $ENV{HALCONROOT}) 
endif()

# ===================================================================================================
# Source and header files
# ===================================================================================================
SET(SOURCE match.cpp)
SET(HEADER match.h)

# ===================================================================================================
# Dependencies - Only HALCON (no HDevEngine needed)
# ===================================================================================================
# Direct HALCON configuration without FindHalcon.cmake
if(DEFINED ENV{HALCONROOT} AND NOT $ENV{HALCONROOT} STREQUAL "")
  set(HALCON_ROOT $ENV{HALCONROOT})
  message(STATUS "Using HALCON from environment variable: ${HALCON_ROOT}")
else()
  # Try to find HALCON-20.11-Steady installation in C: and D: drives
  set(POSSIBLE_HALCON_PATHS
    "C:/Program Files/MVTec/HALCON-20.11-Steady"
    "D:/Program Files/MVTec/HALCON-20.11-Steady"
  )
  
  set(HALCON_ROOT "")
  foreach(PATH ${POSSIBLE_HALCON_PATHS})
    if(EXISTS ${PATH})
      set(HALCON_ROOT ${PATH})
      message(STATUS "Found HALCON-20.11-Steady at: ${HALCON_ROOT}")
      break()
    endif()
  endforeach()
  
  if(HALCON_ROOT STREQUAL "")
    message(FATAL_ERROR "HALCON-20.11-Steady not found! Please set HALCONROOT environment variable or install HALCON-20.11-Steady in C: or D: drive.")
  endif()
endif()

# Set HALCON include directories
set(HALCON_INC_DIRS 
    ${HALCON_ROOT}/include
    ${HALCON_ROOT}/include/halconcpp
)

# Set HALCON libraries
set(HALCON_LIBS
    ${HALCON_ROOT}/lib/x64-win64/halconcpp.lib
    ${HALCON_ROOT}/lib/x64-win64/halcon.lib
)

set (INC_DIRS ${INC_DIRS} ${HALCON_INC_DIRS})
set (LIBS ${LIBS} ${HALCON_LIBS})

# ===================================================================================================
# Build executable
# ===================================================================================================
if(${UNIX})
    set (LIBS ${LIBS} pthread)
endif()

add_executable(${PROJECT_NAME} ${SOURCE} ${HEADER})
set (INC_DIRS ${INC_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${INC_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)

# ===================================================================================================
# Note: No resource copying needed for pure C++ version
# ===================================================================================================
