#ifndef __SECURE_INDEX_DB_H__
#define __SECURE_INDEX_DB_H__

#include <iostream>
#include <string>
#include <thread>
#include <future>
#include "FileSystem.h"
#include "BsiBlock.h"
#include "ISecureIndex.h"
#include "PsiFreq.h"
#include "PsiBlock.h"
#include "PsiPost.h"
#include "PsiMin.h"
#include "HiddenQuery.h"
#include <boost/program_options.hpp>

namespace alex { namespace index
{
    class SecureIndexDb
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

        void clear()
        {
            _secureIndexes.clear();
        };

        SecureIndexDb() {};
        SecureIndexDb(const Params& params) : _params(params) {};

        unsigned int documentFrequency(const HiddenQuery::HiddenTerm& t)
        {
            unsigned int docFreq = 0;
            for (const auto& psi : _secureIndexes)
            {
                if (psi->contains(t))
                    ++docFreq;
            }
            return docFreq;
        };
        std::unordered_map<SecureIndex, double> bm25(const HiddenQuery& q)
        {
            if (q.terms.empty())
                throw std::exception("No Terms");

            double avgDL = 0;
            for (const auto& si : _secureIndexes)
                avgDL += si->getApproximateWords();
            avgDL /= _secureIndexes.size();
            const double N = _secureIndexes.size();

            std::unordered_map<SecureIndex, double> totals;
            for (const auto& t : q.terms)
            {
                const double df = documentFrequency(t);
                const double idf = std::max(std::log((N - df + 0.5) / (df + 0.5)), 0.05);
                for (const auto& si : _secureIndexes)
                {
                    const unsigned int tf = si->getFrequency(t);
                    const double other = tf * (_params.bm25_k1 + 1) /
                        (tf + _params.bm25_k1 * (1 - _params.bm25_b +
                        _params.bm25_b * (si->getApproximateWords() / avgDL)));
                    totals[si] += other * idf;
                }
            }
            return totals;
        };

        // minimum pair-wise distance
        std::unordered_map<SecureIndex, double> minPairwiseDistanceScore(const HiddenQuery& q)
        {
            if (q.terms.empty())
                throw std::exception("No Terms");

            std::unordered_map<SecureIndex, double> minDist;
            for (const auto& si : _secureIndexes)
            {
                if (q.terms.size() == 1)
                {
                    // no min pairwise score to evaluate,
                    // so short-circuit.
                    minDist[si] = 0;
                }
                // let's always let the secure index take care of it -- even if it doesn't have
                // a min pair wise information, it can choose to calculate an expected
                // value.
                else // else if (si->hasProximity())
                {
                    minDist[si] = std::log(_params.alpha_min_pair + std::exp(-_params.beta_min_pair * si->getMinPairwiseScore(q)));
                }
                /*else
                {
                    minDist[si] = std::log(_params.alpha_min_pair);
                }*/
            }

            return minDist;
        };

        // minimum pair-wise distance
        std::unordered_map<SecureIndex, std::map<std::pair<HiddenQuery::HiddenTerm, HiddenQuery::HiddenTerm>, int>>
            minPairwiseDistances(const HiddenQuery& q)
        {
            if (q.terms.size() < 2)
                throw std::exception("Insufficient Terms");

            std::unordered_map<SecureIndex, std::map<std::pair<HiddenQuery::HiddenTerm, HiddenQuery::HiddenTerm>, int>> minDist;
            for (const auto& si : _secureIndexes)
            {
                if (si->hasProximity())
                {
                    minDist[si] = si->getMinPairWiseDistances(q);
                }
            }
            return minDist;
        };

        std::vector<std::pair<SecureIndex, std::vector<uint32_t>>> getLocations(const HiddenQuery::HiddenTerm& t)
        {
            std::vector<std::pair<SecureIndex, std::vector<uint32_t>>> locations;
            for (SecureIndex idx : _secureIndexes)
            {
                auto locs = idx->getLocations(t);
                if (!locs.empty())
                    locations.push_back(std::make_pair(idx, std::move(locs)));
            }
            return locations;
        }
        std::vector<std::pair<SecureIndex, unsigned int>> getFrequencies(const HiddenQuery::HiddenTerm& t)
        {
            std::vector<std::pair<SecureIndex, unsigned int>> results;
            for (const auto& si : _secureIndexes)
                results.push_back(std::make_pair(si, si->getFrequency(t)));

            std::sort(results.begin(), results.end(),
                      [](const std::pair<SecureIndex, unsigned int>& x1,
                         const std::pair<SecureIndex, unsigned int>& x2)
                        { return x2 < x1; });

            return results;
        };
        std::vector<std::pair<SecureIndex, double>> getRanking(const HiddenQuery& hq)
        {
            if (hq.terms.empty())
                throw std::exception("No Terms");

            std::vector<std::pair<SecureIndex, double>> results;
            results.reserve(_secureIndexes.size());

            auto termRel = bm25(hq);
            if (hq.terms.size() == 1)
            {
                for (const auto& si : _secureIndexes)
                    results.push_back(std::make_pair(si, termRel[si]));
            }
            else
            {
                for (const auto& si : _secureIndexes)
                {
                    if (si->hasProximity())
                    {
                        results.push_back(std::make_pair(
                            si, alex::globals::alpha_weighted_avg * termRel[si] +
                            (1 - alex::globals::alpha_weighted_avg) * si->getMinPairwiseScore(hq)));
                    }
                    else
                        results.push_back(std::make_pair(si, termRel[si]));
                }
            }

            std::sort(results.begin(), results.end(),
                [](const std::pair<SecureIndex, double>& x1,
                   const std::pair<SecureIndex, double>& x2)
                { return x2.second < x1.second; });

            return results;
        };
        std::vector<SecureIndex> containsAll(const HiddenQuery& q, bool useConcurrency = true)
        {
            std::vector<SecureIndex> results;
            if (useConcurrency)
                results = containsAllHelper(q, 0, _secureIndexes.size());
            else
            {
                for (const auto& si : _secureIndexes)
                {
                    if (si->containsAll(q))
                        results.push_back(si);
                }
            }
            return results;
        };
        std::vector<SecureIndex> containsAny(const HiddenQuery& q, bool useConcurrency = true)
        {
            std::vector<SecureIndex> results;
            if (useConcurrency)
                results = containsAnyHelper(q, 0, _secureIndexes.size());
            else
            {
                for (const auto& si : _secureIndexes)
                {
                    if (si->containsAny(q))
                        results.push_back(si);
                }
            }
            return results;
        };
        std::vector<SecureIndex> retrieve(const std::string& rx)
        {
            std::vector<SecureIndex> results;
            for (const auto& si : _secureIndexes)
            {
                if (alex::str::hasMatch(si->getReference(), rx))
                    results.push_back(si);
            }
            return results;
        };
        std::vector<SecureIndex> retrieveAll() const { return _secureIndexes; };
        size_t numIndexes() const { return _secureIndexes.size(); };

