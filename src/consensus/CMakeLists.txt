add_executable(consensus src/main.cpp)

target_include_directories(consensus PRIVATE include)

target_link_libraries(consensus PRIVATE simlab_lib)
