/********************************************
 * bloom::autotune namespace                *
 *                                          *
 * The primary function inside this         *
 * namespace is:                            *
 *                                          *
 *     bloomFilterAutoTune                  *
 *                                          *
 * This will automatically find a set       *
 * of hash functions for a BloomFilter      *
 * on given member data and test data.      *
 *                                          *
 * This is a naive implementation as it     *
 * randomly tweaks the parameters of        *
 * a an assortment of hash functions and    *
 * chooses the set of hash functions which  *
 * generated the fewest false positives     *
 * on the test data.                        *
 *                                          *
 * Future implementations may use a         *
 * genetic algorithm or some other more     *
 * clever hillcimbing algoirthm.            *
 *                                          *
 * Author: Alex Towell (queelius@gmail.com) *
 ********************************************/

#ifndef __AUTO_TUNE_BLOOM_FILTER_H__
#define __AUTO_TUNE_BLOOM_FILTER_H__

#include <vector>
#include <set>
#include "Str.h"
#include "BloomFilter.h"
#include "HashStrGenerator.h"

namespace alex
{
namespace bloom
{
namespace autotune
{

double autoTuneNaive(BloomFilter& filter,
                     const std::set<Str>& memberData,
                     const std::vector<Str>& trainingData,
                     HashStrGenerator gen,
                     size_t bloomSize,
                     double fpProbability,
                     size_t maxTrials = 1e+5,
                     size_t minTrials = 0);

};
};
};

#endif