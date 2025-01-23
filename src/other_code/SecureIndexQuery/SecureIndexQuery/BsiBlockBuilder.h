#ifndef __BSI_BLOCK_BUILDER_H__
#define __BSI_BLOCK_BUILDER_H__

#include <string>
#include <unordered_set>
#include <unordered_map>
#include "BsiBlock.h"
#include "BitArray.h"
#include <boost/filesystem.hpp>
#include "Entropy.h"
#include "ISecureIndex.h"
#include "Globals.h"

namespace alex { namespace index
{
    class BsiBlockBuilder
    {
        friend class PsiFreqBsiBlockBuilder;

    public:
        struct BuilderParams
        {
            int verbose;
            uint32_t maxBlocks;
            uint32_t wordsPerBlock;
            double falsePositiveRate;
            std::unordered_set<std::string> ignoreWords;
            bool stemming;
            vector<std::string> secrets;
            // ignore regular expression pattern (list?)

            BuilderParams()
            {
                defaults();
            };

            void defaults()
            {
                verbose = alex::globals::default_verbose;
                wordsPerBlock = alex::globals::default_words_per_block;
                secrets = alex::globals::default_secrets;
                falsePositiveRate = alex::globals::default_fp_rate;
                maxBlocks = alex::globals::default_max_blocks;
                stemming = alex::globals::default_stemming;
                ignoreWords = alex::globals::default_stop_words;
            }
        };

        BsiBlockBuilder(const BuilderParams& params) :
            _params(params),
            _numBlocks(0),
            _approximateWords(0) {};

        BsiBlockBuilder() :
            _approximateWords(0),
            _wordsPerBlock(0)
        {
            _params.defaults();
        }

        // TODO: let block 0 be a meta-data block, no location property -> title, author, manually added keywords
        SecureIndex build()
        {
            if (_words.empty())
                throw std::exception("No Terms");

            auto entropy = alex::stochastic::Entropy<std::minstd_rand0>();

            auto bsi = new BsiBlock();
            bsi->_ref = _ref;
            bsi->_blooms.reserve(_numBlocks);
            bsi->_blockSize = _wordsPerBlock;
            bsi->_approximateWords = _approximateWords;
            
            bf::bloom_parameters params;
            params.false_positive_probability = _params.falsePositiveRate / (double)_numBlocks;

            for (size_t i = 0; i < _numBlocks; ++i)
            {
                // TODO: poison/add noise to bloom filters
                //       randomize stuff for less info leakage
                //          -> auto f = stochastic::Entropy<std::minstd_rand0>();
                params.projected_element_count = _blocks[i].size() * _params.secrets.size();
                params.random_seed = entropy.get();
                params.compute_optimal_parameters();

                bsi->_blooms.push_back(bf::bloom_filter(params));
                for (const auto& term : _blocks[i])
                {
                    for (const auto& hd : term.second)
                        bsi->_blooms[i].insert(hd);
                }
            }

            return SecureIndex(bsi);
        };

        boost::filesystem::path getPreferredFileExtension() const { return BsiBlock::HEADER; };

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

            _blocks.resize(_numBlocks);
            auto& word = _blocks[0][_words[0]];
            auto crypts = alex::utils::cryptoTerms(_words[0], _params.secrets, _ref);

            // add all the hidden term variations (one for each secret)
            // used for obfuscating queries at cost of inflated index
            for (auto crypt : crypts)
                word.push_back(crypt);

            std::string bigram;
            for (unsigned int i = 1; i < _words.size(); ++i)
            {
                unsigned int blck = i / _wordsPerBlock;
                if (_blocks[blck].count(_words[i]) == 0)
                {
                    auto& word = _blocks[blck][_words[i]];
                    const auto& crypts = alex::utils::cryptoTerms(_words[i], _params.secrets, _ref);
                    for (auto crypt : crypts)
                        word.push_back(crypt);
                }

                bigram = _words[i-1] + " " + _words[i];
                std::string digest;
                if (_blocks[blck].count(bigram) == 0)
                {
                    auto& word = _blocks[blck][bigram];
                    const auto& crypts = alex::utils::cryptoTerms(bigram, _params.secrets, _ref);
                    for (auto crypt : crypts)
                        word.push_back(crypt);
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

            _approximateWords = _words.size();
        };

        void printWords(std::ostream& outs = std::cout)
        {
            outs << "Reference: " << _ref << std::endl;
            for (auto& word : _words)
            {
                outs << "\t\"" << word << std::endl;
            }
        };

        void printTerms(std::ostream& outs = std::cout)
        {
            outs << "Reference: " << _ref << std::endl;

            for (size_t i = 0; i < _blocks.size(); ++i)
            {
                outs << "Block " << i << std::endl;
                auto& b = _blocks[i];

                for (const auto& word : b)
                {
                    outs << "\t" << word.first << " -> ";
                    for (const auto& ht : word.second)
                        outs << ht << " ";
                    outs << std::endl;
                }
                outs << std::endl;
            }
        };

    private:
        BuilderParams _params;
        unsigned int _numBlocks;
        unsigned int _wordsPerBlock;
        unsigned int _approximateWords;
        std::string _ref;
        std::vector<std::string> _words;
        std::vector<std::unordered_map<std::string, std::vector<std::string>>> _blocks;
    };
}}

#endif