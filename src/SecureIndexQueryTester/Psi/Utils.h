#ifndef __UTILS_H__
#define __UTILS_H__

#include <vector>
#include <algorithm>
#include <istream>
#include <unordered_set>
#include <set>
#include <map>
#include "Entropy.h"
#include <random>
#include "Porter2Stemmer.h"
#include "SHA.h"
#include "Globals.h"
using namespace alex;

namespace alex { namespace utils
{
    typedef std::vector<std::vector<int>> MatrixInt;

    template <class T = std::string>
    class Pair
    {
    public:
        Pair(const Pair& p) : x(p.x), y(p.y) {};
        Pair(Pair&& p) : x(std::move(p.x)), y(std::move(p.y)) {};

        Pair(const T& x, const T& y)
        {
            if (x < y)
            {
                this->x = x;
                this->y = y;
            }
            else
            {
                this->y = x;
                this->x = y;
            }
        };

        Pair& operator=(Pair&& p)
        {
            x = std::move(p.x);
            y = std::move(p.y);
        };

        Pair& operator=(const Pair& p)
        {
            x = p.x;
            y = p.y;
        };

        bool operator<(const Pair& p) const
        {
            if (x < p.x)
                return true;
            else if (p.x < x)
                return false;
            else
                return y < p.y;
        };

        inline T X() const { return x; };
        inline T Y() const { return y; };

    private:
        T x;
        T y;
    };

    std::map<Pair<std::string>, unsigned int> findMinPairs(std::vector<std::string>& words, unsigned int k)
    {
        std::map<Pair<std::string>, unsigned int> minPairs;
        for (unsigned int i = 0; i < words.size(); ++i)
        {
            for (unsigned int j = i + 1; (j <= i + k) && (j < words.size()); ++j)
            {
                if (words[i] == words[j])
                    continue;
                auto p = Pair<>(words[i], words[j]);
                if (minPairs.count(p) == 0 || j - i < minPairs[p])
                    minPairs[p] = j - i;
            }
        }
        return minPairs;
    };

    void print(MatrixInt m)
    {
        for (size_t i = 0; i < m.size(); ++i)
        {
            for (size_t j = 0; j < m[i].size(); ++j)
                std::cout << m[i][j] << " ";
            std::cout << std::endl;
        }
    };

    void cover(const std::vector<std::vector<uint32_t>>& postings,
               uint32_t next, uint32_t min, uint32_t max,
               std::vector<std::vector<uint32_t>>& output,
               uint32_t cutoff, uint32_t k, std::vector<uint32_t> path, std::vector<std::set<uint32_t>>& used)
    {
        if (output.size() == k)
            return;

        if (next == postings.size())
        {
            output.push_back(path);
            for (size_t i = 0; i < path.size(); ++i)
                used[i].insert(path[i]);
            return;
        }

        for (size_t j = 0; j < postings[next].size(); ++j)
        {
            if (output.size() == k)
                return;

            if (used[next].count(postings[next][j]) != 0)
                continue;

            uint32_t newMax = std::max(postings[next][j], max);
            uint32_t newMin = std::min(postings[next][j], min);

            if (newMax - newMin > cutoff)
                return;

            path[next] = postings[next][j];
            cover(postings, next+1, newMin, newMax, output, cutoff, k, path, used);
        }
    }

    void cover2(const std::vector<std::vector<uint32_t>>& postings,
               uint32_t next, uint32_t min, uint32_t max,
               std::vector<uint32_t>& output,
               uint32_t cutoff, size_t k, std::vector<uint32_t> path, std::vector<std::set<uint32_t>>& used)
    {
        if (k == output.size())
            return;
        else if (next == postings.size())
        {
            for (size_t i = 0; i < path.size(); ++i)
            {
                if (used[i].count(path[i]) != 0)
                    return;
            }

            output.push_back((min + max) / 2);
            for (size_t i = 0; i < path.size(); ++i)
                used[i].insert(path[i]);
        }
        else
        {
            for (size_t j = 0; j < postings[next].size(); ++j)
            {
			    if (k == output.size())
				    return;

                if (used[next].count(j) != 0)
                    continue;

                uint32_t newMax = std::max(postings[next][j], max);
                uint32_t newMin = std::min(postings[next][j], min);

                if (newMax - newMin > cutoff)
                    return;
                path[next] = j;
                cover2(postings, next+1, newMin, newMax, output, cutoff, k, path, used);
            }
        }
    }

    void cover3(const std::vector<std::vector<uint32_t>>& postings,
               uint32_t next, uint32_t min, uint32_t max,
               std::vector<std::pair<uint32_t, uint32_t>>& output,
               uint32_t cutoff, size_t k, std::vector<uint32_t> path, std::vector<std::set<uint32_t>>& used)
    {
        if (k == output.size())
            return;

        if (next == postings.size())
        {
            output.push_back(std::make_pair(min, max));
            for (size_t i = 0; i < path.size(); ++i)
                used[i].insert(path[i]);
        }

        for (size_t j = 0; j < postings[next].size(); ++j)
        {
			if (k == output.size())
				return;

            if (used[next].count(postings[next][j]) != 0)
                continue;

            uint32_t newMax = std::max(postings[next][j], max);
            uint32_t newMin = std::min(postings[next][j], min);

            if (newMax - newMin > cutoff)
                return;

            cover3(postings, next+1, newMin, newMax, output, cutoff, k, path, used);
        }
    }

