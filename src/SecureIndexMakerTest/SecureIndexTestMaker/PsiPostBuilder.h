#ifndef __PSI_POST_BUILDER_H__
#define __PSI_POST_BUILDER_H__

#include <string>
#include <unordered_set>
#include <unordered_map>
#include "PsiPost.h"
#include "BitArray.h"
#include "ISecureIndex.h"
#include <boost/filesystem.hpp>
#include "DiscreteDistribution.h"
#include "RandomVariates.h"

namespace alex { namespace index
{
    namespace psi_post_builder_internal
    {
        uint32_t triangular(float a, float b, float c, float p)
        {
            if (a < 0 || a >= b || a > c || c > b)
                throw std::exception("Invalid Argument: required => a < b, a <= c, c <= b");

            const float Fc = (c - a) / (b - a);
            const float F1 = std::sqrt((b - a) * (c - a));
            const float F2 = std::sqrt((b - a) * (b - c));

            return (uint32_t)std::roundf(p < Fc ?
                a + std::sqrt(p) * F1 : b - std::sqrt(1 - p) * F2);
        }
    };

    class PsiPostBuilder
    {
    public:
        struct BuilderParams
        {
            std::unordered_set<std::string> ignoreWords;
            bool stemming;
            uint32_t falsePositiveBits;
            vector<std::string> secrets;
            uint32_t offsetRadius;
            // ignore regular expression pattern (list?)
            int verbose;
            double loadFactor;
            cmph_cpp::CMPH_ALGO algo;
            double freqError;

            BuilderParams()
            {
                defaults();
            };

            void falsePositiveRate(double rate)
            {
                if(rate < 0 || rate > 1)
                    throw std::exception("False Positive Rate Must Be In Range [0, 1]");
                falsePositiveBits = (uint32_t)std::ceil(-std::log(rate) / std::log(2));
            };

            void defaults()
            {
                freqError = 0;
                algo = alex::globals::default_ph;
                loadFactor = alex::globals::default_load_factor;
                verbose = alex::globals::default_verbose;
                offsetRadius = alex::globals::default_post_range;
                secrets = alex::globals::default_secrets;
                falsePositiveRate(alex::globals::default_fp_rate);
                stemming = alex::globals::default_stemming;
                ignoreWords = alex::globals::default_stop_words;
            }
        };

        PsiPostBuilder(const BuilderParams& params) :
            _params(params) {};

        PsiPostBuilder()
        {
            _params.defaults();
        }
        
        void dump()
        {
            std::cout << "ref: " << _ref << std::endl;
            std::cout << "words.size(): " << _words.size() << std::endl;
            std::cout << "maxOffset: " << _params.offsetRadius << std::endl;
        };

        // TODO: let block 0 be a meta-data block, no location property -> title, author, manually added keywords
        SecureIndex build()
        {
            if (_words.empty())
                throw std::exception("No Terms");

            auto f = stochastic::Entropy<std::minstd_rand0>();

            char** terms = new char*[_terms.size() * _params.secrets.size()];
            unsigned int i = 0;
            for (auto& x = _terms.begin(); x != _terms.end(); ++x)
            {
                for (auto& hash : x->second.hash)
                {
                    terms[i] = new char[alex::globals::digest_size+1];
                    std::memcpy(terms[i], hash.c_str(), alex::globals::digest_size);
                    terms[i][alex::globals::digest_size] = 0;
                    ++i;
                }
            }

            auto psi = new PsiPost();
            uint32_t sz = psi->_ph.build(terms, (_terms.size() * _params.secrets.size()), _params.verbose, _params.loadFactor, _params.algo);
            psi->_ref = _ref;
            psi->_approximateWords = _words.size();
            psi->_maxOffset = _params.offsetRadius;
            psi->_numPostings = sz;
            psi->_postings.resize(sz);
            psi->_hashWidth = _params.falsePositiveBits;
            psi->_maxHash = (1 << _params.falsePositiveBits);
            psi->_hashBits.resizeBits(sz * _params.falsePositiveBits);

            if (_params.loadFactor < 1)
            {
                // poison hash bits -- randomize it. the actual entries will
                // be inserted next, thus overwriting these when necessary.
                for(unsigned int i = 0; i < psi->_hashBits.bytes(); ++i)
                    psi->_hashBits.setByte(i, f.get() % psi->MAX_BYTE);

                unsigned int maxFreq = 0;
                std::vector<bool> used(sz, false);
                for (auto& term : _terms)
                {
                    for (const auto& hash : term.second.hash)
                    {
                        maxFreq = std::max(maxFreq, term.second.posting.size());
                        const uint32_t index = psi->_ph.search(hash);
                        const uint32_t value = alex::hash::jenkinsHash(hash) % psi->_maxHash;
                        psi->_hashBits.set(psi->_hashWidth * index, value, psi->_hashWidth);
                        psi->_postings[index] = term.second.posting;
                        used[index] = true;
                    }
                }

                // poison unused indexes with fake postings lists.
                for(unsigned int i = 0; i < sz; ++i)
                {
                    if (!used[i])
                    {
                        unsigned int k = psi_post_builder_internal::triangular(1, maxFreq, 1, f.get0_1f());
                        psi->_postings[i].reserve(k);
                        for (unsigned int j = 0; j < k; ++j)
                            psi->_postings[i].push_back((uint32_t)f.getInt(0, _words.size()));
                    }
                    std::sort(psi->_postings[i].begin(), psi->_postings[i].end());
                }
            }
            else
            {
                for (auto& term : _terms)
                {
                    std::sort(term.second.posting.begin(), term.second.posting.end());
                    for (const auto& hash : term.second.hash)
                    {
                        const uint32_t index = psi->_ph.search(hash);
                        const uint32_t value = alex::hash::jenkinsHash(hash) % psi->_maxHash;
                        psi->_hashBits.set(psi->_hashWidth * index, value, psi->_hashWidth);
                        psi->_postings[index] = term.second.posting;
                    }
                }
            }

            for (unsigned int i = 0; i < _terms.size(); ++i)
                delete [] terms[i];
            delete [] terms;

            return SecureIndex(psi);
        };

