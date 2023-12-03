cmake_minimum_required(VERSION 3.22.1)

project(
    Cryptography
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

find_program(CLANG_TIDY_EXE NAMES clang-tidy)

if(NOT CLANG_TIDY_EXE)
    message(WARNING "clang-tidy not found")
else()
    execute_process(
        COMMAND ${CLANG_TIDY_EXE} --version
        OUTPUT_VARIABLE CLANG_TIDY_VERSION)
    message("clang-tidy found:\n" ${CLANG_TIDY_VERSION})
endif()

find_package(Git QUIET)

if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)

    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_SUBMOD_RESULT)

        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

add_subdirectory(src)
