
#include "HashStrGenerator.h"
#include <algorithm>
#include <sstream>
#include <random>
using namespace alex;

/****************************
 * Hash string generator    *
 ****************************/
HashStrGenerator::HashStrGenerator() {}

std::vector<size_t> HashStrGenerator::getHashIds() const
{
    return std::vector<size_t>{ 0, 1, 2, 3, 4 };
}

std::shared_ptr<HashStrFn> HashStrGenerator::make(size_t id)
{
    switch (id)
    {
    case 0:
        return std::shared_ptr<HashStrFn>(new HashStrFn1());
    case 1:
        return std::shared_ptr<HashStrFn>(new HashStrFn2());
    case 2:
        return std::shared_ptr<HashStrFn>(new HashStrFn3());
    case 3:
        return std::shared_ptr<HashStrFn>(new HashStrFn4());
    case 4:
        return std::shared_ptr<HashStrFn>(new HashStrFn5());
    default:
        throw std::exception("Invalid Argument");
    }
}

/******************
 * hash functions *
 ******************/
HashStrFn1::HashStrFn1() : coeff({ 1, 31, 137, 1571, 11047, 77813 }) {}

size_t HashStrFn1::get(const Str& x)
{
    size_t hash = 0;
    for (size_t i = 0; i < x.size(); ++i)
    {
        if (i >= coeff.size())
        {
            hash = coeff[coeff.size() - 1] * hash + x.at(i);
        }
        else
        {
            hash = hash + coeff[i] * x.at(i);
        }
    }

    return hash;
}

Str HashStrFn1::toString() const
{
    std::stringstream ss;

    ss  << "hash(x) ->\n"
        << "\n"
        << "    hash <-\n";
    for (size_t i = 0; i < coeff.size(); ++i)
    {
        if (i > 0)
        {
            ss << " +\n";
        }
        ss << "        x(" << i << ") * " << coeff[i];
    }

    ss  << "\n"
        << "    i <- " << coeff.size() << " to size(x) - 1:\n"
        << "        hash <- hash * " << coeff[coeff.size() - 1] << " + x(i)\n";

    return Str(ss.str());
}

bool HashStrFn1::isRandom() const
{
    return false;
}

HashStrFn4::HashStrFn4()
{
    std::random_device rd;
    auto primes = generatePrimes(MAX_PRIME);

    const size_t numCoeff = MIN_NUM_COEFF + rd() % (MAX_NUM_COEFF - MIN_NUM_COEFF +
                            1);
    for (size_t i = 0; i < numCoeff; ++i)
    {
        coeff.push_back(primes[rd() % primes.size()]);
    }
}

size_t HashStrFn4::get(const Str& x)
{
    size_t hash = 0;
    for (size_t i = 0; i < std::min(x.size(), coeff.size()); ++i)
    {
        hash = hash + coeff[i] * x.at(i);
    };

    return hash;
}

Str HashStrFn4::toString() const
{
    std::stringstream ss;

    ss  << "hash(x) -> ";
    for (size_t i = 0; i < coeff.size(); ++i)
    {
        if (i > 0)
        {
            ss << " +\n    ";
        }
        ss << "x(" << i << ") * " << coeff[i];
    }
    ss << " (or earlier if size(x) < " << coeff.size() << ")\n";

    return Str(ss.str());
}

bool HashStrFn4::isRandom() const
{
    return true;
}

HashStrFn2::HashStrFn2() {}

size_t HashStrFn2::get(const Str& x)
{
    size_t hash = 0;
    for (size_t i = 0; i < x.size(); ++i)
    {
        hash = hash * 31 + x.at(i);
    }

    return hash;
}

Str HashStrFn2::toString() const
{
    return "hash(x) -> sum[31^{n-1-i} * x(i)], i <- 0 to n-1\n";
}

bool HashStrFn2::isRandom() const
{
    return false;
}

HashStrFn5::HashStrFn5()
{
    std::random_device rd;
    auto primes = generatePrimes(MAX_PRIME);

    seed = primes[rd() % primes.size()];
    k = primes[rd() % primes.size()];
}

size_t HashStrFn5::get(const Str& x)
{
    size_t hash = seed;
    for (size_t i = 0; i < x.size(); ++i)
    {
        hash = hash * k + x.at(i);
    }

    return hash;
}

Str HashStrFn5::toString() const
{
    std::stringstream ss;
    ss << "hash(x) -> sum[" << k << "^{n-1-i} * x(i)], i <- 0 to n-1\n";
    return Str(ss.str());
}

bool HashStrFn5::isRandom() const
{
    return true;
}

HashStrFn3::HashStrFn3() {}

size_t HashStrFn3::get(const Str& x)
{
    size_t hash = 0;

    for(size_t i = 0; i < x.size(); ++i)
    {
        hash += x.at(i);
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

Str HashStrFn3::toString() const
{
    return
        "hash(x) ->\n"
        "    hash <- 0\n"
        "\n"
        "    for i <- 0 to size(x) - 1:\n"
        "        hash <- hash + x(i)\n"
        "        hash <- hash + (hash << 10)\n"
        "        hash <- hash ^ (hash >> 6)\n"
        "\n"
        "    hash <- hash + (hash << 3)\n"
        "    hash <- hash ^ (hash >> 11)\n"
        "    hash <- hash + (hash << 15)\n"
        "\n"
        "    return hash\n";
}

bool HashStrFn3::isRandom() const
{
    return false;
}

/******************************************
 * helper functions                       *
 ******************************************/
size_t alex::retrievePrime(size_t n)
{
    auto primes = &generatePrimes(n);
    while (primes->size() < n)
    {
        primes = &generatePrimes(2 * primes->size());
    }

    return (*primes).at(n - 1);
}

const std::vector<size_t>& alex::generatePrimes(size_t upto)
{
    // make static to save work
    static std::vector<size_t> primes = { 2, 3 };

    for (size_t n = primes.back() + 2; n <= upto; n += 2)
    {
        if (std::all_of(primes.begin(), primes.end(),
                        [n](size_t x) { return n % x != 0; }))
        {
            primes.push_back(n);
        }
    }

    return primes;
}

