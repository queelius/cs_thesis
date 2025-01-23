#ifndef __PSI_POST_BUILDER_H__
#define __PSI_POST_BUILDER_H__

#include <string>
#include <unordered_set>
#include <unordered_map>
#include "PsiPost.h"
#include "BitArray.h"
#include "ISecureIndex.h"
#include <boost/filesystem.hpp>

namespace alex { namespace index
{
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
            // psi->_hashBits.resizeBits(sz * _params.falsePositiveBits) + f.getInt(0, _params.hashBitsRandomPadding))

            // poison hash bits -- randomize it. the actual entries will
            // be inserted next, thus overwriting these when necessary.
            for(unsigned int i = 0; i < psi->_hashBits.bytes(); ++i)
                psi->_hashBits.setByte(i, f.get() % psi->MAX_BYTE);

            //std::set<uint32_t> usedIndexes;
            for (const auto term : _terms)
            {
                for (auto hash : term.second.hash)
                {
                    const uint32_t index = psi->_ph.search(hash);
                    const uint32_t value = alex::hash::jenkinsHash(hash) % psi->_maxHash;
                    psi->_hashBits.set(psi->_hashWidth * index, value, psi->_hashWidth);
                    psi->_postings[index] = term.second.posting;
                    //usedIndexes.insert(index);
                }
            }

            /*
            // poison unused indexes with fake
            // postings lists.
            for(unsigned int i = 0; i < sz; ++i)
            {
                if (usedIndexes.count(i) != 0)
                    continue;

                // make from 1 to 3 postings for fake
                // word at index i. this should
                // probabably be a discrete distribution
                // that follows the zipf distribution,
                // but it's not important.
                unsigned int n = f.getInt(1, 3);
                psi->_postings[i].reserve(n);
                for (unsigned int j = 0; j < n; ++j)
                    psi->_postings[i].push_back((uint32_t)f.getInt(0, _words.size()));
                std::sort(psi->_postings[i].begin(), psi->_postings[i].end());
            }
            */

            for (unsigned int i = 0; i < _terms.size(); ++i)
                delete [] terms[i];
            delete [] terms;

            return SecureIndex(psi);
        };

        void makeTerms()
        {
            if (_words.empty())
                throw std::exception("No Terms");

            auto f = stochastic::Entropy<std::minstd_rand0>();
            auto& word = _terms[_words[0]];
            auto crypts = alex::utils::cryptoTerms(_words[0], _params.secrets, _ref);
            for (auto crypt : crypts)
                word.hash.push_back(crypt);   
            word.posting.push_back(f.getInt(0, (int)_params.offsetRadius));

            std::string bigram;
            int randomPos;
            for (unsigned int i = 1; i < _words.size(); ++i)
            {
                if (_terms.count(_words[i]) == 0)
                {
                    auto& word = _terms[_words[i]];
                    const auto& crypts = alex::utils::cryptoTerms(_words[i], _params.secrets, _ref);
                    for (auto crypt : crypts)
                        word.hash.push_back(crypt);                   
                }
                randomPos = (int)i + f.getInt(-((int)_params.offsetRadius), (int)_params.offsetRadius);
                _terms[_words[i]].posting.push_back(randomPos < 0 ? 0 : randomPos);

                bigram = _words[i-1] + " " + _words[i];
                if (_terms.count(bigram) == 0)
                {
                    auto& word = _terms[bigram];
                    const auto& crypts = alex::utils::cryptoTerms(bigram, _params.secrets, _ref);
                    for (auto crypt : crypts)
                        word.hash.push_back(crypt);                   
                }
                randomPos = (int)i + f.getInt(-((int)_params.offsetRadius), (int)_params.offsetRadius);
                _terms[bigram].posting.push_back(randomPos < 0 ? 0 : randomPos);
            }

            for (auto& t : _terms)
            {
                std::sort(t.second.posting.begin(), t.second.posting.end());
                //auto newEnd = std::unique(t.second.posting.begin(), t.second.posting.end());
                //t.second.posting.erase(newEnd, t.second.posting.end());
                //t.second.posting.shrink_to_fit();
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