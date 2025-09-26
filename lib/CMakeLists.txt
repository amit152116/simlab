file(GLOB SRC_FILES src/*.cpp)

foreach(src_file ${SRC_FILES})
  # Get the filename without extension
  get_filename_component(exec_name ${src_file} NAME_WE)

  # Create an executable with that name
  add_executable(${exec_name} ${src_file})

  # Add include path
  target_include_directories(${exec_name} PUBLIC include)

  # Link SFML
  target_link_libraries(${exec_name} PRIVATE simlab)
endforeach()

file(GLOB simlab_FILES src/simlab/*.cpp)
add_library(simlab STATIC ${simlab_FILES})
target_include_directories(simlab PUBLIC include)
target_link_libraries(simlab PRIVATE sfml-graphics sfml-window sfml-system)
