#ifndef __ENTROPY_H__
#define __ENTROPY_H__

#include <functional>
#include <random>

namespace alex { namespace stochastic
{
    template <class E = std::mt19937>
    class Entropy
    {
    public:
        Entropy(unsigned long thisSeed = 0)
        {
            seed(thisSeed);
        };

        Entropy(const Entropy& other) :  _eng(other._eng), _seed(other._seed)
        {
        };

        Entropy& operator=(const Entropy& other)
        {
            if (this != &other)
            {
                _eng = other._eng;
                _seed = other._seed;
            }
            return *this;
        };

        void seed(unsigned int thisSeed = 0)
        {
            if (thisSeed == 0)
            {
                std::random_device rd;
                thisSeed = rd();                
            }
            _eng.seed(thisSeed);
        };

        bool flip()
        {
            return get() % 2 == 0;
        };

        unsigned int get()
        {
            return _eng();
        };

        unsigned int get(unsigned int min, unsigned int max)
        {
            return min + get() % (max - min + 1);
        };

        int getInt(int min, int max)
        {
            return (int)(min + get() % (max - min + 1));
        };

        double getReal(double min, double max)
        {
            return min + get0_1() * (max - min);
        };

        double get0_1()
        {
            return (double)get() / _eng.max();
        };

        float get0_1f()
        {
            return (float)get() / _eng.max();
        };

    private:
        E _eng;
    };
}}

#endif