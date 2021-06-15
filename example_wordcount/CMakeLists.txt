
# Example executable
add_executable(example_wordcount
    ../include/pmmr/pmmr.hpp
    src/example_wordcount.cpp
)
    
target_link_libraries(example_wordcount PMMR::libpmmr)
