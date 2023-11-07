# CMake target to build tpp-bypass as part of the whole suite build. 

if(ARCH_LINUX) 
    project(tpp-bypass)
    find_package(Threads REQUIRED)
    find_library(LUTIL util)
    file(GLOB_RECURSE SRC "tpp-bypass.cpp")
    add_executable(tpp-bypass ${SRC})
    target_link_libraries(tpp-bypass ${CMAKE_THREAD_LIBS_INIT} ${LUTIL})

    if(INSTALL STREQUAL tpp-bypass)
        install(TARGETS tpp-bypass DESTINATION bin COMPONENT tpp-bypass)
    endif()

endif()