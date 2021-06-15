
#include <pmmr/pmmr.hpp>

#include "magic_enum.hpp"

#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
#include <thread>



// EXAMPLE: FINDING COMMON FRIENDS
//
// Inspired by: https://stackoverflow.com/a/12375878

enum class Person
{
  Alice, Bob, Chuck, Dan, Eve
};

using PersonPair = std::pair<Person, Person>;

using Friends = std::vector<Person>;


struct CommonFriendFinderConfig
{
  using InputKey          = Person;       // A specific person
  using InputValue        = Friends;      // The friends of this specific person
  using IntermediateKey   = PersonPair;   // A pair of persons
  using IntermediateValue = Friends;      // 
  using OutputKey         = PersonPair;   // 
  using OutputValue       = Friends;      // Common friends for the specified pair of persons   
};


// The types IntermediateKey and OutputKey need to be hashable, for use in std::unordered_map.
// The PersonPair type cannot be hashed by default. Manually define a hashing functor.
template <typename KeyValCfg>
class FriendFinderHashingPolicy
{
protected:
  struct HashPersonPair
  {
    std::size_t operator()(PersonPair const & personPair) const
    {
      // Compute individual hash values for individual values, then combine
      // http://stackoverflow.com/a/1646913/126995
      std::size_t result = 17;
      result = result * 31 + std::hash<int>()(static_cast<int>(personPair.first));
      result = result * 31 + std::hash<int>()(static_cast<int>(personPair.second));
      return result;
    }
  };

public:
  using HashIntermediateKey = HashPersonPair;
  using HashOutputKey = HashPersonPair;
};


class FriendsPrinter {
  public:
  
    static std::string printPerson(Person const & person)
    {
      // For easier readability, just use the first character of a name
      return std::string(magic_enum::enum_name(person)).substr(0, 1);
    }
  
  
    static std::string printPersonPair(PersonPair const & personPair)
    {
      return
        std::string("(") + printPerson(personPair.first) + " " + printPerson(personPair.second) + ")";
    }
  
  
    static std::string printFriends(Friends const & friends)
    {
      std::string friendsStr;
  
      for (auto it = friends.cbegin(); it != friends.cend(); ++it) {
        friendsStr += printPerson(*it);
  
        if (std::next(it) != friends.cend()) {
          friendsStr += " ";
        }
      }
  
      return friendsStr;
    }
  
  
    static std::string printFriendSets(std::vector<Friends> const & friendsSets)
    {
      std::string str;
  
      for (auto it = friendsSets.cbegin(); it != friendsSets.cend(); ++it) {
  
        str += "(" + printFriends(*it) + ")";
  
        if (std::next(it) != friendsSets.cend()) {
          str += " ";
        }
  
      }
    
      return str;
    }
};


class CommonFriendFinder : public pmmr::PMMR<CommonFriendFinder, CommonFriendFinderConfig, pmmr::ExecutionPolicySerial, FriendFinderHashingPolicy>
{
  public:

    class Mapper : public MapperBase
    {
      public:
        using MapperBase::MapperBase;

        void map(InputKey const & personRef, InputValue const & friends) override
        {
          // vvvvvvvvvvvvvvvvvvvv
          std::thread::id const thisThreadId  = std::this_thread::get_id();
          std::cout << "Running \"map\" for \"" << FriendsPrinter::printPerson(personRef) << " --> " << FriendsPrinter::printFriends(friends) << "\" in thread " << thisThreadId << "...\n";
          std::this_thread::sleep_for(std::chrono::milliseconds(500));
          // ^^^^^^^^^^^^^^^^^^^^

          for (auto const & personOther : friends) {
            IntermediateKey const personPairKey = normalizePersonPair({personRef, personOther});

            // vvvvvvvvvvvvvvvvvvvvvvvv
            std::cout << "Emitting: " << FriendsPrinter::printPersonPair(personPairKey) << " --> " << FriendsPrinter::printFriends(friends) << "\n";
            // ^^^^^^^^^^^^^^^^^^^^^^^^

            m_intermediateCoordinator.emitIntermediate(personPairKey, friends);
          }
        }

      protected:
        static PersonPair normalizePersonPair(PersonPair const & personPair)
        {
          // Friendships are bi-directional, i.e., (A, B) is equal to (B, A)
          // ==> normalize pairs to make processing easier
          return {
            std::min(personPair.first, personPair.second),
            std::max(personPair.first, personPair.second),
          };
        }
    };

    class Reducer : public ReducerBase
    {
      public:
        using ReducerBase::ReducerBase;
    
        void reduce(IntermediateKey const & personPair, std::vector<IntermediateValue> const & friendsSets) override
        {
          // vvvvvvvvvvvvvvvvvvvv
          std::thread::id const thisThreadId  = std::this_thread::get_id();
          std::cout << "Running \"reduce\" for pair \"" << FriendsPrinter::printPersonPair(personPair) << " --> " << FriendsPrinter::printFriendSets(friendsSets) << "\" in thread " << thisThreadId << "...\n";
          std::this_thread::sleep_for(std::chrono::milliseconds(500));
          // ^^^^^^^^^^^^^^^^^^^^

          if (friendsSets.size() > 2)
          {
            throw std::runtime_error("Unexpected: Found more than two friends sets!");
          }

          Friends commonFriends;

          // Determine intersection of friend sets (only makes sense if there are exactly two friends sets
          if (friendsSets.size() == 2)
          {
            std::unordered_map<Person, bool> isInSetA;

            for (auto const & person : friendsSets[0]) {
              isInSetA[person] = true;
            }

            for (auto const & person : friendsSets[1]) {
              if (isInSetA.find(person) != isInSetA.end()) {
                commonFriends.push_back(person);
              }
            }
          }

          m_outputCoordinator.emitFinal(personPair, commonFriends);
        }
    };
  
};



int main()
{
  // TODO: This code has a limitation:
  //       Common friends are not computed for two persons who are
  //       not friends themselves.


  Person const A = Person::Alice;
  Person const B = Person::Bob;
  Person const C = Person::Chuck;
  Person const D = Person::Dan;
  Person const E = Person::Eve;

  std::vector<CommonFriendFinder::InputPair> const inputPairs = {
    { A, {    B, C, D    } },
    { B, { A,    C, D, E } },
    { C, { A, B,    D, E } },
    { D, { A, B, C,    E } },
    { E, {    B, C, D    } }
  };

  std::vector<CommonFriendFinder::OutputPair> outputPairs;
  CommonFriendFinder commonFriendFinder;
  commonFriendFinder.processData(inputPairs, outputPairs);


  std::cout << "RESULTS:\n";

  for (auto const & outputPair : outputPairs) {
    PersonPair const & personPair    = outputPair.first;
    Friends    const & commonFriends = outputPair.second;

    std::cout << "Common friends of " << FriendsPrinter::printPersonPair(personPair) << " --> (" << FriendsPrinter::printFriends(commonFriends) << ")\n";
  }
}
