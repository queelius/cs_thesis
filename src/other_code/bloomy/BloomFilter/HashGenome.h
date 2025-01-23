#ifndef __HASH_GENOME_H__
#define __HASH_GENOME_H__

#include <functional>
#include "Entropy.h"
#include "Str.h"

class HashGene
{
public:
    HashGene() { randomize(); };

    void randomize()
    {    
        for (int i = '0'; i <= '9'; ++i)
            map[i] = GET_INTEGER(0, 27644437);
        for (int i = 'a'; i <= 'z'; ++i)
            map[i] = GET_INTEGER(0, 27644437);
    };

    HashGene mate(const HashGene& other) const
    {
        HashGene child;
        const int split = GET_INTEGER(0, 37);

        for (int i = 0; i < split; ++i)
            child.map[i] = map[i];
        for (int i = split; i <= 36; ++i)
            child.map[i] = other.map[i];

        return child;
    };

    inline void set(int c, int v)
    {
        map[c - (c < 'a' ? '0' : 'a')] = v;
    };


    inline int get(int c) const
    {
        return c < 'a' ? map[c - '0'] : map[c - 'a'];
    };

    inline void mutate()
    {
        if (GET_01() < _mutate)
            map[GET_INTEGER(0, 36)] = GET_INTEGER(0, 27644437);
    };

private:
    const static std::function<int(int,int)>& GET_INTEGER;
    const static std::function<double(void)>& GET_01;

    double _mutate;
    int map[36];
    
};

class HashGenome
{
public:
    std::vector<HashGene>& getGenes()
    {
        return genes;
    };

    HashGene& getGene(size_t n)
    {
        if (n >= genes.size())
            throw std::exception("Invalid Argument");

        return genes[n];
    };

    HashGenome mate(const HashGenome& other)
    {
    };

private:
    std::vector<HashGene> genes;
};

const std::function<int(int,int)>& HashGene::GET_INTEGER = alex::stats::Entropy<>::instance().getInt;
const std::function<double(void)>& HashGene::GET_01 = alex::stats::Entropy<>::instance().get0_1;

#endif