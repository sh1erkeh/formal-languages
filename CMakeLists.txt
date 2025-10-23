cmake_minimum_required(VERSION 3.15)
project(RegexParser VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

include_directories(${CMAKE_SOURCE_DIR}/src)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(regex-parser 
    src/regex-parser.cpp 
    src/lexer.cpp 
    src/nfa.cpp 
    src/postfix-to-suffix.cpp
)

add_executable(regex-tests 
    tests/test_main.cpp
    tests/test_lexer.cpp
    tests/test_nfa.cpp
    tests/test_converter.cpp
    src/lexer.cpp 
    src/nfa.cpp 
    src/postfix-to-suffix.cpp
)

target_link_libraries(regex-tests gtest gtest_main)

enable_testing()
add_test(NAME RegexTests COMMAND regex-tests)

add_custom_target(coverage
    COMMAND ./regex-tests
    COMMAND gcov -r src/lexer.cpp src/nfa.cpp src/postfix-to-suffix.cpp
    DEPENDS regex-tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running tests and generating coverage report"
)

add_custom_target(coverage-html
    COMMAND lcov --capture --directory . --output-file coverage.info
    COMMAND genhtml coverage.info --output-directory coverage-html
    DEPENDS coverage
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating HTML coverage report"
)

find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_SOURCE_FILES
        "*.cpp"
        "*.hpp"
        "*.h"
    )
    
    add_custom_target(
        format
        COMMAND ${CLANG_FORMAT}
        -i
        -style=file
        ${ALL_SOURCE_FILES}
        COMMENT "Running clang-format"
    )
endif()
