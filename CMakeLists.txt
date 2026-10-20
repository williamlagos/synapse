cmake_minimum_required(VERSION 3.14)
project(synapse C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(FetchContent)

# ---------------------------------------------------------------------------
# Find system libuv via pkg-config
# ---------------------------------------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUV REQUIRED libuv)

# ---------------------------------------------------------------------------
# Locate or fetch libsockets (uv_sockets)
# ---------------------------------------------------------------------------
find_library(SYSTEM_UV_SOCKETS NAMES uv_sockets uv_sockets-static)
find_path(SYSTEM_UV_SOCKETS_INCLUDE NAMES uv_sockets.h)

if(SYSTEM_UV_SOCKETS AND SYSTEM_UV_SOCKETS_INCLUDE)
    message(STATUS "Using system libsockets: ${SYSTEM_UV_SOCKETS}")
    add_library(uv_sockets_dep UNKNOWN IMPORTED)
    set_target_properties(uv_sockets_dep PROPERTIES
        IMPORTED_LOCATION "${SYSTEM_UV_SOCKETS}"
        INTERFACE_INCLUDE_DIRECTORIES "${SYSTEM_UV_SOCKETS_INCLUDE};${LIBUV_INCLUDE_DIRS}")
else()
    set(SOCKETS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    if(DEFINED LIBSOCKETS_SOURCE_DIR)
        FetchContent_Declare(
            libsockets
            SOURCE_DIR "${LIBSOCKETS_SOURCE_DIR}"
        )
    else()
        FetchContent_Declare(
            libsockets
            GIT_REPOSITORY https://github.com/williamlagos/libsockets.git
            GIT_TAG        master
        )
    endif()
    FetchContent_MakeAvailable(libsockets)
    add_library(uv_sockets_dep ALIAS uv_sockets-static)
endif()

# ---------------------------------------------------------------------------
# Synapse daemon, impulse plugin, and synapse-client CLI
# ---------------------------------------------------------------------------
add_executable(synapse
    src/syn.c
    src/utils.c
    src/sensor.c
    src/process.c
    src/switcher.c)

target_include_directories(synapse PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${LIBUV_INCLUDE_DIRS})
target_link_directories(synapse PRIVATE ${LIBUV_LIBRARY_DIRS})
target_link_libraries(synapse PRIVATE uv_sockets_dep ${LIBUV_LIBRARIES})

add_library(impulse SHARED src/impulse.c)
set_target_properties(impulse PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_executable(synapse-client src/client.c)
target_include_directories(synapse-client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${LIBUV_INCLUDE_DIRS})
target_link_directories(synapse-client PRIVATE ${LIBUV_LIBRARY_DIRS})
target_link_libraries(synapse-client PRIVATE uv_sockets_dep ${LIBUV_LIBRARIES})

# ---------------------------------------------------------------------------
# Installation rules (for PKGBUILD & container builds)
# ---------------------------------------------------------------------------
install(TARGETS synapse synapse-client
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(TARGETS impulse
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/default.cfg
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/synapse)
