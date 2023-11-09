cmake_minimum_required (VERSION 3.5)

# CMake target to build tpp-bypass as part of the whole suite build. 

if(ARCH_LINUX) 
    project(local-pty-test)
    find_package(Threads REQUIRED)
    find_library(LUTIL util)
    file(GLOB_RECURSE SRC "local-pty-test.cpp")
    add_executable(local-pty-test ${SRC})
    target_link_libraries(local-pty-test ${CMAKE_THREAD_LIBS_INIT} ${LUTIL} libtpp)
endif()