#ifndef __PSI_MIN_BUILDER_H__
#define __PSI_MIN_BUILDER_H__

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include "Globals.h"
#include "PsiFreqBuilder.h"
#include "PsiMin.h"
#include "Utils.h"
#include "BitArray.h"
#include <boost/filesystem.hpp>
#include "ISecureIndex.h"

namespace alex { namespace index
{
    class PsiMinBuilder
    {
    public:
        struct BuilderParams
        {
            PsiFreqBuilder::BuilderParams freqParams;
            int verbose;
            uint32_t minPairFalsePositiveBits;
            double minPairLoadFactor;
            cmph_cpp::CMPH_ALGO minPairAlgo;
            uint32_t maxMinPairMaxDist;
            // ignore regular expression pattern (list?)

            void minPairFalsePositiveRate(double rate)
            {
                if(rate < 0 || rate > 1)
                    throw std::exception("False Positive Rate Must Be In Range [0, 1]");
                minPairFalsePositiveBits = (uint32_t)std::ceil(-std::log(rate) / std::log(2));
            };

            BuilderParams()
            {
                defaults();
            };

            void defaults()
            {
                maxMinPairMaxDist = alex::globals::max_min_pair_dist;
                minPairAlgo = alex::globals::default_ph;
                minPairLoadFactor = alex::globals::default_load_factor;
                verbose = alex::globals::default_verbose;
                minPairFalsePositiveRate(alex::globals::default_fp_rate);
                freqParams.defaults();
            };
        };

        void dump()
        {
            // finish this
        };

        PsiMinBuilder(const BuilderParams& params) : _params(params), _maxDist(0) {};

        // TODO: let block 0 be a meta-data block, no location property -> title, author, manually added keywords
        SecureIndex build()
        {
            auto f = stochastic::Entropy<std::minstd_rand0>();
            char** terms = new char*[_minPairs.size()];
            unsigned int i = 0;
            for (auto p : _minPairs)
            {
                terms[i] = new char[alex::globals::digest_size+1];
                std::memcpy(terms[i], p.first.c_str(), alex::globals::digest_size);
                terms[i][alex::globals::digest_size] = 0;
                ++i;
            }

            auto psi = new PsiMin();
            uint32_t sz = psi->_phMin.build(terms, _minPairs.size(),
                                            _params.verbose, _params.minPairLoadFactor,
                                            _params.minPairAlgo);

            //psi->_freq = _freqBuilder.buildAlternative();
            psi->_freq = std::move(*((PsiFreq*)_freqBuilder.build().get()));
            psi->_hashWidth = _params.minPairFalsePositiveBits;
            psi->_maxHash = (1 << _params.minPairFalsePositiveBits);
            psi->_hashBits.resizeBits(sz * _params.minPairFalsePositiveBits);
            // psi->_hashBits.resizeBits(sz * _params.falsePositiveBits + f.getInt(0, _params.hashBitsRandomPadding))
            psi->_distWidth = (uint32_t)std::ceil(std::log(_maxDist) / std::log(2));
            psi->_distBits.resizeBits(sz * psi->_distWidth);
            
            // randomize bit data -- only useful if using a non-minimal perfect hashing
            // to obfuscate contents
            for(unsigned int i = 0; i < psi->_hashBits.bytes(); ++i)
                psi->_hashBits.setByte(i, f.get() % psi->MAX_BYTE);
            for(unsigned int i = 0; i < psi->_distBits.bytes(); ++i)
                psi->_distBits.setByte(i, f.get() % psi->MAX_BYTE);

            for (const auto& p : _minPairs)
            {
                const uint32_t index = psi->_phMin.search(p.first);
                const uint32_t value = alex::hash::jenkinsHash(p.first) % psi->_maxHash;
                psi->_hashBits.set(psi->_hashWidth * index, value, psi->_hashWidth);
                psi->_distBits.set(psi->_distWidth * index, p.second, psi->_distWidth);
            }

            for (unsigned int i = 0; i < _minPairs.size(); ++i)
                delete [] terms[i];
            delete [] terms;

            return SecureIndex(psi);
        };

        void makeTerms()
        {
            _freqBuilder.makeTerms();

            auto minPairDist = alex::utils::findMinPairs(_freqBuilder._words, _params.maxMinPairMaxDist);
            for (auto x : minPairDist)
            {
                const auto& hs1 = _freqBuilder._terms[x.first.X()].hash;
                const auto& hs2 = _freqBuilder._terms[x.first.Y()].hash;

                const uint32_t minDist = x.second;
                _maxDist = std::max(minDist, _maxDist);

                for (const auto& h1 : hs1)
                {
                    for (const auto& h2 : hs2)
                    {
                        auto hash = alex::crypt::hexdigest<alex::globals::digest_size>(h1 < h2 ? h1 + "|" + h2 : h2 + "|" + h1);
                        _minPairs.push_back(std::make_pair(std::move(hash), minDist));
                    }
                }
            }
        };

        void parse(const boost::filesystem::path& file)
        {
            if (!boost::filesystem::is_regular_file(file))
            {
                throw std::exception(("Not A File: \"" + file.string() + "\"").c_str());
            }
            parse(file.string(), std::ifstream(file.string()));
        };

        void parse(const std::string& ref, std::istream& file)
        {
            _freqBuilder.parse(ref, file);
        }

        boost::filesystem::path getPreferredFileExtension() const { return PsiMin::HEADER; };

        void printWords(std::ostream& outs = std::cout)
        {
            _freqBuilder.printWords();
        };

        void printTerms(std::ostream& outs = std::cout)
        {
            outs << "Freq Terms" << std::endl;
            _freqBuilder.printTerms();

            outs << "Min Pairs" << std::endl;
            for (auto p : _minPairs)
            {
                outs << "Hash: " << p.first << std::endl;
                outs << "Dist: " << p.second << std::endl;
            }
        };

    private:
        BuilderParams _params;
        uint32_t _maxDist;
        PsiFreqBuilder _freqBuilder;
        std::vector<std::pair<std::string, uint32_t>> _minPairs;
    };
}}

#endif