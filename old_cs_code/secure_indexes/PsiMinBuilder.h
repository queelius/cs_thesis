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
            uint32_t falsePositiveBits;
            int verbose;
            std::string ref;
            double loadFactor;
            cmph_cpp::CMPH_ALGO algo;
            uint32_t maxMinPairDist;

            std::unordered_set<std::string> ignoreWords;
            std::vector<std::string> secrets;
            bool stemming;

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
                maxMinPairDist = alex::globals::max_min_pair_dist;
                falsePositiveRate(alex::globals::default_fp_rate);
                algo = alex::globals::default_ph;
                loadFactor = alex::globals::default_load_factor;
                verbose = alex::globals::default_verbose;
                secrets = alex::globals::default_secrets;
                ignoreWords = alex::globals::default_stop_words;
            };
        };

        void dump()
        {
            std::cout << "maxMinPairDist: " << _params.maxMinPairDist << std::endl;
            std::cout << "maxDist: " << this->_maxMinDist << std::endl;
            for (auto x : this->_minPairs)
            {
                std::cout << x.first << ", " << x.second << std::endl;
            }
        };

        PsiMinBuilder(const BuilderParams& params) : _params(params), _maxMinDist(0)
        {
            _ref = params.ref;
            if (_params.falsePositiveBits < 1)
                _params.falsePositiveBits = 1;
        };

        SecureIndex build()
        {
            auto f = stochastic::Entropy<std::minstd_rand0>();
            char** terms = new char*[_minPairs.size()];
            unsigned int i = 0;

            for (const auto& p : _minPairs)
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

            //std::cerr << "size n: " << sz << std::endl;
            //std::cerr << "ref: " << _ref << std::endl;

            psi->_hashWidth = _params.falsePositiveBits;
            psi->_maxHash = (1 << _params.falsePositiveBits);
            psi->_hashBits.resizeBits(sz * psi->_hashWidth);
            psi->_ref = _ref;

            if (_maxMinDist <= 1)
            {
                psi->_distWidth = 1;
                psi->_distBits.resizeBits(sz);
            }
            else
            {
                psi->_distWidth = (uint32_t)std::ceil(std::log(_maxMinDist) / std::log(2));
                psi->_distThreshold = _params.maxMinPairDist;
                psi->_distBits.resizeBits(sz * psi->_distWidth);
            }

            //std::cerr << "hash width: " << psi->_hashWidth << std::endl;
            //std::cerr << "dist thresh" << psi->_distThreshold << std::endl;
            //std::cerr << "dist width: " << psi->_distWidth << std::endl;

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
            for (auto w : _words)
            {
                if (_terms.count(w) == 0)
                {
                    auto crypts = alex::utils::cryptoTerms(w, _params.secrets, _ref);
                    _terms[w].reserve(crypts.size());
                    for (auto crypt : crypts)
                        _terms[w].push_back(std::move(crypt));

                    //std::cerr << "_term[" << w << "]: " << _terms[w].size() << std::endl;
                }
            }

            //std::cerr << "_terms: " << _terms.size() << std::endl;
            //std::cerr << "num sec: " << _params.secrets.size() << std::endl;
            //std::cerr << "max min pair dist: " << _params.maxMinPairDist << std::endl;
            //std::cerr << "words: " << _words.size() << std::endl;

            auto minPairDist = alex::utils::findMinPairs(_words, _params.maxMinPairDist);

            //std::cerr << "minpairdist: " << minPairDist.size() << std::endl;

            for (const auto& x : minPairDist)
            {
                const auto& hs1 = _terms[x.first.X()];
                const auto& hs2 = _terms[x.first.Y()];

                const uint32_t minDist = x.second;
                _maxMinDist = std::max(minDist, _maxMinDist);

                for (const auto& h1 : hs1)
                {
                    for (const auto& h2 : hs2)
                    {
                        auto hash = alex::crypt::hexdigest<alex::globals::digest_size>(h1 < h2 ? h1 + "|" + h2 : h2 + "|" + h1);
                        _minPairs.push_back(std::make_pair(std::move(hash), minDist));
                    }
                }
            }

            //std::cerr << "minpairs: " << _minPairs.size() << std::endl;
        };

        void parse(const boost::filesystem::path& file)
        {
            if (!boost::filesystem::is_regular_file(file))
            {
                throw std::exception(("Not A File: \"" + file.string() + "\"").c_str());
            }
            parse(file.string(), std::ifstream(file.string()));
        };

        boost::filesystem::path getPreferredFileExtension() const { return PsiMin::HEADER; };

        void printWords(std::ostream& outs = std::cout)
        {
            std::cout << "Reference: " << _ref << std::endl;
            for (auto& word : _words)
                std::cout << "\t" << word << std::endl;
        };

        void printTerms(std::ostream& outs = std::cout)
        {
            outs << "Min Pairs" << std::endl;
            for (auto p : _minPairs)
            {
                outs << "\tHash: " << p.first << std::endl;
                outs << "\tDist: " << p.second << std::endl;
            }
        };

        void parse(const std::string& ref, std::istream& file)
        {
            _ref = ref;
            std::string word;
            char ch;

            while(file.good())
            {
                ch = file.get();
                if(isalnum(ch))
                {
                    word += tolower(ch);
                }
                else if (ch == '\'')
                {
                    // skip apostrophe's, but don't let them end the current word
                }
                else
                {
                    if (!word.empty())
                    {
                        if (_params.ignoreWords.count(word) == 0)
                        {
                            if(_params.stemming)
                                porter2stemmer::stem(word);
                            _words.push_back(word);
                        }
                        word = "";
                    }
                }
            }
        };

    private:
        BuilderParams _params;
        uint32_t _maxMinDist;
        std::vector<std::pair<std::string, uint32_t>> _minPairs;
        std::string _ref;
        std::vector<std::string> _words;

        std::unordered_map<std::string, std::vector<std::string>> _terms;
    };
}}

#endif