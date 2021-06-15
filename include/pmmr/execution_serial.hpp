#pragma once

#include <iostream>      // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <vector>        // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <unordered_map> // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <algorithm>     // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <thread>        // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <mutex>         // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!

#include <pmmr/execution_common.hpp>


namespace pmmr {


template <typename Cfg>
class IntermediateCoordinatorSerial
{
  public:
    using IntermediateKey     = typename Cfg::IntermediateKey;
    using IntermediateValue   = typename Cfg::IntermediateValue;
    using HashIntermediateKey = typename Cfg::HashIntermediateKey;

    using IntermediateKeyValBackend = std::unordered_map<IntermediateKey, std::vector<IntermediateValue>, HashIntermediateKey>;
    
    IntermediateCoordinatorSerial(IntermediateKeyValBackend & backend)
      : m_backend(backend)
    {
    }

    void emitIntermediate(IntermediateKey const & intermediateKey, IntermediateValue const & intermediateValue)
    {
      if (m_backend.find(intermediateKey) == m_backend.end())
      {
        m_backend[intermediateKey] = {intermediateValue};
      }
      else
      {
        m_backend[intermediateKey].push_back(intermediateValue);
      }
    };
        
  protected:
    IntermediateKeyValBackend & m_backend;
};

    
template <typename Cfg>
class OutputCoordinatorSerial
{
  public:
    using OutputKey     = typename Cfg::OutputKey;
    using OutputValue   = typename Cfg::OutputValue;
    using HashOutputKey = typename Cfg::HashOutputKey;

    using OutputPair = std::pair<OutputKey, OutputValue>;
       
    OutputCoordinatorSerial(std::vector<OutputPair> & outputPairs)
      : m_outputPairs(outputPairs)
    {
    }

    void emitFinal(OutputKey const & outputKey, OutputValue const & outputValue)
    {
      // Ensure that there are no duplicate output keys
      if (m_duplicateCheckHelper.find(outputKey) != m_duplicateCheckHelper.end())
      {
        throw std::runtime_error("Duplicate output key detected!");
      }
      m_duplicateCheckHelper[outputKey] = true;

      m_outputPairs.push_back({outputKey, outputValue});
    };
    
  protected:
    std::unordered_map<OutputKey, bool, HashOutputKey>   m_duplicateCheckHelper;
    std::vector<OutputPair>                            & m_outputPairs;
};


template <typename Cfg>
class MapExecutorSerial : public MapExecutorBase<Cfg, IntermediateCoordinatorSerial<Cfg>>
{
  public:
    using Base = MapExecutorBase<Cfg, IntermediateCoordinatorSerial<Cfg>>;
    using Base::MapExecutorBase;

    template <typename Mapper>
    void triggerMap(typename Base::InputKey const & inputKey, typename Base::InputValue const & inputValue)
    {
      // Just run the function in this thread
      Mapper mapper(this->m_intermediateCoordinator);
      mapper.map(inputKey, inputValue);
    }
};


template <typename Cfg>
class ReduceExecutorSerial : public ReduceExecutorBase<Cfg, OutputCoordinatorSerial<Cfg>>
{
  public:
    using Base = ReduceExecutorBase<Cfg, OutputCoordinatorSerial<Cfg>>;
    using Base::ReduceExecutorBase;

    template <typename Reducer>
    void triggerReduce(typename Base::IntermediateKey const & intermediateKey, std::vector<typename Base::IntermediateValue> const & intermediateValues)
    {
      // Just run the function in this thread
      Reducer reducer(this->m_outputCoordinator);
      reducer.reduce(intermediateKey, intermediateValues);
    }
};


template <typename Cfg>
struct ExecutionPolicySerial
{
  using IntermediateCoordinator = IntermediateCoordinatorSerial<Cfg>;
  using OutputCoordinator       = OutputCoordinatorSerial<Cfg>;
  using MapExecutor             = MapExecutorSerial<Cfg>;
  using ReduceExecutor          = ReduceExecutorSerial<Cfg>;
};


} // namespace pmmr
