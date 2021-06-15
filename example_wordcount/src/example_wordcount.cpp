
#include <pmmr/pmmr.hpp>

#include <string>
#include <vector>
#include <utility>
#include <regex>
#include <cctype>
#include <unordered_map>
#include <thread>


struct WordCountConfig
{
  using InputKey          = std::string;    // Document name (not used)
  using InputValue        = std::string;    // Document content
  using IntermediateKey   = std::string;    // Word
  using IntermediateValue = int;            // Count of a word within a certain document
  using OutputKey         = std::string;    // Word
  using OutputValue       = int;            // Total count of a word
};


class WordCount : public pmmr::PMMR<WordCount, WordCountConfig, pmmr::ExecutionPolicySerial>
{
  public:

    class Mapper : public MapperBase
    {
      public:
        using MapperBase::MapperBase;

        void map(InputKey const & docName, InputValue const & docContent) override
        {
          // vvvvvvvvvvvvvvvvvvvv
          std::thread::id const thisThreadId  = std::this_thread::get_id();
          std::cout << "Running \"map\" on \"" << docContent << "\" in thread " << thisThreadId << "...\n";
          std::this_thread::sleep_for(std::chrono::milliseconds(500));
          // ^^^^^^^^^^^^^^^^^^^^

          auto const words = getNormalizedWords(docContent);
          
          std::unordered_map<std::string, int> wordCounts;
          for (auto const word : words) {          
            if (wordCounts.find(word) == wordCounts.end()) {
              wordCounts[word] = 1;
            } else {
              wordCounts[word] += 1;
            }
          }

          for (auto const & wordCount : wordCounts) {
            std::string const & word  = wordCount.first;
            int                 count = wordCount.second;

            m_intermediateCoordinator.emitIntermediate(word, count);
          }
                  
        }

      protected:
        static std::vector<std::string> getNormalizedWords(std::string const & input)
        {
          std::vector<std::string> output;

          std::regex splitRegex("[\\s\\.,!]+"); 

          // default constructor = end-of-sequence:
          std::regex_token_iterator<std::string::const_iterator> const rend;

          std::regex_token_iterator<std::string::const_iterator> it(input.cbegin(), input.cend(), splitRegex, -1);
          while (it != rend) {

            std::string currWord = *it++;

            // Convert to lower case
            std::transform(currWord.begin(), currWord.end(), currWord.begin(),
              [](unsigned char c){ return std::tolower(c); });

            output.push_back(currWord);
          }

          return output;
        }
    };

    class Reducer : public ReducerBase
    {
      public:
        using ReducerBase::ReducerBase;
    
        void reduce(IntermediateKey const & word, std::vector<IntermediateValue> const & partialCounts) override
        {
          // vvvvvvvvvvvvvvvvvvvv
          std::thread::id const thisThreadId  = std::this_thread::get_id();
          std::cout << "Running \"reduce\" for word \"" << word << "\" in thread " << thisThreadId << "...\n";
          std::this_thread::sleep_for(std::chrono::milliseconds(500));
          // ^^^^^^^^^^^^^^^^^^^^

          IntermediateValue totalCount = 0;
          for (IntermediateValue partialCount : partialCounts) {
            totalCount += partialCount;
          }
          
          m_outputCoordinator.emitFinal(word, totalCount);
        }
    };
  

};


int main()
{
  std::vector<WordCount::InputPair> const inputPairs = {
    { "input_1", "One fish, Two fish," },
    { "input_2", "Red Fish, Blue fish." },
    { "input_3", "Even more fish!" },
  };

  std::vector<WordCount::OutputPair> outputPairs;
  WordCount wordCounter;
  wordCounter.processData(inputPairs, outputPairs);

  for (auto const & outputPair : outputPairs) {
    std::cout << outputPair.first << " " << outputPair.second << "\n";
  }
}
