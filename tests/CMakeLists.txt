project(run-tests)

file(GLOB_RECURSE TESTS_HELPERS "../helpers/tests/*.h" "../helpers/tests/*.cpp")
file(GLOB_RECURSE LIBTPP_HELPERS "../libtpp/tests/*.h" "../libtpp/tests/*.cpp")

SET(COVERAGE_COMPILE_FLAGS "-g -O0 -coverage -fprofile-arcs -ftest-coverage")
SET(COVERAGE_LINK_FLAGS    "-coverage -lgcov")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${COVERAGE_COMPILE_FLAGS}" )
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${COVERAGE_LINK_FLAGS}" )

add_executable(run-tests "run-tests.cpp" ${TESTS_HELPERS} ${LIBTPP_HELPERS})
target_link_libraries(run-tests libtpp)

add_custom_target(coverage-tests 
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND GCOV_PREFIX=${CMAKE_BINARY_DIR}/coverage-tests GCOV_PREFIX_STRIP=8 ./run-tests
    COMMAND lcov -o lcov_app.info -c -d ${CMAKE_BINARY_DIR}/coverage-tests
    DEPENDS run-tests
)