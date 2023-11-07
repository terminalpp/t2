# platform detection 
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message(STATUS "Windows platform detected")
    add_definitions(-DARCH_WINDOWS)
    add_definitions(-DARCH=\"Windows\")
    set(ARCH_WINDOWS true)
    add_compile_definitions(NOMINMAX)
    add_compile_definitions(_UNICODE)
    add_compile_definitions(UNICODE)
# https://stackoverflow.com/questions/21995777/cygwins-cmake-does-not-match-for-cmake-system-name
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "MSYS")
    message(STATUS "MSYS platform detected")
    add_definitions(-DARCH_UNIX)
    add_definitions(-DARCH_LINUX)
    add_definitions(-DARCH=\"Linux\")
    set(ARCH_UNIX true)
    set(ARCH_LINUX true)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    message(STATUS "Linux platform detected")
    add_definitions(-DARCH_UNIX)
    add_definitions(-DARCH_LINUX)
    add_definitions(-DARCH=\"Linux\")
    set(ARCH_UNIX true)
    set(ARCH_LINUX true)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    message(STATUS "macOS platform detected")
    add_definitions(-DARCH_UNIX)
    add_definitions(-DARCH_MACOS)
    add_definitions(-DARCH=\"macOS\")
    set(ARCH_UNIX true)
    set(ARCH_MACOS true)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    message(STATUS "BSD platform detected")
    add_definitions(-DARCH_UNIX)
    add_definitions(-DARCH_BSD)
    add_definitions(-DARCH=\"FreeBSD\")
    set(ARCH_UNIX true)
    set(ARCH_BSD true)
else()
    message(FATAL_ERROR "Unsupported platform detected: '${CMAKE_SYSTEM_NAME}' ${XXX}")
endif()

if (${CMAKE_SIZEOF_VOID_P} EQUAL "8")
  add_definitions(-DARCH_SIZE=64)
elseif (${CMAKE_SIZEOF_VOID_P} EQUAL "4")
  add_definitions(-DARCH_SIZE=32)
else()
endif()

add_definitions(-DARCH_COMPILER=\"${CMAKE_CXX_COMPILER_ID}\")
add_definitions(-DARCH_COMPILER_VERSION=\"${CMAKE_CXX_COMPILER_VERSION}\")



#if(MSVC)
#  add_definitions(-DARCH_WINDOWS)
#  add_definitions(-DARCH=\"Windows\")
#else()
#  add_compile_options(-std=c++17 -Wall -Wextra -pedantic)
#endif()

# add project name and version as set by cmake to macros as well
add_definitions(-DPROJECT_NAME=\"${PROJECT_NAME}\")
add_definitions(-DPROJECT_VERSION=\"${PROJECT_VERSION}\")

# Global C++ settings
#
# This is a bit complex, C++20 is the best candidate, but the features used are not supported everywhere (namely all but latest version of Windows gives trouble with the SDK), so for now Windows uses C++17 and everyone else C++20. 
#
# C++20 should be enabled in Windows as soon as possible, see https://devblogs.microsoft.com/cppblog/announcing-full-support-for-a-c-c-conformant-preprocessor-in-msvc/ for more details. 
#
# C++20 is also not supported by sonar cloud
if(ARCH_WINDOWS AND MSVC)
  add_compile_options(/W4)
  set(CMAKE_CXX_STANDARD 17)
    #set(CMAKE_CXX_STANDARD 20)
    #add_compile_options(/Zc:preprocessor)
elseif(ARCH_WINDOWS)
    set(CMAKE_CXX_STANDARD 17)
    add_compile_options(-Wall -Wextra -pedantic)
    add_compile_options(-Wno-gnu-zero-variadic-macro-arguments)
else()
    set(CMAKE_CXX_STANDARD 20)
    add_compile_options(-Wall -Wextra -pedantic)
    add_compile_options(-Wno-gnu-zero-variadic-macro-arguments)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
