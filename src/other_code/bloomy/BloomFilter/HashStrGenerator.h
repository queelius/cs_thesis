#ifndef __HASH_STR_GENERATOR_H__
#define __HASH_STR_GENERATOR_H__

#include <functional>
#include <memory>
#include <vector>
#include "Str.h"

namespace alex
{

size_t retrievePrime(size_t n);
const std::vector<size_t>& generatePrimes(size_t upto);

class HashStrFn: public std::function<size_t(const Str&)>
{
public:
    virtual size_t get(const Str&) = 0;
    virtual Str toString() const = 0;
    virtual bool isRandom() const = 0;
};

class HashStrFn1: public HashStrFn
{
public:
    HashStrFn1();

    size_t get(const Str& x);

    Str toString() const;

    bool isRandom() const;

private:
    std::vector<size_t> coeff;
};

class HashStrFn4: public HashStrFn
{
public:
    static const size_t MAX_PRIME = 70000;
    static const size_t MIN_NUM_COEFF = 3;
    static const size_t MAX_NUM_COEFF = 10;

    HashStrFn4();

    size_t get(const Str& x);

    Str toString() const;

    bool isRandom() const;

private:
    std::vector<size_t> coeff;
};

class HashStrFn2: public HashStrFn
{
public:
    HashStrFn2();

    size_t get(const Str& x);

    Str toString() const;

    bool isRandom() const;
};

class HashStrFn5: public HashStrFn
{
public:
    static const size_t MAX_PRIME = 70000;

    HashStrFn5();

    size_t get(const Str& x);

    Str toString() const;

    bool isRandom() const;

private:
    size_t seed;
    size_t k;
};

class HashStrFn3: public HashStrFn
{
public:
    HashStrFn3();
    
    size_t get(const Str& x);

    Str toString() const;

    bool isRandom() const;
};

class HashStrGenerator
{
public:
    HashStrGenerator();

    std::vector<size_t> getHashIds() const;
    std::shared_ptr<HashStrFn> make(size_t id);
};

};

#endif