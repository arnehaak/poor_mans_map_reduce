#pragma once

#include <iostream>      // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <vector>        // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <unordered_map> // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <algorithm>     // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <thread>        // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!
#include <mutex>         // TODO: CHECK IF THIS HEADER IS REALLY REQUIRED HERE!


namespace pmmr {


// Automatically determine a hashing class based on type, whenever possible.
// Might require overriding if this is not possible for certain custom types.
template <typename KeyValCfg>
class HashingPolicyAutomatic
{
  protected:
    // Simple "hash" for enum class enums
    struct EnumClassHash
    {
      template <typename EnumClass>
      std::size_t operator()(EnumClass const & enumVal) const
      {
        return static_cast<std::size_t>(enumVal);
      }
    };
  
    template <typename KeyType>
    using AutoHash = typename std::conditional<std::is_enum<KeyType>::value, EnumClassHash, std::hash<KeyType>>::type;

  public:
    using HashIntermediateKey = AutoHash<typename KeyValCfg::IntermediateKey>;
    using HashOutputKey       = AutoHash<typename KeyValCfg::OutputKey>;
};


} // namespace pmmr
