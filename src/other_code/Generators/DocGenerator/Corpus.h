#ifndef __CORPUS_H__
#define __CORPUS_H__

#include "FileSystem.h"
#include "TokenDistribution.h"
#include "Entropy.h"
#include <boost/filesystem.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string>
#include <memory>
#include <fstream>
#include <iostream>

namespace alex { namespace corpus
{
    template <class T>
    void makeRandomUnigramDocument(size_t tokens, alex::stochastic::TokenDistribution dist,
                            std::ofstream& file, alex::stochastic::Entropy<T>& entropy);

    template <class T>
    void makeRandomUnigramCorpus(size_t numDocs,
                            size_t minUniqueTokens,
                            size_t maxUniqueTokens,
                            size_t minTokens,
                            size_t maxTokens,
                            const boost::filesystem::path& outputDir,
                            alex::stochastic::TokenDistribution dist,
                            alex::stochastic::Entropy<T>& entropy);

    template <class T>
    void makeRandomUnigramCorpus(size_t numDocs,
                            size_t minUniqueTokens,
                            size_t maxUniqueTokens,
                            size_t minTokens,
                            size_t maxTokens,
                            const boost::filesystem::path& outputDir,
                            alex::stochastic::TokenDistribution dist,
                            alex::stochastic::Entropy<T>& entropy)
    {
        alex::io::filesystem::makeDirectory(outputDir);

        for (size_t i = 0; i < numDocs; ++i)
        {
            size_t uniqueTokens = entropy.getInt(minUniqueTokens, maxUniqueTokens);
            size_t tokens = entropy.getInt(minTokens, maxTokens);
            std::stringstream ss;
            ss << boost::uuids::random_generator()();

            auto path = outputDir;
            path += path.preferred_separator;
            path += ss.str();
            std::ofstream file(path.string());

            if (uniqueTokens == dist.size())
                makeRandomUnigramDocument(tokens, dist, file, entropy);
            else
                makeRandomUnigramDocument(tokens, dist.reduce(uniqueTokens, entropy), file, entropy);
        }
    };

    template <class T>
    void makeRandomUnigramDocument(size_t numTokens, alex::stochastic::TokenDistribution dist,
                          std::ofstream& file, alex::stochastic::Entropy<T>& entropy)
    {
        const static size_t MIN_PARAGRAPH_LENGTH = 100;

        file << dist.inverseCdf(entropy.get0_1());
        for (size_t i = 1; i < numTokens; ++i)
        {
            if (i % MIN_PARAGRAPH_LENGTH == 0)
                file << "\n\n";
            else
                file << " ";
            file << dist.inverseCdf(entropy.get0_1());
        }
    };
}}

#endif
