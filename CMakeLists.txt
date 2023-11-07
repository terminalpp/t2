cmake_minimum_required (VERSION 3.8)

if(${CMAKE_VERSION} VERSION_GREATER "3.14")
    cmake_policy(SET CMP0091 NEW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# The pseudoproject for the tools defined in this cmake. 
project(t2)

set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED true)

# architecture macros (ARCH_WINDOWS, ARCH_LINUX, ARCH_BSD, ARCH_MACOS, ARCH_UNIX)
include(${CMAKE_CURRENT_SOURCE_DIR}/helpers/platform.cmake)

include_directories(${CMAKE_SOURCE_DIR})

# Subprojects
# ===========
#
# All libraries and executables produced by the main buildscript are included now. Some subprojects only make sense for certain platforms, see their respective CMakeLists.txt for details.
add_subdirectory("libtpp")
add_subdirectory("bypass")
add_subdirectory("ropen")
add_subdirectory("utils")
add_subdirectory("tests")
