#ifndef __EVALUATION_TOOLS_H__
#define __EVALUATION_TOOLS_H__

#include <vector>
#include <unordered_map>
#include "ISecureIndex.h"
#include "InvertedIndex.h"

namespace alex { namespace eval
{
    template <class T>
    T absoluteError(const std::unordered_map<alex::index::Index, T>& canonical,
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
        return sum;
    }

    double averagePrecisionAtK(const std::vector<double>& precision_k, size_t k = 0)
    {
        if (k == 0 || k > precision_k.size())
            k = precision_k.size();

		if (k == 0)
			return 1;

        double sum = 0;
        for (size_t i = 0; i < k; ++i)
        {
            sum += precision_k[i];
        }
        double avgPrec = sum / k;

        return avgPrec;
    };

    std::vector<double> precisionAtK(const std::unordered_map<alex::index::Index, double>& canonical,
                                     const std::unordered_map<alex::index::SecureIndex, double>& candidate,
                                     size_t k = 0, bool verbose = true)
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

        //vector<double> precision_k(k);
        vector<double> precision_k;
        for (size_t i = 0; i < k; ++i)
        {
            if (canonicalRanked[i].second < 1e-30 && candidateRanked[i].second < 1e-30)
                break;

            canonicalSet.insert(canonicalRanked[i].first);
            candidateSet.insert(candidateRanked[i].first);
            precision_k.push_back(0);
            //precision_k[i] = 0;
            for (const auto& x : canonicalSet)
            {
                if (candidateSet.count(x) != 0)
                {
                    precision_k[i]++;
                }
            }
            precision_k[i] /= canonicalSet.size();
        }

        return precision_k;
    };

    double recall(const std::unordered_set<alex::index::Index>& canonical,
                  const std::unordered_set<alex::index::SecureIndex>& candidate,
                  std::string q = "")
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

        /*
        if (canonicalSet.size() > candidateSet.size())
        {
            std::cout << "Error on q == " << q << std::endl;

            vector<string> diff(canonicalSet.size());
            auto it = set_difference(canonicalSet.begin(), canonicalSet.end(), candidateSet.begin(), candidateSet.end(), diff.begin());
            diff.resize(it - diff.begin());

            for (auto x : diff)
            {
                std::cout << "\t" << x << std::endl;
            }
        }
        */

        vector<string> intersection(canonicalSet.size());
        auto it = set_intersection(canonicalSet.begin(), canonicalSet.end(), candidateSet.begin(), candidateSet.end(), intersection.begin());
        intersection.resize(it - intersection.begin());

        if (intersection.size() == 0 && canonicalSet.size() == 0)
            return 1;

        if (intersection.size() != 0 && canonicalSet.size() == 0)
            return 0;

        return (double)intersection.size() / (double)canonicalSet.size();
    };

    double precision(const std::unordered_set<alex::index::Index>& canonical,
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

        vector<string> intersection(canonicalSet.size());

        auto it = set_intersection(canonicalSet.begin(), canonicalSet.end(), candidateSet.begin(), candidateSet.end(), intersection.begin());
        intersection.resize(it - intersection.begin());

        if (intersection.size() == 0 && candidateSet.size() == 0)
            return 1;

        if (intersection.size() != 0 && candidateSet.size() == 0)
            return 0;

        return (double)intersection.size() / (double)candidateSet.size();
    };
}}

#endif