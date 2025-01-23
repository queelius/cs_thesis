#ifndef __PSI_FREQ_BUILDER_H__
#define __PSI_FREQ_BUILDER_H__

#include <string>
#include <unordered_set>
#include <unordered_map>
#include "Globals.h"
#include "PsiFreq.h"
#include "BitArray.h"
#include <boost/filesystem.hpp>
#include "ISecureIndex.h"

namespace alex { namespace index
{
    class PsiFreqBuilder
    {
        friend class PsiMinBuilder;
        friend class PsiFreqBsiBlockBuilder;

    public:
        struct BuilderParams
        {
            int verbose;
            uint32_t maxFreq;
            uint32_t falsePositiveBits;
            std::unordered_set<std::string> ignoreWords;
            double loadFactor;
            bool stemming;
            vector<std::string> secrets;
            cmph_cpp::CMPH_ALGO algo;
            double freqError;

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
                algo = alex::globals::default_ph;
                loadFactor = alex::globals::default_load_factor;
                freqError = 0;
                verbose = alex::globals::default_verbose;
                secrets = alex::globals::default_secrets;
                falsePositiveRate(alex::globals::default_fp_rate);
                stemming = alex::globals::default_stemming;
                ignoreWords = alex::globals::default_stop_words;
                maxFreq = std::numeric_limits<uint32_t>::max();
            };
        };

        void dump()
        {
            std::cout << "maxFreq: " << this->_maxFreq << std::endl;
            std::cout << "\tref: " << this->_ref << std::endl;
            
            std::cout << "\tparams.fpBits: " << this->_params.falsePositiveBits << std::endl;
            std::cout << "\tparams.maxFreq: " << this->_params.maxFreq << std::endl;
            std::cout << "\tparams.secrets: ";
            for (auto x : this->_params.secrets)
                std::cout << x << " ";
            std::cout << std::endl;
            std::cout << "\tparams.ignoreWords: ";
            for (auto x : this->_params.ignoreWords)
                std::cout << x << " ";
            std::cout << std::endl;
            std::cout << "\tparams.stemming: " << this->_params.stemming << std::endl;
            std::cout << "\tparams.loadFactor: " << this->_params.loadFactor << std::endl;
            std::cout << "\tparams.freqError: " << this->_params.freqError << std::endl;
            std::cout << "\tparams.verbose: " << this->_params.verbose << std::endl;
            std::cout << "\tparams.algo: " << this->_params.algo << std::endl;
        };

        PsiFreqBuilder() : _maxFreq(0) {}

        PsiFreqBuilder(const BuilderParams& params) : _params(params), _maxFreq(0) {}

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

            auto psi = new PsiFreq();
            uint32_t sz = psi->_ph.build(terms, (_terms.size() * _params.secrets.size()), _params.verbose, _params.loadFactor, _params.algo);

            psi->_ref = _ref;
            psi->_hashWidth = _params.falsePositiveBits;
            psi->_maxHash = (1 << _params.falsePositiveBits);
            psi->_hashBits.resizeBits(sz * _params.falsePositiveBits);
            psi->_approximateWords = _words.size();

            if (_maxFreq <= 1)
                psi->_freqWidth = 0;
            else
            {
                psi->_freqWidth = (uint32_t)std::ceil(std::log(_maxFreq) / std::log(2));
                psi->_freqBits.resizeBits(sz * psi->_freqWidth);
            }
            
            if (_params.loadFactor < 1)
            {
                for(unsigned int i = 0; i < psi->_hashBits.bytes(); ++i)
                    psi->_hashBits.setByte(i, f.get() % psi->MAX_BYTE);
                for(unsigned int i = 0; i < psi->_freqBits.bytes(); ++i)
                    psi->_freqBits.setByte(i, f.get() % psi->MAX_BYTE);
            }

            for (const auto& term : _terms)
            {
                for (auto hash : term.second.hash)
                {
                    const uint32_t index = psi->_ph.search(hash);
                    const uint32_t value = alex::hash::jenkinsHash(hash) % psi->_maxHash;
                    psi->_hashBits.set(psi->_hashWidth * index, value, _params.falsePositiveBits);
                    psi->_freqBits.set(psi->_freqWidth * index, term.second.freq, psi->_freqWidth);
                }
            }

            for (unsigned int i = 0; i < _terms.size(); ++i)
                delete [] terms[i];
            delete [] terms;

            return SecureIndex(psi);
        };

        void makeTerms()
        {
            if (_words.empty())
                throw std::exception("No Terms");

            _maxFreq = 1;
            auto& word = _terms[_words[0]];
            auto crypts = alex::utils::cryptoTerms(_words[0], _params.secrets, _ref);
            for (auto crypt : crypts)
                word.hash.push_back(std::move(crypt));                   
            word.freq = 1;

            std::string bigram;
            for (unsigned int i = 1; i < _words.size(); ++i)
            {
                if (_terms.count(_words[i]) == 0)
                {
                    auto& word = _terms[_words[i]];
                    word.freq = 0;
                    const auto& crypts = alex::utils::cryptoTerms(_words[i], _params.secrets, _ref);
                    for (auto crypt : crypts)
                        word.hash.push_back(std::move(crypt));                   
                }
                ++_terms[_words[i]].freq;
                if (_terms[_words[i]].freq > _maxFreq)
                {
                    if (_terms[_words[i]].freq > _params.maxFreq)
                        _terms[_words[i]].freq = _params.maxFreq;
                    _maxFreq = _terms[_words[i]].freq;
                }

                bigram = _words[i-1] + " " + _words[i];
                if (_terms.count(bigram) == 0)
                {
                    auto& word = _terms[bigram];
                    word.freq = 0;
                    const auto& crypts = alex::utils::cryptoTerms(bigram, _params.secrets, _ref);
                    for (auto crypt : crypts)
                        word.hash.push_back(std::move(crypt));                   
                }
                ++_terms[bigram].freq;
                if (_terms[bigram].freq > _maxFreq)
                {
                    if (_terms[bigram].freq > _params.maxFreq)
                        _terms[bigram].freq = _params.maxFreq;
                    _maxFreq = _terms[bigram].freq;
                }
            }

            if (_params.freqError > 0)
            {
                auto f = stochastic::Entropy<std::minstd_rand0>();
                for (auto& term : _terms)
                {
                    term.second.freq = std::max(1, (int)std::round(term.second.freq * (1 + f.getReal(-_params.freqError, _params.freqError))));
                    _maxFreq = std::max(_maxFreq, term.second.freq);
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

        boost::filesystem::path getPreferredFileExtension() const { return PsiFreq::HEADER; };

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
                outs << "\t Key: " << term.first << std::endl;
                outs << "\tHash: ";
                for (auto x : term.second.hash)
                     outs << x << " ";
                outs << std::endl;
                outs << "\tFreq: " << term.second.freq << std::endl;
                outs << std::endl;
            }
        };

    private:
        BuilderParams _params;
        std::string _ref;
        std::vector<std::string> _words;
        uint32_t _maxFreq;

        struct Record
        {
            std::vector<std::string> hash;
            uint32_t freq;
        };

        std::unordered_map<std::string, Record> _terms;
    };
}}

#endif