        void write(const boost::filesystem::path& outDirectory)
        {
            for (const auto& si : _secureIndexes)
            {
                boost::filesystem::path outfile;
                outfile += outDirectory;
                outfile += outfile.preferred_separator;
                outfile += si->getReference();

                outfile.replace_extension("");
                boost::filesystem::path tmp(outfile);
                tmp.replace_extension(si->getPreferredFileExtension());
                tmp.normalize();
                unsigned int append = 0;
                while (boost::filesystem::exists(tmp))
                {
                    tmp = outfile;
                    tmp += "_" + std::to_string(++append);
                    tmp.replace_extension(si->getPreferredFileExtension());
                    tmp.normalize();
                }
                        
                outfile = tmp;
                alex::io::filesystem::makeDirectory(outfile.parent_path());
                si->write(outfile);
            }
        };

        size_t read(const std::vector<boost::filesystem::path>& files)
        {
            _secureIndexes.reserve(_secureIndexes.size() + files.size());
            for (const auto& file : files)
            {
                try
                {
                    SecureIndex idx = nullptr;
                    const std::string& hdr = io::binary::readHeader(file);

                    if (hdr == PsiBlock::HEADER)
                        idx = SecureIndex(new PsiBlock);
                    else if (hdr == PsiFreq::HEADER)
                        idx = SecureIndex(new PsiFreq);
                    else if (hdr == PsiPost::HEADER)
                        idx = SecureIndex(new PsiPost);
                    else if (hdr == PsiMin::HEADER)
                        idx = SecureIndex(new PsiMin);
                    else if (hdr == BsiBlock::HEADER)
                        idx = SecureIndex(new BsiBlock);
                    else
                        std::cout << "Unrecognized header \"" << hdr << "\" -- skipping " << file << "." << std::endl;

                    if (idx != nullptr)
                    {
                        idx->read(file);
                        _secureIndexes.push_back(idx);
                    }
                    else
                    {
                        std::cout << "Recognized header \"" << hdr << "\" in file " << file << " was not loaded for unknown reasons." << std::endl;
                    }
                }
                catch (const std::bad_alloc&)
                {
                    std::cout << "Error: Cannot load any more indexes due to memory limitations." << std::endl;
                    _secureIndexes.shrink_to_fit();
                    return _secureIndexes.size();
                }
                catch (const std::exception& e)
                {
                    std::cout << "Error processing file " << file << ": " << e.what() << std::endl;
                }
            }
            return _secureIndexes.size();
        }

        size_t read(const boost::filesystem::path& inDirectory,
                   const std::string& inFilePattern = ".*", size_t recursionDepth = std::numeric_limits<size_t>::max())
        {
            return read(alex::io::filesystem::getFiles(inDirectory, inFilePattern, recursionDepth));
        };

    private:
        Params _params;
        std::vector<SecureIndex> _secureIndexes;

        std::vector<SecureIndex> containsAllHelper(const HiddenQuery& q, int start, int end)
        {
            if (end - start < 20)
            {
                std::vector<SecureIndex> sis;
                for (int i = start; i < end; ++i)
                {
                    if (_secureIndexes[i]->containsAll(q))
                        sis.push_back(_secureIndexes[i]);
                }
                return sis;
            }
            else
            {
                int mid = (end + start) / 2;
                auto& results1 = std::async(&SecureIndexDb::containsAllHelper, this, q, start, mid);
                auto& results2 = containsAllHelper(q, mid, end);
                auto& x = results1.get();
                results2.insert(results2.end(), x.begin(), x.end());
                return results2;
            }
        };

        std::vector<SecureIndex> containsAnyHelper(const HiddenQuery& q, int start, int end)
        {
            if (end - start < 20)
            {
                std::vector<SecureIndex> sis;
                for (int i = start; i < end; ++i)
                {
                    if (_secureIndexes[i]->containsAny(q))
                        sis.push_back(_secureIndexes[i]);
                }
                return sis;
            }
            else
            {
                int mid = (end + start) / 2;
                auto& results1 = std::async(&SecureIndexDb::containsAnyHelper, this, q, start, mid);
                auto& results2 = containsAnyHelper(q, mid, end);
                auto& x = results1.get();
                results2.insert(results2.end(), x.begin(), x.end());
                return results2;
            }
        };
    };
}}

#endif