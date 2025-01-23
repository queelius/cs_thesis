#ifndef __PSI_FREQ_BLOCK_BUILDER_H__
#define __PSI_FREQ_BLOCK_BUILDER_H__

#include <string>
#include <unordered_set>
#include <unordered_map>
#include "Globals.h"
#include "PsiFreqBsiBlock.h"
#include "PsiFreqBuilder.h"
#include "BsiBlockBuilder.h"
#include "BitArray.h"
#include <boost/filesystem.hpp>
#include "ISecureIndex.h"

namespace alex { namespace index
{
    class PsiFreqBsiBlockBuilder
    {
    public:
        struct BuilderParams
        {
            int verbose;
            uint32_t maxFreq;
            std::unordered_set<std::string> ignoreWords;
            double loadFactor;
            bool stemming;
            vector<std::string> secrets;
            cmph_cpp::CMPH_ALGO algo;
            uint32_t maxBlocks;
            uint32_t wordsPerBlock;
            double falsePositiveRate;
            // ignore regular expression pattern (list?)

            uint32_t bitsNeededForFalsePositiveRate(double rate) const
            {
                if(rate < 0 || rate > 1)
                    throw std::exception("False Positive Rate Must Be In Range [0, 1]");
                return (uint32_t)std::ceil(-std::log(rate) / std::log(2));
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
                secrets = alex::globals::default_secrets;
                stemming = alex::globals::default_stemming;
                ignoreWords = alex::globals::default_stop_words;
                maxFreq = std::numeric_limits<uint32_t>::max();
                wordsPerBlock = alex::globals::default_words_per_block;
                falsePositiveRate = alex::globals::default_fp_rate;
                maxBlocks = alex::globals::default_max_blocks;
            }
        };

        void dump()
        {
            // finish this
        };

        PsiFreqBsiBlockBuilder()
        {
            BuilderParams params;
            params.defaults();

            //_freq._params.falsePositiveBits = params.bitsNeededForFalsePositiveRate(params.falsePositiveRate) / 2;
            _freq._params.falsePositiveBits = params.bitsNeededForFalsePositiveRate(params.falsePositiveRate);
            //_block._params.falsePositiveRate = params.falsePositiveRate * params.falsePositiveRate;
            _block._params.falsePositiveRate = params.falsePositiveRate;
            _freq._params.stemming = params.stemming;
            _block._params.stemming = params.stemming;
            _freq._params.ignoreWords = params.ignoreWords;
            _block._params.ignoreWords = params.ignoreWords;
            _freq._params.loadFactor = params.loadFactor;
            _block._params.verbose = params.verbose;
            _freq._params.verbose = params.verbose;
            _block._params.maxBlocks = params.maxBlocks;
            _freq._params.maxFreq = params.maxFreq;
            _block._params.wordsPerBlock = params.wordsPerBlock;
            _freq._params.secrets = params.secrets;
            _block._params.secrets = params.secrets;
        }

        PsiFreqBsiBlockBuilder(const BuilderParams& params)
        {
            _freq._params.falsePositiveBits = params.bitsNeededForFalsePositiveRate(params.falsePositiveRate);
            _block._params.falsePositiveRate = params.falsePositiveRate;
            _freq._params.stemming = params.stemming;
            _block._params.stemming = params.stemming;
            _freq._params.ignoreWords = params.ignoreWords;
            _block._params.ignoreWords = params.ignoreWords;
            _freq._params.loadFactor = params.loadFactor;
            _block._params.verbose = params.verbose;
            _freq._params.verbose = params.verbose;
            _block._params.maxBlocks = params.maxBlocks;
            _freq._params.maxFreq = params.maxFreq;
            _block._params.wordsPerBlock = params.wordsPerBlock;
            _freq._params.secrets = params.secrets;
            _block._params.secrets = params.secrets;
        }

        SecureIndex build()
        {
            PsiFreqBsiBlock* si = new PsiFreqBsiBlock();

            si->_freq = _freq.build();
            si->_block = _block.build();
            return SecureIndex(si);
        };

        void makeTerms()
        {
            _freq.makeTerms();
            _block.makeTerms();
        };

        void parse(const boost::filesystem::path& file)
        {
            parse(file.string(), std::ifstream(file.string()));
        };

        void parse(const std::string& ref, std::istream& file)
        {
            _freq.parse(ref, file);
            file.clear();
            file.seekg (0, file.beg);
            _block.parse(ref, file);
        };

        boost::filesystem::path getPreferredFileExtension() const { return PsiFreqBsiBlock::HEADER; };

        void printWords(std::ostream& outs = std::cout)
        {
            _freq.printWords();
        };

        void printTerms(std::ostream& outs = std::cout)
        {
            _freq.printTerms();
        };

    private:
        BsiBlockBuilder _block;
        PsiFreqBuilder _freq;
    };
}}

#endif