cmake_minimum_required(VERSION 3.13...3.16 FATAL_ERROR)

project(pmmr VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)


# Create library target
# (adding headers here only so that they show up in the IDE)
add_library(libpmmr
    include/pmmr/pmmr.hpp
    include/pmmr/pmmr_main_class.hpp
    include/pmmr/execution_common.hpp
    include/pmmr/execution_serial.hpp
    include/pmmr/execution_parallel.hpp
    include/pmmr/hash_policy_automatic.hpp
    src/pmmr_common.cpp
)



#Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(PMMR::libpmmr ALIAS libpmmr)

#Set target properties
target_include_directories(libpmmr
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

find_package(Threads REQUIRED)

target_link_libraries(libpmmr
    PUBLIC
        Threads::Threads
)

target_compile_options(libpmmr PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Wall -Wextra -Wpedantic>)



# Test
enable_testing()
add_subdirectory(test)

# Usage examples
add_subdirectory(example_wordcount)
add_subdirectory(example_commonfriends)
