#ifndef __FILE_SYSTEM_H__
#define __FILE_SYSTEM_H__

#include <boost/filesystem.hpp>
#include <vector>
#include <iostream>
#include "Str.h"

namespace alex { namespace io { namespace filesystem
{
    std::vector<boost::filesystem::path> getFiles(const boost::filesystem::path& p, std::string filePattern = ".*", size_t recursionDepth = 1)
    {
        std::vector<boost::filesystem::path> files;
        if (boost::filesystem::exists(p))
        {
            if (boost::filesystem::is_regular_file(p))
            {
                if (alex::str::hasMatch(p.filename().string(), filePattern))
                {
                    files.push_back(p);
                }
            }
            else if (recursionDepth != 0 && boost::filesystem::is_directory(p))
            {
                for (auto it = boost::filesystem::directory_iterator(p), it_end = boost::filesystem::directory_iterator(); it != it_end; ++it)
                {
                    const auto results = getFiles(*it, filePattern, recursionDepth-1);
                    files.insert(files.end(), results.begin(), results.end());
                }
            }
        }
        else
        {
            throw std::exception((p.string() + " Not Found").c_str());
        }

        return files;
    };

    size_t getDirectorySize(const boost::filesystem::path& p, std::string filePattern = ".*", size_t recursionDepth = 1)
    {
        size_t totalSize = 0;
        auto files = getFiles(p, filePattern, recursionDepth);
        for (auto file : files)
        {
            auto x = boost::filesystem::file_size(file);
            totalSize += boost::filesystem::file_size(file);
        }

        return totalSize;
    };

    bool makeDirectory(const boost::filesystem::path& p)
    {
        if (p.has_parent_path())
        {
            if (!makeDirectory(p.parent_path()))
            {
                return false;
            }
        }

        if (!boost::filesystem::exists(p))
        {
            return boost::filesystem::create_directory(p);
        }
        return true;
    }
}}}

#endif