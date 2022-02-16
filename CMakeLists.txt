# CMake
cmake_minimum_required(VERSION 3.21)
project(O2FS)

# C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Binaries names
set(EXECUTABLE_NAME "O2FS.Launcher")
set(LIBRARY_NAME "O2FS")

# Main directories
file(GLOB_RECURSE SRCS src/O2FS/*.cpp)
file(GLOB_RECURSE HEADERS src/O2FS/*.hpp)
file(GLOB_RECURSE HEADERS include/O2FS/*.hpp include/Logger.hpp)
file(GLOB_RECURSE EXTLIBS_HEADERS extlibs/include/*.hpp)
include_directories(src)
include_directories(include)
include_directories(extlibs/include)

# Launcher directories
file(GLOB_RECURSE LAUNCHER_SRCS src/Launcher/*.cpp)
file(GLOB_RECURSE LAUNCHER_HEADERS include/Launcher/*.hpp include/Logger.hpp)
include_directories(include/Launcher)

# Modules directories
set(MODULES_DIR "${CMAKE_SOURCE_DIR}/extlibs/lib")
set(DETOUR_LIB ${MODULES_DIR}/detours.lib)

# Source and linkage
add_library(${LIBRARY_NAME} SHARED ${SRCS} ${HEADERS} ${EXTLIBS_HEADERS})
add_executable(${EXECUTABLE_NAME} ${LAUNCHER_SRCS} ${LAUNCHER_HEADERS})
target_link_libraries(${LIBRARY_NAME} ${DETOUR_LIB})

# Output directory
set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/bin/")
file(MAKE_DIRECTORY ${OUTPUT_DIR})

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG  ${OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR}
)
set_target_properties(${LIBRARY_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG  ${OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR}
)

# Runtime Library
set_property(TARGET ${LIBRARY_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET ${EXECUTABLE_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

