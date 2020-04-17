project(synapse-sensors)
cmake_minimum_required(VERSION 3.5)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(NM REQUIRED libnm)
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${NM_INCLUDE_DIRS})
add_library(network SHARED sensor/network/network.c sensor/network/connections.c sensor/network/configurations.c)
target_link_libraries(network INTERFACE ${GLIB_LDFLAGS})

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
add_library(event SHARED sensor/event/event.c)
target_link_libraries(event ${SDL2_LIBRARIES})

add_library(stream SHARED sensor/stream/stream.c)