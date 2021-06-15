
# Example executable
add_executable(example_commonfriends
    ../include/pmmr/pmmr.hpp
    src/magic_enum.hpp
    src/example_commonfriends.cpp
)
    
target_link_libraries(example_commonfriends PMMR::libpmmr)
