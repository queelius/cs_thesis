#ifndef __INDEX_BATCH_BUILDER_H__
#define __INDEX_BATCH_BUILDER_H__

#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <set>
#include <thread>
#include <future>
#include <unordered_map>
#include <unordered_set>
#include "BitArray.h"
#include "Utils.h"
#include "FileSystem.h"
#include "Str.h"
#include "BsiBlockBuilder.h"
#include "PsiBlockBuilder.h"
#include "PsiFreqBuilder.h"

namespace alex { namespace index
{
    template <class IndexBuilder>
    class IndexBatchBuilder
    {
    public:
        IndexBatchBuilder(const typename IndexBuilder::BuilderParams& params) :
            _params(params) {};

        size_t build(const boost::filesystem::path& outDirectory, const boost::filesystem::path& inDirectory,
                   size_t recursionDepth = std::numeric_limits<size_t>::max(), const std::string& inFilePattern = ".*",
                   bool useConcurrency = true, bool verbose = true)
        {
            const auto files = alex::io::filesystem::getFiles(inDirectory, inFilePattern, recursionDepth);
            std::vector<std::future<std::pair<boost::filesystem::path, bool>>> futures;

            const auto& params = _params;
            for (const auto& file : files)
            {
                if (file.stem().string() == "README")
                    continue;

                auto launch = std::launch::async | std::launch::deferred;
                if (!useConcurrency)
                    launch = std::launch::deferred;

                futures.push_back(std::async(launch,
                    [&file, &outDirectory, &params, verbose]() -> std::pair<boost::filesystem::path, bool>
                {
                    try
                    {
                        IndexBuilder builder(params);

                        builder.parse(file.string());
                        builder.makeTerms();
                        auto si = builder.build();
                        
                        boost::filesystem::path outfile;
                        outfile += outDirectory;
                        outfile += outfile.preferred_separator;
                        outfile += file;

                        boost::filesystem::path tmp(outfile);
                        tmp.replace_extension(builder.getPreferredFileExtension());
                        tmp.normalize();
                        unsigned int append = 0;
                        while (boost::filesystem::exists(tmp))
                        {
                            tmp = outfile;
                            tmp += "_" + std::to_string(++append);
                            tmp.replace_extension(builder.getPreferredFileExtension());
                            tmp.normalize();
                        }
                        outfile = tmp;

                        alex::io::filesystem::makeDirectory(outfile.parent_path());
                        si->write(outfile.string());

                        if (verbose)
                            std::cout << "* Built index: " << outfile << std::endl;

                        return std::make_pair(file, true);
                    }
                    catch (...)
                    {
                        return std::make_pair(file, false);
                    }
                }));
            }

            for (auto& future : futures)
            {
                auto p = future.get();
                if (!p.second)
                    std::cout << "* Failed to index " << p.first << "." << std::endl;
            }

            return futures.size();
        };

    private:
        typename IndexBuilder::BuilderParams _params;
    };
}}

#endif