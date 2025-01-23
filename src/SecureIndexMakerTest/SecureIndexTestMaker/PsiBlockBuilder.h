#ifndef __PSI_BLOCK_BUILDER_H__
#define __PSI_BLOCK_BUILDER_H__

#include <string>
#include <unordered_set>
#include <unordered_map>
#include "PsiBlock.h"
#include "BitArray.h"
#include "ISecureIndex.h"
#include <boost/filesystem.hpp>
#include "Globals.h"

namespace alex { namespace index
{
    class PsiBlockBuilder
    {
    public:
        struct BuilderParams
        {
            uint32_t maxBlocks;
            uint32_t wordsPerBlock;
            uint32_t falsePositiveBits;
            std::unordered_set<std::string> ignoreWords;
            int verbose;
            double loadFactor;
            bool stemming;
            vector<std::string> secrets;
            cmph_cpp::CMPH_ALGO algo;

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
                verbose = alex::globals::default_verbose;
                wordsPerBlock = alex::globals::default_words_per_block;
                secrets = alex::globals::default_secrets;
                falsePositiveRate(alex::globals::default_fp_rate);
                maxBlocks = alex::globals::default_max_blocks;
                stemming = alex::globals::default_stemming;
                ignoreWords = alex::globals::default_stop_words;
            };
        };

        PsiBlockBuilder(const BuilderParams& params) : _params(params), _numBlocks(0) {};

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

            auto psi = new PsiBlock();
            uint32_t sz = psi->_ph.build(terms, (_terms.size() * _params.secrets.size()), (bool)_params.verbose, _params.loadFactor, _params.algo);
            psi->_ref = _ref;
            psi->_blockSize = _wordsPerBlock;
            psi->_blockWidth = _numBlocks;
            psi->_approximateWords = _approximateWords;
            psi->_blockBits.resizeBytes(alex::utils::toBytes(sz * _numBlocks));
            psi->_hashWidth = _params.falsePositiveBits;
            psi->_maxHash = (1 << _params.falsePositiveBits);
            psi->_hashBits.resizeBytes(alex::utils::toBytes(sz * _params.falsePositiveBits));

            if (_params.loadFactor < 1)
            {
                for(unsigned int i = 0; i < psi->_hashBits.bytes(); ++i)
                    psi->_hashBits.setByte(i, f.get() % psi->MAX_BYTE);
                for(unsigned int i = 0; i < psi->_blockBits.bytes(); ++i)
                    psi->_blockBits.setByte(i, f.get() % psi->MAX_BYTE);
            }

            for (const auto& term : _terms)
            {
                for (const auto& hash : term.second.hash)
                {
                    const uint32_t index = psi->_ph.search(hash);
                    const uint32_t value = alex::hash::jenkinsHash(hash) % psi->_maxHash;
                    psi->_hashBits.set(psi->_hashWidth * index, value, _params.falsePositiveBits);

                    auto& bits = term.second.bits;
                    const unsigned int w = index * _numBlocks;
                    for (unsigned int i = 0; i < _numBlocks; ++i)
                    {
                        if (bits.getBit(i))
                            psi->_blockBits.setBit(w + i);
                        else
                            psi->_blockBits.clearBit(w + i);
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
            if (_words.empty())
                throw std::exception("No Terms");

            _wordsPerBlock = _params.wordsPerBlock;
            _numBlocks = (_words.size()-1) / _wordsPerBlock + 1;
            if (_numBlocks > _params.maxBlocks)
            {
                _wordsPerBlock = (_words.size()-1) / (_params.maxBlocks - 1);
                _numBlocks = _params.maxBlocks;
            }

            auto& word = _terms[_words[0]];
            auto crypts = alex::utils::cryptoTerms(_words[0], _params.secrets, _ref);
            for (auto crypt : crypts)
                word.hash.push_back(crypt);                   
            word.bits.resizeBits(_numBlocks);
            word.bits.setBit(0);

            std::string bigram;
            for (unsigned int i = 1; i < _words.size(); ++i)
            {
                if (_terms.count(_words[i]) == 0)
                {
                    auto& word = _terms[_words[i]];
                    const auto& crypts = alex::utils::cryptoTerms(_words[i], _params.secrets, _ref);
                    for (auto crypt : crypts)
                        word.hash.push_back(crypt);                   
                    word.bits.resizeBits(_numBlocks);
                }
                _terms[_words[i]].bits.setBit(i / _wordsPerBlock);

                bigram = _words[i-1] + " " + _words[i];
                if (_terms.count(bigram) == 0)
                {
                    auto& word = _terms[bigram];
                    const auto& crypts = alex::utils::cryptoTerms(bigram, _params.secrets, _ref);
                    for (auto crypt : crypts)
                        word.hash.push_back(crypt);                   
                    word.bits.resizeBits(_numBlocks);
                }

                _terms[bigram].bits.setBit(i / _wordsPerBlock);
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

            _approximateWords = _words.size();
        };

        void printWords(std::ostream& outs = std::cout)
        {
            std::cout << "Reference: " << _ref << std::endl;
            for (auto& word : _words)
            {
                std::cout << "\t" << word << std::endl;
            }
        };

        void printTerms(std::ostream& outs = std::cout)
        {
            std::cout << "Reference: " << _ref << std::endl;
            for (auto& term : _terms)
            {
                outs << "\t   Key: " << term.first << std::endl;
                outs << "\t  Hash: ";
                for (auto x : term.second.hash)
                     outs << x << " ";
                outs << std::endl;
                outs << "\tBlocks: ";
                for (unsigned int i = 0; i < term.second.bits.size(); ++i)
                    outs << term.second.bits.getBit(i);
                outs << std::endl << std::endl;
            }
        };

        boost::filesystem::path getPreferredFileExtension() const { return PsiBlock::HEADER; };

    private:
        unsigned int _numBlocks;
        std::string _ref;
        unsigned int _approximateWords;
        unsigned int _wordsPerBlock;
        std::vector<std::string> _words;
        BuilderParams _params;

        struct Record
        {
            std::vector<std::string> hash;
            alex::collections::BitArray bits;
        };

        // TODO: benchmark this compared to making the map map to a Record
        //       struct which has the set/unordered_set of integers
        std::unordered_map<std::string, Record> _terms;
    };
}}

#endif