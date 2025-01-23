#ifndef __INVERTED_INDEX_DB_H__
#define __INVERTED_INDEX_DB_H__

#include "InvertedIndex.h"
#include <vector>
#include <iostream>
#include <string>
#include <list>
#include <cstdint>
#include "Query.h"
#include "FileSystem.h"
#include "BinaryIO.h"
#include "Str.h"
#include "Utils.h"
#include <unordered_map>
#include <thread>
#include <future>
#include <map>

namespace alex { namespace index
{
    class InvertedIndexDb
    {
    public:
        struct Params
        {
            double bm25_k1;
            double bm25_b;
            double alpha_min_pair;
            double beta_min_pair;
            double alpha_weighted_avg;

            Params()
            {
                defaults();
            };

            void defaults()
            {
                bm25_k1 = alex::globals::bm25_k1;
                bm25_b = alex::globals::bm25_b;
                alpha_min_pair = alex::globals::alpha_min_pair;
                beta_min_pair = alex::globals::beta_min_pair;
                alpha_weighted_avg = alex::globals::alpha_weighted_avg;
            };
        };

        InvertedIndexDb(const Params& params) : _params(params) {};

        std::vector<std::pair<Index, std::vector<uint32_t>>> getLocations(const Query::Term& t)
        {
            std::vector<std::pair<Index, std::vector<uint32_t>>> locations;
            for (Index idx : _indexes)
            {
                auto locs = idx->getLocations(t);
                if (!locs.empty())
                    locations.push_back(std::make_pair(idx, locs));
            }
            return locations;
        };

        unsigned int documentFrequency(const Query::Term& t)
        {
            unsigned int docFreq = 0;
            for (const auto& idx : _indexes)
            {
                if (idx->contains(t))
                    ++docFreq;
            }
            return docFreq;
        };

        std::unordered_map<Index, double> bm25(const Query& q)
        {
            if (q.terms.empty())
                throw std::exception("No Terms");

            double avgDL = 0;
            for (const auto& idx : _indexes)
                avgDL += idx->getWordCount();
            avgDL /= _indexes.size();

            const double N = _indexes.size();

            std::unordered_map<Index, double> totals;
            for (const auto& t : q.terms)
            {
                const double df = documentFrequency(t);
                const double idf = std::max(std::log((N - df + 0.5) / (df + 0.5)), 0.05);
                for (const auto& idx : _indexes)
                {
                    const unsigned int tf = idx->getFrequency(t);
                    const double other = tf * (_params.bm25_k1 + 1) / (tf + _params.bm25_k1 * (1 - _params.bm25_b + _params.bm25_b * (idx->getWordCount() / avgDL)));
                    totals[idx] += other * idf;
                }
            }
            return totals;
        };

        std::unordered_map<Index, std::map<std::pair<Query::Term, Query::Term>, int>>
            minPairwiseDistances(const Query& q)
        {
            if (q.terms.size() < 2)
                throw std::exception("Insufficient Terms");

            std::unordered_map<Index, std::map<std::pair<Query::Term, Query::Term>, int>> minDist;
            for (const auto& si : _indexes)
            {
                minDist[si] = si->getMinPairWiseDistances(q);
            }
            return minDist;
        };

        // minimum pair-wise distance
        std::unordered_map<Index, double> minPairwiseDistanceScore(const Query& q)
        {
            if (q.terms.empty())
                throw std::exception("No Terms");

            std::unordered_map<Index, double> minDist;
            for (const auto& idx : _indexes)
            {
                if (q.terms.size() == 1)
                {
                    minDist[idx] = 0;
                }
                else
                {
                    minDist[idx] = std::log(_params.alpha_min_pair + std::exp(-_params.beta_min_pair * idx->getMinPairwiseScore(q)));
                }
            }

            return minDist;
        };

        std::vector<std::pair<Index, unsigned int>> getFrequencies(const Query::Term& t)
        {
            std::vector<std::pair<Index, unsigned int>> results;
            for (const auto& idx : _indexes)
                results.push_back(std::make_pair(idx, idx->getFrequency(t)));

            std::sort(results.begin(), results.end(),
                      [](const std::pair<Index, unsigned int>& x1,
                         const std::pair<Index, unsigned int>& x2)
                        { return x2 < x1; });

            return results;
        };

        std::unordered_map<Index, double> getRanking2(const Query& q)
        {
            if (q.terms.empty())
                throw std::exception("No Terms");

            std::unordered_map<Index, double> results;

            auto termRel = bm25(q);
            if (q.terms.size() == 1)
            {
                for (const auto& idx : _indexes)
                    results[idx] = termRel[idx];
            }
            else
            {
                for (const auto& idx : _indexes)
                {
                    results[idx] = _params.alpha_weighted_avg * termRel[idx] + (1 - _params.alpha_weighted_avg) * idx->getMinPairwiseScore(q);
                }
            }

            return results;
        };

