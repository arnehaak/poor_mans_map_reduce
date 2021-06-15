#include <catch.hpp>

#include <taskman/taskman.hpp>
#include "test_sample_generator.hpp"


TEST_CASE( "PolicyBlocking works according to specification", "[policy]" ) {

    std::size_t const numTestSamples = 1000;

    test::SampleDataGenerator sdg;
    auto const samples = sdg.generateSamples(numTestSamples);

    taskman::PolicyBlocking policy;

    SECTION( "PolicyBlocking::getPidToKill() always returns std::nullopt" ) {
         
        bool isAlwaysNullOpt = true;
        
        for (auto const & sample : samples) {
          auto const currPidToKill = policy.getPidToKill(sample.process);
          isAlwaysNullOpt = isAlwaysNullOpt && (!currPidToKill.has_value());
        }

        REQUIRE( isAlwaysNullOpt );
    }

}
