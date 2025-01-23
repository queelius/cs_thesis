#ifndef __CORPUS_H__
#define __CORPUS_H__

#include "FileSystem.h"
#include "TokenDistribution.h"
#include <string>
#include <memory>
#include <fstream>
#include <iostream>

namespace alex { namespace corpus
{
    // read from a real collection of documents
    alex::stochastic::TokenDistribution scanDirectory(const std::vector<boost::filesystem::path>& dirs, const std::string& filePattern = ".*", size_t recursionDepth = 1)
    {
        std::vector<boost::filesystem::path> files;
            
        for (const auto& dir : dirs)
        {
            auto fs = alex::io::filesystem::getFiles(std::move(dir), filePattern, recursionDepth);
            files.insert(files.end(), fs.begin(), fs.end());
        }
            
        std::map<std::string, float> pdf;
        for (const auto& file : files)
        {
            std::ifstream ins(file.string());
            if (!ins.is_open())
            {
                std::cout << "Error: Could Not Open " << file << std::endl;
                continue;
            }

            size_t i = 0;
            std::string token;
            while (ins.good())
            {
                char c = ins.get();
                if (isalnum(c))
                {
                    c = tolower(c);
                    token += c;
                }
                else if (c == '\'')
                    continue;
                else
                {
                    if (!token.empty())
                    {
                        if (pdf.count(token) == 0)
                            pdf[std::move(token)] = 1;
                        else
                            ++pdf[std::move(token)];
                        token = "";
                    }
                }
            }
        }

        return alex::stochastic::TokenDistribution(std::move(
            std::vector<std::pair<std::string, float>>(pdf.begin(), pdf.end())));
    }
}}

#endif