        std::vector<std::pair<Index, double>> getRanking(const Query& q)
        {
            if (q.terms.empty())
                throw std::exception("No Terms");

            std::vector<std::pair<Index, double>> results;
            results.reserve(_indexes.size());

            auto termRel = bm25(q);
            if (q.terms.size() == 1)
            {
                for (const auto& idx : _indexes)
                    results.push_back(std::make_pair(idx, termRel[idx]));
            }
            else
            {
                for (const auto& idx : _indexes)
                {
                    results.push_back(std::make_pair(
                        idx, _params.alpha_weighted_avg * termRel[idx] +
                        (1 - _params.alpha_weighted_avg) * idx->getMinPairwiseScore(q)));
                }
            }

            std::sort(results.begin(), results.end(),
                [](const std::pair<Index, double>& x1,
                   const std::pair<Index, double>& x2)
                { return x2.second < x1.second; });

            return results;
        };
        std::vector<Index> containsAll(const Query& q, bool useConcurrency = true)
        {
            std::vector<Index> results;
            if (useConcurrency)
                results = containsAllHelper(q, 0, _indexes.size());
            else
            {
                for (const auto& idx : _indexes)
                {
                    if (idx->containsAll(q))
                        results.push_back(idx);
                }
            }

            return results;
        };
        std::vector<Index> containsAny(const Query& q)
        {
            std::vector<Index> results;
            for (const auto& idx : _indexes)
            {
                if (idx->containsAny(q))
                    results.push_back(idx);
            }
            return results;
        };
        std::vector<Index> retrieve(const std::string& rx)
        {
            std::vector<Index> results;
            for (const auto& idx : _indexes)
            {
                if (alex::str::hasMatch(idx->getReference(), rx))
                    results.push_back(idx);
            }
            return results;
        };
        std::vector<Index> retrieveAll() const { return _indexes; };
        size_t numIndexes() const { return _indexes.size(); };

        void write(const boost::filesystem::path& outDirectory)
        {
            for (const auto& idx : _indexes)
            {
                boost::filesystem::path outfile;
                outfile += outDirectory;
                outfile += outfile.preferred_separator;
                outfile += idx->getReference();

                outfile.replace_extension("");
                boost::filesystem::path tmp(outfile);
                tmp.replace_extension(idx->getPreferredFileExtension());
                tmp.normalize();
                unsigned int append = 0;
                while (boost::filesystem::exists(tmp))
                {
                    tmp = outfile;
                    tmp += "-" + std::to_string(++append);
                    tmp.replace_extension(idx->getPreferredFileExtension());
                    tmp.normalize();
                }
                        
                outfile = tmp;
                alex::io::filesystem::makeDirectory(outfile.parent_path());
                idx->write(outfile);
            }
        };
        void read(const std::vector<boost::filesystem::path>& files)
        {
            for (const auto& file : files)
            {
                try
                {
                    Index idx = nullptr;
                    const std::string& hdr = io::binary::readHeader(file);
                    if (hdr == InvertedIndex::HEADER)
                        idx = Index(new InvertedIndex);
                    else
                        std::cout << "* Skipping file " << file << "." << std::endl;

                    if (idx)
                    {
                        idx->read(file);
                        _indexes.push_back(idx);
                    }
                }
                catch (const std::exception& e)
                {
                    std::cout << "Error Reading File " << file << " Into IndexDb: " << e.what() << std::endl;
                }
            }
        };

        void read(const boost::filesystem::path& inDirectory,
                   const std::string& inFilePattern = ".*", size_t recursionDepth = std::numeric_limits<size_t>::max())
        {
            read(alex::io::filesystem::getFiles(inDirectory, inFilePattern, recursionDepth));
        };

    private:
        std::vector<Index> _indexes;
        Params _params;

        std::vector<Index> containsAllHelper(const Query& q, int start, int end)
        {
            if (end - start < 20)
            {
                std::vector<Index> idx;
                for (int i = start; i < end; ++i)
                {
                    if (_indexes[i]->containsAll(q))
                        idx.push_back(_indexes[i]);
                }
                return idx;
            }
            else
            {
                int mid = (end + start) / 2;
                auto& results1 = std::async(&InvertedIndexDb::containsAllHelper, this, q, start, mid);
                auto& results2 = containsAllHelper(q, mid, end);
                auto& x = results1.get();
                results2.insert(results2.end(), x.begin(), x.end());
                return results2;
            }
        };
    };
}}

#endif