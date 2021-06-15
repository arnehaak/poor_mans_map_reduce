#pragma once

#include <iostream>      // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <vector>        // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <unordered_map> // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <algorithm>     // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <thread>        // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <mutex>         // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!


namespace pmmr {


template <
  typename Derived,
  typename KeyValCfg,
  template<typename Cfg>       typename ExecutionPolicyClass = ExecutionPolicyParallel,
  template<typename KeyValCfg> typename HashingPolicyClass   = HashingPolicyAutomatic
>
class PMMR
{
  public:

    // Enhance configuration structure with hashing policies
    struct Cfg : public KeyValCfg
    {
      using HashIntermediateKey = typename HashingPolicyClass<KeyValCfg>::HashIntermediateKey;
      using HashOutputKey       = typename HashingPolicyClass<KeyValCfg>::HashOutputKey;
    };

    using ExecutionPolicy = ExecutionPolicyClass<Cfg>;

    using InputKey          = typename Cfg::InputKey;
    using InputValue        = typename Cfg::InputValue;
    using IntermediateKey   = typename Cfg::IntermediateKey;
    using IntermediateValue = typename Cfg::IntermediateValue;
    using OutputKey         = typename Cfg::OutputKey;
    using OutputValue       = typename Cfg::OutputValue;

    using InputPair  = std::pair<InputKey, InputValue>;
    using OutputPair = std::pair<OutputKey, OutputValue>;

    using IntermediateCoordinator = typename ExecutionPolicy::IntermediateCoordinator;
    using OutputCoordinator       = typename ExecutionPolicy::OutputCoordinator;
    using MapExecutor             = typename ExecutionPolicy::MapExecutor;
    using ReduceExecutor          = typename ExecutionPolicy::ReduceExecutor;


    class MapperBase
    {
      public:
        MapperBase(IntermediateCoordinator & intermediateCoordinator)
          : m_intermediateCoordinator(intermediateCoordinator)
        {
        }

        virtual void map(InputKey const & inputKey, InputValue const & inputValue) = 0;

      protected:
        IntermediateCoordinator & m_intermediateCoordinator;
    };


    class ReducerBase
    {
      public:
        ReducerBase(OutputCoordinator & outputCoordinator)
          : m_outputCoordinator(outputCoordinator)
        {
        }

        virtual void reduce(IntermediateKey const & intermediateKey, std::vector<IntermediateValue> const & intermediateValues) = 0;

      protected:
        OutputCoordinator & m_outputCoordinator;
    };


    void processData(std::vector<InputPair> const & inputPairs, std::vector<OutputPair> & outputPairs)
    {
      // Map phase 
      typename IntermediateCoordinator::IntermediateKeyValBackend intermediateKeyValBackend;
      {
        IntermediateCoordinator intermediateCoordinator(intermediateKeyValBackend);
        auto mapExecutor = MapExecutor(intermediateCoordinator);
      
        for (InputPair const & inputPair : inputPairs) {
          InputKey   const & currInputKey   = inputPair.first;
          InputValue const & currInputValue = inputPair.second;
      
          mapExecutor.triggerMap<typename Derived::Mapper>(currInputKey, currInputValue);
        }
      }
      std::cout << "Finished MAP phase...\n";

      // Shuffle phase
      // (nothing to do here, as this is all running on the same machine)
      
      // Reduce phase 
      {
        OutputCoordinator outputCoordinator(outputPairs);
        auto reduceExecutor = ReduceExecutor(outputCoordinator);
      
        auto const & intermediateBackend = intermediateKeyValBackend;
        for (auto it = intermediateBackend.cbegin(); it != intermediateBackend.cend(); ++it) {
          IntermediateKey                const & currIntermediateKey    = it->first;
          std::vector<IntermediateValue> const & currIntermediateValues = it->second;
      
          reduceExecutor.triggerReduce<typename Derived::Reducer>(currIntermediateKey, currIntermediateValues);
        }
      }
      std::cout << "Finished REDUCE phase...\n";
    }
};


} // namespace pmmr
