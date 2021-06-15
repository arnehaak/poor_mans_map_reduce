#pragma once

#include <taskman/taskman.hpp>

#include <ctime>
#include <cstddef>
#include <random>
#include <vector>
#include <algorithm>
#include <unordered_map>


namespace test {
  

class SampleDataGenerator
{
  public:

    struct Sample {
      os::Process process;
      std::time_t creationTime;
    };

    std::vector<Sample> generateSamples(std::size_t numSamples) {
      std::vector<Sample> samples;
  
      for (std::size_t idx = 0; idx < numSamples; ++idx) {
        Sample sample{os::Process(randomPID(), randomPrio()), randomTimeOfDay()};
        samples.push_back(sample);
      }

      // Sort by creation time
      auto comparator = SampleComparatorCreationTimeLT();
      std::sort(samples.begin(), samples.end(), comparator);

      return samples;
    }
    

  protected:
    // Random engine with a fixed seed.
    // At least on the same OS with the same compiler version outputs should be identical.
    // The C++ standard makes no guarantess about the underlying uniform_int_distribution.
    std::mt19937 m_randomEngine{42};

    // Remember all generated PIDs    
    std::unordered_map<os::Process::PID, bool> m_knownPids;


    // Comparator to sort processes by creation time
    struct SampleComparatorCreationTimeLT {
      bool operator ()(Sample const & lhs, Sample const & rhs) {
        return lhs.creationTime < rhs.creationTime;
      }
    };


    template <typename IntType>
    IntType generateRandomInt(IntType valMin, IntType valMax) {
      // Both iMin and iMax (inclusive) are part of the inverval of potential random values!
      std::uniform_int_distribution<uint32_t> randomDistUniInt(valMin, valMax);

      return randomDistUniInt(m_randomEngine);
    }


    std::time_t randomTimeOfDay(std::time_t day = 1595152800) {
      // Generate a random time on the day specified by "day".
      // The default value 1595152800 is 2020-07-19.
      std::tm time = *(std::gmtime(&day));
      time.tm_hour = generateRandomInt<int>(0, 23);
      time.tm_min  = generateRandomInt<int>(0, 59);
      time.tm_sec  = generateRandomInt<int>(0, 59);
      return std::mktime(&time);
    }


    os::Process::PID randomPID() {
      // Ensure that the same PID is not produced twice

      os::Process::PID const pidMin = 1;
      os::Process::PID const pidMax = 65535;

      os::Process::PID pid = generateRandomInt<os::Process::PID>(pidMin, pidMax);

      // Generate until we find an unused PID
      while (m_knownPids.find(pid) != m_knownPids.end()) {
        pid = generateRandomInt<os::Process::PID>(pidMin, pidMax);
      }

      return pid;
    }


    os::Process::Prio randomPrio() {
      int const randomPrio = generateRandomInt<int>(1, 3);

      os::Process::Prio prio = os::Process::Prio::low;
      
      if (randomPrio == 2) {
        prio = os::Process::Prio::mid;
      } else if (randomPrio == 3) {
        prio = os::Process::Prio::high;
      }

      return prio;
    }

};


} // namespace test
