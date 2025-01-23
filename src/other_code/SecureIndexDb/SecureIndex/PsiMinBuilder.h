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
            uint32_t maxFreq;
            uint32_t falsePositiveBits;
            int verbose;
            std::string ref;
            double loadFactor;
            cmph_cpp::CMPH_ALGO algo;
            uint32_t maxMinPairMaxDist;
            std::unordered_set<std::string> ignoreWords;
            std::vector<std::string> secrets;
            bool stemming;
            // ignore regular expression pattern (list?)

            void falsePositiveRate(double rate)
            {
                if(rate < 0 || rate > 1)
                    throw std::exception("False Positive Rate Must Be In Range [0, 1]");
                falsePositiveBits = (uint32_t)std::ceil(-std::log(rate) / std::log(2));
            };

            BuilderParams()
            {
                defaults();
            };

            void defaults()
            {
                maxMinPairMaxDist = alex::globals::max_min_pair_dist;
                falsePositiveRate(alex::globals::default_fp_rate);
                algo = alex::globals::default_ph;
                loadFactor = alex::globals::default_load_factor;
                verbose = alex::globals::default_verbose;
                secrets = alex::globals::default_secrets;
                maxFreq = std::numeric_limits<uint32_t>::max();
                ignoreWords = alex::globals::default_stop_words;
            };
        };

        void dump()
        {
            _freqBuilder.dump();
            std::cout << "maxMinPairMaxDist: " << _params.maxMinPairMaxDist << std::endl;
            std::cout << "maxDist: " << this->_maxDist << std::endl;
            for (auto x : this->_minPairs)
            {
                std::cout << x.first << ", " << x.second << std::endl;
            }
        };

        PsiMinBuilder(const BuilderParams& params) : _params(params), _maxDist(0)
        {
            this->_freqBuilder._ref = params.ref;
            this->_freqBuilder._maxFreq = 0;
            this->_freqBuilder._params.algo = params.algo;
            this->_freqBuilder._params.verbose = params.verbose;
            this->_freqBuilder._params.falsePositiveBits = params.falsePositiveBits;
            this->_freqBuilder._params.ignoreWords = params.ignoreWords;
            this->_freqBuilder._params.loadFactor = params.loadFactor;
            this->_freqBuilder._params.maxFreq = params.maxFreq;
            this->_freqBuilder._params.secrets = params.secrets;
            this->_freqBuilder._params.stemming = params.stemming;
        };

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
                                            _params.verbose, _params.loadFactor,
                                            _params.algo);
            psi->_freq = _freqBuilder.build();
            psi->_hashWidth = _params.falsePositiveBits;
            psi->_maxHash = (1 << _params.falsePositiveBits);
            psi->_hashBits.resizeBits(sz * _params.falsePositiveBits);
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