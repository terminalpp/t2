# CMake target to build tpp-bypass as part of the whole suite build. 

if(ARCH_LINUX) 
    project(ropen)
    find_package(Threads REQUIRED)
    find_library(LUTIL util)
    file(GLOB_RECURSE SRC "ropen.cpp")
    add_executable(ropen ${SRC})

    if(INSTALL STREQUAL ropen)
        install(TARGETS ropen DESTINATION bin COMPONENT ropen)
    endif()

endif()