#pragma once

#include <iostream>      // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <vector>        // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <unordered_map> // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <algorithm>     // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <thread>        // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <mutex>         // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!


namespace pmmr {


template <typename Cfg, typename IntermediateCoordinatorClass>
class MapExecutorBase
{
  public:
    using InputKey   = typename Cfg::InputKey;
    using InputValue = typename Cfg::InputValue;

    using IntermediateCoordinator = IntermediateCoordinatorClass;

    MapExecutorBase(IntermediateCoordinator & intermediateCoordinator)
      : m_intermediateCoordinator(intermediateCoordinator)
    {
    }

  protected:
    IntermediateCoordinator & m_intermediateCoordinator;
};


template <typename Cfg, typename OutputCoordinatorClass>
class ReduceExecutorBase
{
  public:
    using IntermediateKey   = typename Cfg::IntermediateKey;
    using IntermediateValue = typename Cfg::IntermediateValue;

    using OutputCoordinator = OutputCoordinatorClass;

    ReduceExecutorBase(OutputCoordinator & outputCoordinator)
      : m_outputCoordinator(outputCoordinator)
    {
    }

    protected:
      OutputCoordinator & m_outputCoordinator;
};


} // namespace pmmr
