cmake_minimum_required(VERSION 3.10)

project(daw_text_table_link_prj)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard whose features are requested.")

message("CMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}")
enable_testing()

include("${CMAKE_SOURCE_DIR}/dependent_projects/CMakeListsCompiler.txt")

include_directories("./include")
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(GLEAN_CACHE "${CMAKE_SOURCE_DIR}/.glean/debug")
    set(GLEAN_CACHE2 "${CMAKE_SOURCE_DIR}/.glean/release")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set(GLEAN_CACHE "${CMAKE_SOURCE_DIR}/.glean/release")
    set(GLEAN_CACHE2 "${CMAKE_SOURCE_DIR}/.glean/debug")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(GLEAN_CACHE "${CMAKE_SOURCE_DIR}/.glean/release")
    set(GLEAN_CACHE2 "${CMAKE_SOURCE_DIR}/.glean/debug")
endif ()

message("Building for: ${CMAKE_BUILD_TYPE}")
message("Checking for glean cache: '${GLEAN_CACHE}'")

option(GLEAN_INSTALL_ROOT "")
if (EXISTS "${GLEAN_INSTALL_ROOT}")
    message("using glean dependencies @ ${GLEAN_INSTALL_ROOT}")
    message("installing to ${GLEAN_INSTALL_ROOT}")
    add_custom_target(dependency_stub)
    include_directories(SYSTEM "${GLEAN_INSTALL_ROOT}/include")
    link_directories("${GLEAN_INSTALL_ROOT}/lib")
elseif (EXISTS "${GLEAN_CACHE}")
    message("using glean dependencies at ${GLEAN_CACHE}")
    add_custom_target(dependency_stub)
    include_directories(SYSTEM "${GLEAN_CACHE}/include")
    link_directories("${GLEAN_CACHE}/lib")
elseif (EXISTS "${GLEAN_CACHE2}")
    message("looks like glean is in use but missing ${GLEAN_CACHE}")
elseif (EXISTS "${CMAKE_SOURCE_DIR}/glean.cmake")
    message("glean.cmake detected")
    include("${CMAKE_SOURCE_DIR}/glean.cmake")
    add_library(dependency_stub STATIC "${CMAKE_SOURCE_DIR}/dependent_projects/stub.cpp")
    add_dependencies(dependency_stub ${DEP_PROJECT_DEPS})
    include_directories(SYSTEM "${CMAKE_BINARY_DIR}/install/include")
    link_directories("${CMAKE_BINARY_DIR}/install/lib")
endif ()

set(HEADER_FOLDER "include")
set(TEST_FOLDER "tests")
set(SOURCE_FOLDER "src")

include_directories(${HEADER_FOLDER})

set(HEADER_FILES
        ${HEADER_FOLDER}/daw/text_table/daw_text_table_link.h
        ${HEADER_FOLDER}/daw/text_table/daw_text_table_iterator.h
        ${HEADER_FOLDER}/daw/text_table/impl/daw_text_table_assert.h
        ${HEADER_FOLDER}/daw/text_table/impl/daw_text_table_link_common.h
        ${HEADER_FOLDER}/daw/text_table/impl/daw_text_table_link_parsers.h
        ${HEADER_FOLDER}/daw/text_table/impl/daw_csv_table.h
        ${HEADER_FOLDER}/daw/text_table/impl/daw_text_table_link_table_state.h
)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
add_custom_target(full)
add_dependencies(check full)

add_executable(daw_text_table_link_test_bin EXCLUDE_FROM_ALL ${HEADER_FILES} ${TEST_FOLDER}/daw_text_table_link_test.cpp)
target_link_libraries( daw_text_table_link_test_bin absl_strings absl_base absl_throw_delegate )
add_dependencies(daw_text_table_link_test_bin dependency_stub)
add_test(NAME daw_text_table_link_test COMMAND daw_text_table_link_test_bin)
add_dependencies(full daw_text_table_link_test_bin)

add_executable(worldcitiespop_test_bin EXCLUDE_FROM_ALL ${HEADER_FILES} ${TEST_FOLDER}/worldcitiespop_test.cpp)
target_link_libraries( worldcitiespop_test_bin absl_strings absl_base absl_throw_delegate )
add_dependencies(worldcitiespop_test_bin dependency_stub)
add_test(NAME worldcitiespop_test COMMAND worldcitiespop_test_bin)
add_dependencies(full worldcitiespop_test_bin)

install(DIRECTORY ${HEADER_FOLDER}/daw/text_table/ DESTINATION include/daw/text_table/)

