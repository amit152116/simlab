file(GLOB SRC_FILES src/*.cpp)

add_library(simlab_lib ${SRC_FILES})

target_include_directories(simlab_lib PUBLIC include)

target_link_libraries(simlab_lib PUBLIC sfml-graphics sfml-window sfml-system)
