#ifndef __QUERY_SET_BUILDER_H__
#define __QUERY_SET_BUILDER_H__

#include "FileSystem.h"
#include "Query.h"
#include "Entropy.h"
#include "RandomVariates.h"
#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <set>

namespace alex { namespace corpus
{
    class QuerySetSeeder
    {
    public:
        struct Params
        {
            std::vector<alex::index::new_query_interface::Query> queries;
            float minLoadFactor, maxLoadFactor;
            float minBinomial, maxBinomial;
            size_t maxBlockSize;
            size_t minBlockSize;

            Params()
            {
                defaults();
            };

            void defaults()
            {
                minBinomial = 0.02f;
                maxBinomial = 0.16f;
                minLoadFactor = 0.0001f;
                maxLoadFactor = 0.005f;
                maxBlockSize = 7500;
                minBlockSize = 1500;
            };
        };

        QuerySetSeeder(const Params& params) : _params(params) {};

        Params getParams() const
        {
            return _params;
        };

        Params& getParams()
        {
            return _params;
        };

        template <class T>
        void seedDocuments(alex::stochastic::Entropy<T>& entropy,
                           const std::vector<std::string>& dirs,
                           const std::string& outSeededDir,
                           const std::string& filePattern = ".*",
                           size_t recursionDepth = 1,
                           std::string querySetLabel = "",
                           bool verbose = false)
        {
            std::vector<boost::filesystem::path> files;
            
            for (const auto& dir : dirs)
            {
                auto fs = alex::io::filesystem::getFiles(dir, filePattern, recursionDepth);
                files.insert(files.end(), fs.begin(), fs.end());
            }

            if (files.empty())
            {
                std::cout << "No matching files." << std::endl;
                return;
            }

            auto outSeed = boost::filesystem::path(outSeededDir);
            alex::io::filesystem::makeDirectory(outSeed);
            for (auto file : files)
            {
                try
                {
                    if (verbose)
                        std::cout << "\t* Parsing file: " << file.string() << std::endl;

                    std::ifstream inStream(file.string());

                    if (!inStream.is_open())
                    {
                        std::cout << "Error: Could Not Open " << file << std::endl;
                        continue;
                    }

                    auto tokens = tokenize(inStream);
                    std::vector<std::pair<alex::index::new_query_interface::Query::Term, size_t>> seedPos =
                        getSeedPositions(tokens, entropy, verbose);

                    if (!seedPos.empty())
                    {
                        for (size_t i = 0; i < seedPos.size(); ++i)
                        {
                            for (size_t j = 0; j < seedPos[i].first.size(); ++j)
                            {
                                if (seedPos[i].second + 2*j < tokens.size())
                                    tokens[seedPos[i].second + 2*j] = seedPos[i].first[j];
                                else
                                    tokens.push_back(seedPos[i].first[j]);

                                if (seedPos[i].second + 2*j + 1 < tokens.size())
                                    tokens[seedPos[i].second + 2*j + 1] = " ";
                                else
                                    tokens.push_back(" ");
                            }
                        }
                    }
                    
                    auto newFilename = outSeed;
                    newFilename += file.preferred_separator;
                    newFilename += file.filename();

                    std::ofstream outputFile(newFilename.string());
                    if (!outputFile.is_open())
                    {
                        std::cout << "\t* Could not write file: " << newFilename << std::endl;
                    }
                    else
                    {
                        if (verbose)
                            std::cout << "\t* Writing to file: " << newFilename << std::endl;
                        for (const auto& token : tokens)
                            outputFile << token;
                    }
                }
                catch (const std::exception& e)
                {
                    std::cout << "Error: " << e.what() << std::endl;
                }
                catch (const char* e)
                {
                    std::cout << "Error: " << e << std::endl;
                }
                catch (...)
                {
                    std::cout << "Error: Unknown cause" << std::endl;
                }
            }
        };

        template <class T>
        std::vector<std::pair<alex::index::new_query_interface::Query::Term, size_t>> getSeedPositions(const std::vector<std::string>& doc, alex::stochastic::Entropy<T>& entropy,
                                                                                  bool verbose = false)
        {
            namespace q = alex::index;
            size_t docSize = doc.size();
            std::vector<std::pair<q::new_query_interface::Query::Term, size_t>> positions;

            for (auto query : _params.queries)
            {
                unsigned int blockSize = entropy.getInt(_params.minBlockSize, _params.maxBlockSize);
                if (blockSize > doc.size())
                    blockSize = doc.size();
                unsigned int start = entropy.getInt(0, doc.size() - blockSize);

                if (verbose)
                    std::cout << "\t* Processing query: " << query.toString() << std::endl;

                for (size_t i = 0; i < query.size(); ++i)
                {

                    // this is wrong
                    // was for most experiments: float p = entropy.getReal(_params.minBinomial, _params.maxBinomial);
                    // should have been:         float p = entropy.getReal(_params.minBinomial, _params.maxBinomial);
                    // all old experiments were using a using a constant probability per term: 0.2
                    float p = entropy.getReal(_params.minBinomial, _params.maxBinomial);
                    if (entropy.get0_1() >= p)
                        continue;
                    float loadFactor = entropy.getReal(_params.minLoadFactor, _params.maxLoadFactor);

                    auto term = query[i];

                    // at least the term should appear once since it already passed the test above
                    //size_t freq = (size_t)(loadFactor * std::pow(std::sqrt((float)docSize), 1.0f / std::sqrt((float)term.size())));
                    size_t freq = (size_t)(loadFactor * std::pow((float)blockSize, 4.0f / 5.0f));
                    if (freq == 0)
                        freq = 1;

                    for (size_t k = 0; k < freq; ++k)
                    {
                        //size_t pos = (size_t)entropy.getInt(0, (int)docSize-1);
                        size_t pos = (size_t)entropy.getInt(start, start + (int)blockSize - 1);
                        while (pos < doc.size() && (isspace(doc.at(pos).at(0)) || ispunct(doc.at(pos).at(0))))
                            pos = (size_t)entropy.getInt(start, start + (int)blockSize - 1);
                        positions.push_back(std::make_pair(term, pos));
                    }
                }
            }

            std::sort(positions.begin(), positions.end(), [](
                const std::pair<q::new_query_interface::Query::Term, size_t>& x1, const std::pair<q::new_query_interface::Query::Term, size_t>& x2)
            {
                return x1.second < x2.second;
            });

            return positions;
        };

        std::vector<std::string> tokenize(std::ifstream& file)
        {
            std::string token;
            std::vector<std::string> tokens;
            while (file.good())
            {
                char c = file.get();
                if (!isspace(c))
                {
                    token += c;
                }
                else
                {
                    if (!token.empty())
                    {
                        tokens.push_back(std::move(token));
                        token = "";
                    }
                    tokens.push_back(std::string() + c);
                }
            }

            if (token.size() > 1)
                tokens.push_back(std::move(token));

            return tokens;
        };

        Params _params;

    };
}}

#endif