#include "AutoTune.h"
#include <algorithm>
#include <random>

using namespace alex;
using namespace alex::bloom;

#define DEBUG

double alex::bloom::autotune::autoTuneNaive(BloomFilter& filter,
                     const std::set<Str>& memberData,
                     const std::vector<Str>& trainingData,
                     HashStrGenerator gen,
                     size_t bloomSize,
                     double fpProbability,
                     size_t maxTrials,
                     size_t minTrials)
{
    const size_t MIN_HASH = 1;
    const size_t MAX_HASH = 5;

#ifdef DEBUG
    std::cout << "---[ bloom::autotune::autoTuneNaive(...) ]---" << std::endl;
    std::cout << "\tfpProbability: " << fpProbability << std::endl;
    std::cout << "\tminTrials: " << minTrials << std::endl;
    std::cout << "\tmaxTrials: " << maxTrials << std::endl;
    std::cout << "\tMIN_HASH: " << MIN_HASH << std::endl;
    std::cout << "\tMAX_HASH: " << MAX_HASH << std::endl;
    std::cout << "\tbloomSize: " << bloomSize << std::endl;
    std::cout << "\tsize(memberData): " << memberData.size() << std::endl;
    std::cout << "\tsize(trainingData): " << trainingData.size() << std::endl;
#endif

    size_t minFailures = std::numeric_limits<size_t>::max();
    double minFp = 1;
    std::random_device rd;
    const auto hashIds = gen.getHashIds();

    for (size_t trial = 0; trial < maxTrials; ++trial)
    {
        std::vector<std::shared_ptr<HashStrFn>> hashes;
        size_t numHash = MIN_HASH + rd() % (MAX_HASH - MIN_HASH + 1);
        std::vector<bool> valid(hashIds.size(), true);
        for (size_t i = 0; i < numHash; ++i)
        {
            size_t index = rd() % hashIds.size();
            while (!valid[index])
            {
                index = rd() % hashIds.size();
            }
            auto h = gen.make(hashIds[index]);
            valid[hashIds[index]] = h->isRandom();
            hashes.push_back(h);
        }

        BloomFilter bloom(bloomSize, hashes);
        for (const auto& e : memberData)
        {
            bloom.insert(e);
        }

        size_t failures = 0;
        for (size_t i = 0; i < trainingData.size(); ++i)
        {
            const bool actual = bloom.member(trainingData.at(i));
            const bool expected = memberData.count(trainingData.at(i)) != 0;

            if (actual != expected)
            {
                ++failures;
            }
        }

        if (failures <= minFailures)
        {
            if (minFailures != std::numeric_limits<size_t>::max())
            {
                if (bloom.getHashes().size() > 1 && failures == minFailures
                        && bloom.getHashes().size() > filter.getHashes().size())
                {
                    continue;
                }
            }

            filter = bloom;
            minFailures = failures;
            minFp = failures / (double)trainingData.size();

#ifdef DEBUG
    std::cout << "\t\ttrial " << trial << ": bloom filter size = " << bloomSize << ", ";
    std::cout << "hashes = " << hashes.size() << ", ";
    std::cout << "fp probability = " << minFp << std::endl;
#endif

            if ((minFp == 0 && filter.getHashes().size() == 1)
                    || (minFp <= fpProbability && trial > minTrials))
            {
                return minFp;
            }
        }
    }

    return minFp;
}
