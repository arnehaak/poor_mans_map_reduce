
# Catch2 test framework
add_library(catch2 INTERFACE)
target_include_directories(catch2 INTERFACE 3rdparty/catch2)

# Test executable
add_executable(libpmmr_test
    ../include/pmmr/pmmr.hpp
    src/main.cpp
    src/pmmr.test.cpp
    src/policy.test.cpp
)
    
target_link_libraries(libpmmr_test catch2 PMMR::libpmmr)

add_test(NAME libpmmr_test COMMAND libpmmr_test)