    MatrixInt makeMatrixInt(int n, int value)
    {
        MatrixInt m;
        m.reserve(n);
        for (int i = 0; i < n; ++i)
        {
            m.push_back(std::vector<int>(n, value));
        }
        return m;
    };

    std::vector<std::string> cryptoTerms(const std::string& term, const std::vector<std::string>& secrets, const std::string& salt)
    {
        std::vector<std::string> results;
        results.reserve(secrets.size());
        for (auto secret : secrets)
        {
            std::string stage1 = alex::crypt::hexdigest<alex::globals::digest_size>(term + "|" + std::move(secret));
            std::string stage2 = alex::crypt::hexdigest<alex::globals::digest_size>(std::move(stage1) + "|" + salt);
            results.push_back(std::move(stage2));
        }
        return results;
    }

    template <class T>
    std::string toString(const std::vector<T>& vs)
    {
        std::stringstream ss;
        ss << "[ ";
        for (const auto& v : vs)
            ss << v << " ";
        ss << "]";
        return ss.str();
    }

    MatrixInt getMinPwDistances(const std::vector<std::vector<uint32_t>>& p)
    {
        std::vector<int> places(p.size(), 0);
        std::vector<int> last(p.size(), -1);
        alex::utils::MatrixInt m = makeMatrixInt(p.size(), std::numeric_limits<int>::max());
        int idx, minPlace;

        do
        {
            minPlace = std::numeric_limits<int>::max();
            idx = -1;
            for (int i = 0; i < places.size(); ++i)
            {
                if (p[i].size() <= places[i])
                    continue;

                auto place = p[i][places[i]];
                if (place < minPlace)
                {
                    idx = i;
                    minPlace = place;
                }
            }

            if (idx != -1)
            {
                last[idx] = p[idx][places[idx]];
                for (int i = 0; i < p.size(); ++i)
                {
                    if (i == idx || last[i] == -1)
                        continue;

                    int d = std::abs(last[i] - last[idx]);
                    if (d < m[idx][i])
                        m[idx][i] = m[i][idx] = d;
                }
                ++places[idx];
            }
        } while (idx != -1);

        return m;
    };

    double getMinPwDist(const std::vector<std::vector<uint32_t>>& p, int max)
    {
        const auto& m = getMinPwDistances(p);
        double d = 0;
        for (size_t i = 1; i < m.size(); ++i)
        {
            for (size_t j = 0; j < i; ++j)
            {
                if (m[i][j] == std::numeric_limits<int>::max())
                    d += max;
                else
                    d += m[i][j];
            }
        }
        return d;
    };

    std::vector<std::string> ngrams(const std::vector<std::string>& words,
                                    size_t minGram,
                                    size_t maxGram, bool lexographic);
    std::vector<std::string> extractTerms(std::istream& ins, bool stem = false, bool stopWords = false);
    void removeDupes(std::vector<std::string>& xs);
    template <class Iter> char** toArray(Iter begin, Iter end);
    size_t toBytes(size_t bits);
    template <class T> unsigned int levenshtein(const T& s1, const T& s2);
    std::string generateId(size_t size);

    std::string generateId(size_t size)
    {
        const static std::vector<char> alphabet =
        {
            'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
            'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
            's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0',
            '1', '2', '3', '4', '5', '6', '7', '8', '9'
        };

        std::string s;

        auto& entropy = stochastic::Entropy<std::minstd_rand0>();
        for(size_t i = 0; i < size; ++i)
        {
            s.push_back(alphabet[entropy.get() % alphabet.size()]);
        }
        return s;
    }

    std::vector<std::string> ngrams(const std::vector<std::string>& words,
                                    size_t minGram,
                                    size_t maxGram, bool lexographic)
    {
        std::vector<std::string> ngrams;
        for(size_t j = 0; j < words.size(); ++j)
        {
            std::vector<std::string> kgram;
            const size_t sz = words.size() < (j + maxGram) ?
                              words.size() : j + maxGram;

            if(lexographic)
            {
                for(size_t k = j; k < sz; ++k)
                {
                    kgram.insert(std::lower_bound(kgram.begin(), kgram.end(), words[k]), words[k]);
                    if(k - j + 1 < minGram)
                    {
                        continue;
                    }

                    std::string gramStr = kgram[0];
                    for(size_t i = 1; i < kgram.size(); ++i)
                    {
                        gramStr += " " + kgram[i];
                    }
                    ngrams.push_back(gramStr);
                }
            }
            else
            {
                std::string kgram;
                for(size_t k = j; k < sz; ++k)
                {
                    if(k != j)
                    {
                        kgram += " ";
                    }
                    kgram += words[k];
                    if(k - j + 1 < minGram)
                    {
                        continue;
                    }

                    ngrams.push_back(kgram);
                }
            }
        }

        return ngrams;
    }

    void removeDupes(std::vector<std::string>& xs)
    {
        std::sort(xs.begin(), xs.end());
        auto newEnd = std::unique(xs.begin(), xs.end());
        xs.erase(newEnd, xs.end());
    }

    template <class Iter> char** toArray(Iter begin, Iter end)
    {
        char** v = new char* [std::distance(begin, end)];
        size_t cnt = 0;
        for(auto i = begin; i != end; ++i)
        {
            v[cnt] = new char[i->size() + 1];
            std::copy(i->begin(), i->end(), v[cnt]);
            v[cnt++][i->size()] = 0;
        }
        return v;
    }

    size_t toBytes(size_t bits)
    {
        return (bits - 1) / CHAR_BIT + 1;
    }
}}

#endif