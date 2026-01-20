# Find SFML
find_package(
  SFML 2.5
  COMPONENTS graphics window system
  REQUIRED)
find_package(fmt REQUIRED)

file(GLOB SRC_FILES src/*.cpp)

foreach(src_file ${SRC_FILES})
  # Get the filename without extension
  get_filename_component(EXECUTABLE ${src_file} NAME_WE)

  # Create an executable with that name
  add_executable(${EXECUTABLE} ${src_file})

  # Add include path
  target_include_directories(${EXECUTABLE} PUBLIC include)

  # Link SFML
  target_link_libraries(${EXECUTABLE} PRIVATE simlab)

  install(TARGETS ${EXECUTABLE} RUNTIME DESTINATION bin)
endforeach()

file(GLOB_RECURSE SIMLAB_FILES src/simlab/*.cpp)
add_library(simlab STATIC ${SIMLAB_FILES})
target_include_directories(simlab PUBLIC include)
target_link_libraries(
  simlab
  PRIVATE sfml-graphics sfml-window sfml-system
  PUBLIC fmt::fmt)
