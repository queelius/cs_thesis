/*********************************************
 * Class: BloomFilter                        *
 *                                           *
 * A very simple BloomFilter.                *
 *                                           *
 * Author: Alex Towell (queelius@gmail.com)  *
 *********************************************/

#include <vector>
#include <functional>
#include <algorithm>
#include "HashStrGenerator.h"
#include "BloomFilter.h"
#include "Str.h"
#include <memory>

using namespace alex;
using namespace alex::bloom;

BloomFilter::BloomFilter() {}

BloomFilter::BloomFilter(size_t size,
                         const std::vector<std::shared_ptr<HashStrFn>>& hashes) : hashes(hashes),
    elements(size)
{
    // do nothing else
}

size_t BloomFilter::size() const
{
    return elements.size();
}

bool BloomFilter::member(const Str& x) const
{
    for (size_t i = 0; i < hashes.size(); ++i)
    {
        if (!elements.at(hashes[i]->get(x) % elements.size()))
        {
            return false;
        }
    }

    return true;
}

void BloomFilter::insert(const Str& x)
{
    for (size_t i = 0; i < hashes.size(); ++i)
    {
        elements[hashes[i]->get(x) % elements.size()] = true;
    }
}

std::vector<std::shared_ptr<HashStrFn>> BloomFilter::getHashes()
{
    return hashes;
}
