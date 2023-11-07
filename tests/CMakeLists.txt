project(run-tests)

file(GLOB_RECURSE TESTS_HELPERS "../helpers/tests/*.h" "../helpers/tests/*.cpp")
file(GLOB_RECURSE LIBTPP_HELPERS "../libtpp/tests/*.h" "../libtpp/tests/*.cpp")

add_executable(run-tests "run-tests.cpp" ${TESTS_HELPERS} ${LIBTPP_HELPERS})
target_link_libraries(run-tests libtpp)