#ifndef __CORPUS_H__
#define __CORPUS_H__

#include "FileSystem.h"
#include "Entropy.h"
#include "NGramModel.h"
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
    void makeNGramDocument(size_t numTokens,
                           size_t nGramSize,
                           alex::ngrams::NGramModel& model,
                           std::ofstream& file,
                           alex::stochastic::Entropy<T>& entropy);

    template <class T>
    void makeNGramCorpus(size_t numDocs,
                         size_t minTokens,
                         size_t maxTokens,
                         size_t nGramSize,
                         const boost::filesystem::path& outputDir,
                         alex::ngrams::NGramModel& model,
                         alex::stochastic::Entropy<T>& entropy);

    template <class T>
    void makeNGramCorpus(size_t numDocs,
                         size_t minTokens,
                         size_t maxTokens,
                         size_t nGramSize,
                         const boost::filesystem::path& outputDir,
                         alex::ngrams::NGramModel& model,
                         alex::stochastic::Entropy<T>& entropy)
    {
        alex::io::filesystem::makeDirectory(outputDir);

        for (size_t i = 0; i < numDocs; ++i)
        {
            size_t tokens = entropy.getInt(minTokens, maxTokens);
            std::stringstream ss;
            ss << boost::uuids::random_generator()();

            auto path = outputDir;
            path += path.preferred_separator;
            path += ss.str();
            std::ofstream file(path.string());
            makeNGramDocument(tokens, nGramSize, model, file, entropy);
        }
    }

    template <class T>
    void makeNGramDocument(size_t numTokens,
                           size_t nGramSize,
                           alex::ngrams::NGramModel& model,
                          std::ofstream& file,
                          alex::stochastic::Entropy<T>& entropy)
    {
        const static size_t MIN_PARAGRAPH_LENGTH = 100;

        auto inverseCdf = model.inverseCdfFn();
        std::vector<std::string> evidence;
        for (size_t i = 1; i < nGramSize; ++i)
            evidence.push_back("<START>");

        size_t n = 0;
        for (size_t i = 1; i < numTokens; ++i)
        {
            std::string token = inverseCdf((float)entropy.get0_1(), evidence);

            if (token == "<START>")
                continue;
            else if (token == "<END>")
            {
                n = 0;
                file << "\n\n\n";

                evidence.clear();
                for (size_t i = 1; i < nGramSize; ++i)
                    evidence.push_back("<START>");
            }
            else
                file << token;

            if (i % MIN_PARAGRAPH_LENGTH == 0)
                file << "\n\n";
            else
                file << " ";
            
            evidence.erase(evidence.begin());
            evidence.push_back(token);
        }
    }
}}

#endif
