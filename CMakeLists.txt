project(synapse)
cmake_minimum_required(VERSION 3.5)
add_subdirectory(libuv)
link_directories(libuv)
add_executable(synapse src/syn.c src/utils.c src/sensor.c src/process.c)
add_library(impulse SHARED src/impulse.c)
target_link_libraries(synapse uv)