        void makeTerms()
        {
            namespace helper = psi_post_builder_internal;
            if (_words.empty())
                throw std::exception("No Terms");

            auto f = stochastic::Entropy<std::minstd_rand0>();

            auto& word = _terms[_words[0]];
            auto crypts = alex::utils::cryptoTerms(_words[0], _params.secrets, _ref);
            for (auto crypt : crypts)
                word.hash.push_back(crypt);   

            uint32_t x = helper::triangular(0, _params.offsetRadius, 0, f.get0_1f());
            word.posting.push_back(x);

            std::string bigram;
            for (unsigned int i = 1; i < _words.size(); ++i)
            {
                if (_terms.count(_words[i]) == 0)
                {
                    auto& word = _terms[_words[i]];
                    auto crypts = alex::utils::cryptoTerms(_words[i], _params.secrets, _ref);
                    for (auto crypt : crypts)
                        word.hash.push_back(std::move(crypt));                   
                }
                
                int left = std::max(0, (int)i - (int)_params.offsetRadius);
                int right = std::min((int)_words.size(), (int)i + (int)_params.offsetRadius);

                x = helper::triangular(left, right, i, f.get0_1f());
                _terms[_words[i]].posting.push_back(x);

                bigram = _words[i-1] + " " + _words[i];
                if (_terms.count(bigram) == 0)
                {
                    auto& word = _terms[bigram];
                    auto crypts = alex::utils::cryptoTerms(bigram, _params.secrets, _ref);
                    for (auto crypt : crypts)
                        word.hash.push_back(std::move(crypt));                   
                }
                
                x = helper::triangular(left, right, i, f.get0_1f());
                _terms[bigram].posting.push_back(x);
            }

            if (_params.freqError > 0)
            {
                auto f = stochastic::Entropy<std::minstd_rand0>();
                for (auto& t : _terms)
                {
                    unsigned int n = (unsigned int)std::round(t.second.posting.size() * f.getReal(0, _params.freqError));
                    for (unsigned int i = 0; i < n; ++i)
                        t.second.posting.push_back(f.getInt(0, _words.size()));
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

        void printWords(std::ostream& outs = std::cout)
        {
            std::cout << "Reference: " << _ref << std::endl;
            for (auto& word : _words)
                std::cout << "\t" << word << std::endl;
        };

        void printTerms(std::ostream& outs = std::cout)
        {
            std::cout << "Reference: " << _ref << std::endl;

            for (auto& term : _terms)
            {
                outs << "\t    Key: " << term.first << std::endl;
                outs << "\t   Hash: ";
                for (auto x : term.second.hash)
                     outs << x << " ";
                outs << std::endl;
                outs << "\tPosting: ";
                for (const auto& x : term.second.posting)
                    outs << x << " ";
                outs << std::endl;
            }
        };

        boost::filesystem::path getPreferredFileExtension() const { return PsiPost::HEADER; };

    private:
        BuilderParams _params;
        std::string _ref;
        std::vector<std::string> _words;

        struct Record
        {
            std::vector<std::string> hash;
            std::vector<uint32_t> posting;
        };

        // TODO: benchmark this compared to making the map map to a Record
        //       struct which has the set/unordered_set of integers
        std::unordered_map<std::string, Record> _terms;
    };
}}

#endif