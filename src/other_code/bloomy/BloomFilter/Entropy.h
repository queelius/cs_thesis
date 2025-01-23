#ifndef __ENTROPY_H__
#define __ENTROPY_H__

#include <functional>
#include <random>

namespace alex
{
namespace stats
{
template <class ENGINE = std::mt19937>
class Entropy
{
public:
    static Entropy& instance()                  { static Entropy<ENGINE> entropy; return entropy; };

    void autoseed()                             { _eng.seed(_rd()); };
    void seed(long value)                       { _seed = value; _eng.seed(value); };
    bool flip()                                 { return _eng() % 2 == 0; };
    size_t get()                                { return _eng(); };
    size_t get(size_t min, size_t max)          { return min + get() % (max - min + 1); };
    int getInt(int min, int max)                { return (int)(min + get() % (max - min + 1)); };
    double getReal(double min, double max)      { return min + get0_1() * (max - min); };
    double get0_1()                             { return (double)_eng() / _eng.max(); };
    long getSeed()                              { return _seed; };

    std::random_device& randomDevice()          { return _rd; };
    ENGINE& engine()                            { return _eng; };

private:
    Entropy() { autoseed(); };

    Entropy(const Entropy&);
    void operator=(const Entropy&);

    long _seed;
    ENGINE _eng;
    std::random_device _rd;
};
};
};

#endif