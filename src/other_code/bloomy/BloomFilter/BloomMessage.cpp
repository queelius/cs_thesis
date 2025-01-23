#include <algorithm>
#include "BloomMessage.h"
#include "AutoTune.h"

using namespace alex::bloom;
using namespace alex;

//#define DEBUG

BloomMessage::BloomMessage(const BloomFilter& filter,
                           size_t minGram,
                           size_t maxGram,
                           const std::vector<Str>& stopWords,
                           bool stemmed,
                           bool ignoreOrder) :
    _minGram(minGram),
    _maxGram(maxGram),
    _filter(filter),
    _stemmed(stemmed),
    _stopWords(stopWords),
    _ignoreOrder(ignoreOrder)
{
}

bool BloomMessage::hasExactMatch(const Str& query) const
{
    return _filter.member(query);
}

bool BloomMessage::hasMatch(Str query) const
{
    // make ignore word list expression
    Str removeStopWordsRx;
    for (size_t i = 0; i < _stopWords.size(); ++i)
    {
        if (i > 0)
        {
            removeStopWordsRx += "|";
        }
        removeStopWordsRx += _stopWords[i];
    }
    if (!removeStopWordsRx.empty())
    {
        removeStopWordsRx = Str("\\b(" + removeStopWordsRx.data() + ")\\b");
    }
    query = query.lower().replace(removeStopWordsRx, "");

    auto terms = alex::utils::terms(query);
    if (_stemmed)
    {
        for (auto& term : terms)
        {
            term = alex::utils::stemmer(term);           
        }
    }

    for (auto& term : terms)
    {
        if (_ignoreOrder)
            std::sort(term.begin(), term.end());

        Str q = term[0];
        for (size_t i = 1; i < term.size(); ++i)
            q = q + " " + term[i];

        if (!_filter.member(q))
            return false;
    }
    return true;
}

/**************************************
 * make constructors for BloomMessage *
 **************************************/

BloomMessage bloom::tune(const std::vector<Str>& memberData,
                         const std::vector<Str>& trainingData,
                         double fpProbability,
                         const std::vector<Str>& stopWords,
                         bool ignoreOrder,
                         bool stemming,
                         size_t minTrials,
                         size_t maxTrials)
{
    if (fpProbability < 0 || fpProbability > 1)
    {
        throw std::exception("False Positive Probability Must Be In Range [0, 1]");
    }

    if (minTrials > maxTrials)
    {
        std::swap(minTrials, maxTrials);
    }

    size_t minGram = std::numeric_limits<size_t>::max();
    size_t maxGram = 0;
    for (const auto& m : memberData)
    {
        const size_t sz = 1 + m.find("\\s").size();
        minGram = std::min(sz, minGram);
        maxGram = std::max(sz, maxGram);
    }

    for (const auto& t : trainingData)
    {
        const size_t sz = 1 + t.find("\\s").size();
        if (sz < minGram || sz > maxGram)
        {
            throw std::exception("Invalid Training Data");
        }
    }

#ifdef DEBUG
    std::cout << "---[ bloom::tune(...) ]---" << std::endl;
    std::cout << "\tstemming: " << stemming << std::endl;
    std::cout << "\tignoreOrder: " << ignoreOrder << std::endl;
    std::cout << "\tfpProbability: " << fpProbability << std::endl;
    std::cout << "\tminTrials: " << minTrials << std::endl;
    std::cout << "\tmaxTrials: " << maxTrials << std::endl;
    std::cout << "\tminGram: " << minGram << std::endl;
    std::cout << "\tmaxGram: " << maxGram << std::endl;
#endif

    const std::set<Str> memberDataSet(memberData.begin(), memberData.end());
    HashStrGenerator gen;
    size_t bloomSize = memberData.size();
    double error;
    BloomFilter filter;
    do
    {
        error = bloom::autotune::autoTuneNaive(filter,
                                               memberDataSet,
                                               trainingData,
                                               gen,
                                               bloomSize,
                                               fpProbability,
                                               minTrials,
                                               maxTrials);
        bloomSize *= 2;
    }
    while (error > fpProbability);

    return BloomMessage(filter, minGram, maxGram, stopWords, stemming,
                        ignoreOrder);
}
