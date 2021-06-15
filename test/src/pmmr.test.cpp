#include <catch.hpp>

#include <taskman/taskman.hpp>
#include "test_sample_generator.hpp"


TEMPLATE_TEST_CASE( "Task managers contain no data after calling killAll()", "[taskman][killing]", taskman::TaskMan, taskman::TaskManFIFO, taskman::TaskManPrio ) {

    std::size_t const capacity       = 100;
    std::size_t const numTestSamples = 50;

    test::SampleDataGenerator sdg;
    auto const samples = sdg.generateSamples(numTestSamples);

    TestType tm(capacity);
   
    // Add test samples
    std::size_t numSamplesPrioLow  = 0;
    std::size_t numSamplesPrioMid  = 0;
    std::size_t numSamplesPrioHigh = 0;

    for (auto const & sample : samples) {
      
      // tm->add(sample.process);

      // For testing purposes, add processes with a custom creation time.
      // Otherwise we would not see much of a difference here (all processes
      // would have pretty much exactly the same time, as this loop finishes
      // very quickly)
      tm.addWithCreationTime(sample.process, sample.creationTime);

      if (sample.process.getPrio() == os::Process::Prio::low) {
        numSamplesPrioLow += 1;
      }
      if (sample.process.getPrio() == os::Process::Prio::mid) {
        numSamplesPrioMid += 1;
      }
      if (sample.process.getPrio() == os::Process::Prio::high) {
        numSamplesPrioHigh += 1;
      }
    }

    REQUIRE( tm.list(taskman::SortBy::id).size() == numTestSamples );
    REQUIRE( numTestSamples == numSamplesPrioLow + numSamplesPrioMid + numSamplesPrioHigh );


    SECTION( "killAll() removes all tracked processes" ) {

        tm.killAll();        

        REQUIRE( tm.list(taskman::SortBy::id).size() == 0 );
    }

    SECTION( "killPrio(os::Process:Prio::low) kills all low priority processes" ) {
    
        tm.killPrio(os::Process::Prio::low);    
    
        bool isNoRemainingProcessesPrioLow = true;
        for (auto const & elem : tm.list(taskman::SortBy::id)) {
          isNoRemainingProcessesPrioLow = isNoRemainingProcessesPrioLow && (elem.prio != os::Process::Prio::low);
        }
    
        REQUIRE( tm.list(taskman::SortBy::id).size() == numSamplesPrioMid + numSamplesPrioHigh );
        REQUIRE( isNoRemainingProcessesPrioLow );
    }
    
    SECTION( "killPrio(os::Process:Prio::mid) kills all mid priority processes" ) {
    
        tm.killPrio(os::Process::Prio::mid);    
    
        bool isNoRemainingProcessesPrioMid = true;
        for (auto const & elem : tm.list(taskman::SortBy::id)) {
          isNoRemainingProcessesPrioMid = isNoRemainingProcessesPrioMid && (elem.prio != os::Process::Prio::mid);
        }
    
        REQUIRE( tm.list(taskman::SortBy::id).size() == numSamplesPrioLow + numSamplesPrioHigh );
        REQUIRE( isNoRemainingProcessesPrioMid );
    }
    
    SECTION( "killPrio(os::Process:Prio::high) kills all high priority processes" ) {
    
        tm.killPrio(os::Process::Prio::high);    
    
        bool isNoRemainingProcessesPrioHigh = true;
        for (auto const & elem : tm.list(taskman::SortBy::id)) {
          isNoRemainingProcessesPrioHigh = isNoRemainingProcessesPrioHigh && (elem.prio != os::Process::Prio::high);
        }
    
        REQUIRE( tm.list(taskman::SortBy::id).size() == numSamplesPrioLow + numSamplesPrioMid );
        REQUIRE( isNoRemainingProcessesPrioHigh );
    }

}