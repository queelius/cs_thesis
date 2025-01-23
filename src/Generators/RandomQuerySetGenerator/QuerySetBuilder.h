#ifndef __QUERY_SET_BUILDER_H__
#define __QUERY_SET_BUILDER_H__

#include "FileSystem.h"
#include "Query.h"
#include "Entropy.h"
#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <set>

namespace alex { namespace corpus
{
    class RandomQuerySetBuilder
    {
    public:
        struct Params
        {
            size_t numQueries;
            size_t minTerms;
            size_t maxTerms;
            size_t minTokens;
            size_t maxTokens;
            size_t maxTokenSize;
            size_t minTokenSize;
            std::vector<char> alphabet;

            Params()
            {
                defaults();
            };

            void defaults()
            {
                numQueries = 1;
                minTerms = 1;
                maxTerms = 1;
                minTokens = 1;
                maxTokens = 1;
                maxTokenSize = 12;
                minTokenSize = 1;
                alphabet =
                {
                    'a', 'b', 'c', 'd', 'e',
                    'f', 'g', 'h', 'i', 'j',
                    'k', 'l', 'm', 'n', 'o',
                    'p', 'q', 'r', 's', 't',
                    'u', 'v', 'w', 'x', 'y',
                    'z'
                };
            };
        };

        RandomQuerySetBuilder(const Params& params) : _params(params) {};

        Params getParams() const
        {
            return _params;
        };

        Params& getParams()
        {
            return _params;
        };

        std::string makeRandomToken(alex::stochastic::Entropy<>& ent)
        {
            std::string token;
            size_t sz = ent.getInt(_params.minTokenSize, _params.maxTokenSize);
            token.reserve(sz);
            for (size_t i = 0; i < sz; ++i)
                token.push_back(_params.alphabet[ent.get() % _params.alphabet.size()]);

            return token;
        }

        template <class T>
        std::set<alex::index::Query> make(alex::stochastic::Entropy<T>& entropy)
        {
            std::set<alex::index::Query> queries;

            while (queries.size() < _params.numQueries)
            {
                std::set<alex::index::Query::Term> terms;

                size_t numTerms = entropy.getInt(_params.minTerms, _params.maxTerms);
                while (terms.size() < numTerms)
                {
                    std::set<std::string> tokenSet;

                    size_t numTokens = entropy.getInt(_params.minTokens, _params.maxTokens);
                    while (tokenSet.size() < numTokens)
                        tokenSet.insert(makeRandomToken(entropy));

                    std::vector<std::string> shuffledTokens;
                    for (const auto& token : tokenSet)
                        shuffledTokens.push_back(token);
                    std::random_shuffle(shuffledTokens.begin(), shuffledTokens.end());

                    alex::index::Query::Term term;
                    for (const auto& token : shuffledTokens)
                        term.addToken(token);

                    terms.insert(term);
                }

                alex::index::Query q;
                for (const auto& term : terms)
                    q.addTerm(term);

                queries.insert(q);
            }

            return queries;
        };

        Params _params;

    };
}}

#endif