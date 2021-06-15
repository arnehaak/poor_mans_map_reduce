#pragma once

#include <iostream>      // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <vector>        // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <unordered_map> // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <algorithm>     // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <thread>        // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <mutex>         // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!

#include <pmmr/execution_serial.hpp>


namespace pmmr {


template <typename Cfg>
class IntermediateCoordinatorParallel : public IntermediateCoordinatorSerial<Cfg>
{
  public:
    using IntermediateKey   = typename Cfg::IntermediateKey;
    using IntermediateValue = typename Cfg::IntermediateValue;

    using IntermediateCoordinatorSerial<Cfg>::IntermediateCoordinatorSerial;

    void emitIntermediate(IntermediateKey const & intermediateKey, IntermediateValue const & intermediateValue)
    {
      std::lock_guard<std::mutex> const lock(m_mutex);
      IntermediateCoordinatorSerial<Cfg>::emitIntermediate(intermediateKey, intermediateValue);
    }    
  
  protected:
    std::mutex m_mutex;
};


template <typename Cfg>
class OutputCoordinatorParallel : public OutputCoordinatorSerial<Cfg>
{
  public:
    using OutputKey   = typename Cfg::OutputKey;
    using OutputValue = typename Cfg::OutputValue;

    using OutputCoordinatorSerial<Cfg>::OutputCoordinatorSerial;

    void emitFinal(OutputKey const & outputKey, OutputValue const & outputValue)
    {
      std::lock_guard<std::mutex> const lock(m_mutex);
      OutputCoordinatorSerial<Cfg>::emitFinal(outputKey, outputValue);
    }    
  
  protected:
    std::mutex m_mutex;
};


template <typename Cfg>
class MapExecutorParallel : public MapExecutorBase<Cfg, IntermediateCoordinatorParallel<Cfg>>
{
  public:
    using Base = MapExecutorBase<Cfg, IntermediateCoordinatorParallel<Cfg>>;
    using Base::MapExecutorBase;

    ~MapExecutorParallel()
    {
      std::cout << "Waiting for MAP worker threads to finish...\n";
      for (auto it = m_threads.begin(); it != m_threads.end(); ++it) {
        it->join();
      }
    }

    template <typename Mapper>
    void triggerMap(typename Base::InputKey const & inputKey, typename Base::InputValue const & inputValue)
    {
      // Run in a newly created thread
      m_threads.push_back(std::thread([&]() {
        Mapper mapper(this->m_intermediateCoordinator);
        mapper.map(inputKey, inputValue);
      }));
    }

  protected:
    std::vector<std::thread> m_threads;
};


template <typename Cfg>
class ReduceExecutorParallel : public ReduceExecutorBase<Cfg, OutputCoordinatorParallel<Cfg>>
{
  public:
    using Base = ReduceExecutorBase<Cfg, OutputCoordinatorParallel<Cfg>>;
    using Base::ReduceExecutorBase;

    ~ReduceExecutorParallel()
    {
      std::cout << "Waiting for REDUCE worker threads to finish...\n";
      for (auto it = m_threads.begin(); it != m_threads.end(); ++it) {
        it->join();
      }
    }

    template <typename Reducer>
    void triggerReduce(typename Base::IntermediateKey const & intermediateKey, std::vector<typename Base::IntermediateValue> const & intermediateValues)
    {
      // Run in a newly created thread
      m_threads.push_back(std::thread([&]() {
        Reducer reducer(this->m_outputCoordinator);
        reducer.reduce(intermediateKey, intermediateValues);
      }));
    }

  protected:
    std::vector<std::thread> m_threads;
};


template <typename Cfg>
struct ExecutionPolicyParallel
{
  using IntermediateCoordinator = IntermediateCoordinatorParallel<Cfg>;
  using OutputCoordinator       = OutputCoordinatorParallel<Cfg>;
  using MapExecutor             = MapExecutorParallel<Cfg>;
  using ReduceExecutor          = ReduceExecutorParallel<Cfg>;
};


} // namespace pmmr
