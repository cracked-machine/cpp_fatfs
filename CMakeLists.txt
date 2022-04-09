# This file is for running X86 unit tests only.
cmake_minimum_required(VERSION 3.13)

project(FATFS_TEST_SUITE VERSION 0.1 DESCRIPTION "FATFS_TEST_SUITE")

# if this is the main project then configure the build for test_suite only
if(CMAKE_PROJECT_NAME STREQUAL FATFS_TEST_SUITE)

    # generate the header file to define USED_API macro for unit test only
    set(USED_API "__attribute__((__used__))")
    configure_file(cmake/gnuc_ext_defs.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/include/gnuc_ext_defs.hpp @ONLY)
    include_directories(build/include)

    # include the mock library with all unit test build source files
    add_definitions(-include ${CMAKE_BINARY_DIR}/embedded_utils/tests/mocks/mock.hpp) 

    # x86 build config
    include(cmake/linux.cmake)
    
    # BUILD_NAME is set in cmake/linux.cmake
    add_executable(${BUILD_NAME} "")
    target_compile_features(${BUILD_NAME} PUBLIC cxx_std_20)

    # add external deps from git repos
    include(cmake/external.cmake)
    # we only need to include this for unit testing
    target_include_directories(${BUILD_NAME} PRIVATE 
        ${CMAKE_BINARY_DIR}/embedded_utils/tests/mocks
    )    

    # enable static analysis
    include(cmake/cppcheck.cmake)

    # add catch2 test lib and unit test
    find_package(Catch2 3 REQUIRED)
    target_link_libraries(${BUILD_NAME} PRIVATE Catch2::Catch2WithMain)
    add_subdirectory(tests)

endif()

# if this is submodule in another project then just build as standalone
add_subdirectory(src)


