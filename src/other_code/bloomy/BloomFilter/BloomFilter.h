/*********************************************
 * Class: BloomFilter                        *
 *                                           *
 * A very simple BloomFilter.                *
 *                                           *
 * Author: Alex Towell (queelius@gmail.com)  *
 *********************************************/

#ifndef __BLOOM_FILTER_H__
#define __BLOOM_FILTER_H__

#include <vector>
#include <functional>
#include "Str.h"
#include <memory>
#include "HashStrGenerator.h"

namespace alex
{
namespace bloom
{

class BloomFilter
{
public:
    BloomFilter();

    BloomFilter(size_t size, const std::vector<std::shared_ptr<HashStrFn>>& hashes);

    bool member(const Str& x) const;

    void insert(const Str& x);
    
    size_t size() const;

    std::vector<std::shared_ptr<HashStrFn>> getHashes();

private:
    std::vector<std::shared_ptr<HashStrFn>> hashes;
    std::vector<bool> elements;
};

};
};

#endif