cmake_minimum_required(VERSION 3.10)
project(SimLab)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # allows shared lib use
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Put all executables in build/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find SFML
find_package(
  SFML 2.5
  COMPONENTS graphics window system
  REQUIRED)
find_package(fmt REQUIRED)

# Add common lib
add_subdirectory(lib)

# Automatically add all subdirectories in src/ as subprojects
file(
  GLOB SUBDIRS
  LIST_DIRECTORIES true
  src/*)

foreach(subdir ${SUBDIRS})
  if(IS_DIRECTORY ${subdir})
    add_subdirectory(${subdir})
  endif()
endforeach()
