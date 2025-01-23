#ifndef __EVALUATION_TOOLS_H__
#define __EVALUATION_TOOLS_H__

#include <vector>
#include <unordered_map>
#include "ISecureIndex.h"
#include "InvertedIndex.h"

namespace alex { namespace eval
{
    template <class T>
    void absoluteError(const std::unordered_map<alex::index::Index, T>& canonical,
                       const std::unordered_map<alex::index::SecureIndex, T>& candidate)
    {
        using namespace std;
        namespace i = alex::index;
        map<string, T> canonicalTable;
        map<string, T> candidateTable;
        for (const auto& x : canonical)
            { canonicalTable[x.first->getReference()] = x.second; };
        for (const auto& x : candidate)
            { candidateTable[x.first->getReference()] = x.second; };

        vector<pair<string, T>> differenceTable;
        for (const auto& x : canonicalTable)
        {
            differenceTable.push_back(std::make_pair(x.first, std::abs(canonicalTable[x.first] - candidateTable[x.first])));
        }
        sort(differenceTable.begin(), differenceTable.end(), [](const pair<string, T>& x1, const std::pair<string, T>& x2)
        {
            return x2.second < x1.second;
        });
        
        T sum = 0;
        for (const auto& x : differenceTable)
        {
            sum += x.second;
        }
        std::cout << "absolute error: " << sum << std::endl;
    }

    double averagePrecisionAtK(const std::vector<double>& precision_k, size_t k = 0)
    {
        if (k == 0 || k > precision_k.size())
            k = precision_k.size();

        double sum = 0;
        for (double x : precision_k)
        {
            sum += x;
        }
        double avgPrec = sum / k;
        return avgPrec;
    };

    std::vector<double> precisionAtK(const std::unordered_map<alex::index::Index, double>& canonical,
                                     const std::unordered_map<alex::index::SecureIndex, double>& candidate,
                                     size_t k = 0)
    {
        using namespace std;
        vector<pair<string, double>> canonicalRanked;
        vector<pair<string, double>> candidateRanked;

        for (const auto& x : canonical)
            canonicalRanked.push_back(make_pair(x.first->getReference(), x.second));

        for (const auto& x : candidate)
            candidateRanked.push_back(make_pair(x.first->getReference(), x.second));

        sort(canonicalRanked.begin(), canonicalRanked.end(), [](const pair<string, double>& x1, const pair<string, double>& x2)
        {
            return x2.second < x1.second;
        });

        sort(candidateRanked.begin(), candidateRanked.end(), [](const pair<string, double>& x1, const pair<string, double>& x2)
        {
            return x2.second < x1.second;
        });

        set<string> canonicalSet;
        set<string> candidateSet;
        size_t count = 0;

        if (k == 0 || k > canonicalRanked.size())
            k = canonicalRanked.size();

        vector<double> precision_k(k);
        for (size_t i = 0; i < k; ++i)
        {
            canonicalSet.insert(canonicalRanked[i].first);
            candidateSet.insert(candidateRanked[i].first);
            precision_k[i] = 0;
            for (const auto& x : canonicalSet)
            {
                if (candidateSet.count(x) != 0)
                {
                    precision_k[i]++;
                }
            }
            precision_k[i] /= (double)(i+1);
        }

        return precision_k;
    };

    double recall(const std::unordered_set<alex::index::Index>& canonical,
                  const std::unordered_set<alex::index::SecureIndex>& candidate)
    {
        using namespace std;
        set<string> canonicalSet;
        set<string> candidateSet;

        for (const auto& x : canonical)
        {
            canonicalSet.insert(x->getReference());
        }
        for (const auto& x : candidate)
        {
            candidateSet.insert(x->getReference());
        }

        // recall: retrieved AND relevant / relevant
        // relevant = canonical set
        // retrieved = candidate set
        // retrieved AND relevant = intersection of both

        vector<string> intersection;
        set_intersection(canonicalSet.begin(), canonicalSet.end(), candidateSet.begin(), candidateSet.end(), intersection.begin());
        return (double)intersection.size() / (double)canonicalSet.size();
    };

    double precision(const std::unordered_set<alex::index::Index>& canonical,
                  const std::unordered_set<alex::index::SecureIndex>& candidate)
    {
        // precision: retrieved AND relevant / retrieved
        return ((double)candidate.size() / (double)canonical.size()) * recall(canonical, candidate);
    };
}}

#endif