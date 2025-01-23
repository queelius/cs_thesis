/*******************************************
 * Class: BloomMessage                      *
 *                                          *
 * Immutable class. This class uses a bloom *
 * filter to test whether a substring       *
 * belongs to a message.                    *
 *                                          *
 * Author: Alex Towell (queelius@gmail.com) *
 ********************************************/

#ifndef __BLOOM_MESSAGE_H__
#define __BLOOM_MESSAGE_H__

#include <vector>
#include <set>
#include <functional>
#include "Str.h"
#include "BloomFilter.h"
#include "Utils.h"

namespace alex
{
namespace bloom
{
class BloomMessage;

const size_t DEFAULT_MIN_TRIALS = 250;
const size_t DEFAULT_MAX_TRIALS = 500;

BloomMessage tune(const std::vector<Str>& memberData,
                  const std::vector<Str>& trainingData,
                  double fpProbability,
                  const std::vector<Str>& stopWords = DEFAULT_STOP_WORDS,
                  bool ignoreOrder = true,
                  bool stemming = true,
                  size_t minTrials = DEFAULT_MIN_TRIALS,
                  size_t maxTrials = DEFAULT_MAX_TRIALS);

class BloomMessage
{
public:
    BloomMessage(const BloomFilter& filter,
                 size_t minGram,
                 size_t maxGram,
                 const std::vector<Str>& stopWords,
                 bool stemmed = true,
                 bool ignoreOrder = true);

    bool hasMatch(Str query) const;
    bool hasExactMatch(const Str& query) const;

    BloomFilter getBloomFilter() const { return _filter; };
    bool stemmed() const { return _stemmed; };
    bool ignoreOrder() const { return _ignoreOrder; };
    std::vector<Str> stopWords() const { return _stopWords; };
    size_t minGram() const { return _minGram; };
    size_t maxGram() const { return _maxGram; };

private:
    const BloomFilter _filter;
    const std::vector<Str> _stopWords;
    const bool _stemmed;
    const bool _ignoreOrder;
    const size_t _minGram;
    const size_t _maxGram;
};

};
};

#endif