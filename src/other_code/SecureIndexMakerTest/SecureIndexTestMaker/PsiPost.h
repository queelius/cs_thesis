#ifndef __PSI_POST_H__
#define __PSI_POST_H__

#include <iostream>
#include <string>
#include <ostream>
#include "BitArray.h"
#include "ICmph.h"
#include "Cfg.h"
#include "Hash.h"
#include "Utils.h"
#include "BinaryIO.h"
#include <iomanip>
#include <cstdint>
#include "HiddenQuery.h"
#include "ISecureIndex.h"

namespace alex { namespace index
{
    // Perfect secure index (postings list)
    class PsiPost: public ISecureIndex
    {
        friend class PsiPostBuilder;

        public:
            static const uint8_t VERSION = 1;
            static const char HEADER[];

            PsiPost() : _numPostings(0), _hashWidth(0), _maxHash(0), _maxOffset(0) {};

            PsiPost(const PsiPost& copy) :
                _maxOffset(copy._maxOffset),
                _postings(copy._postings),
                _numPostings(copy._numPostings),
                _approximateWords(copy._approximateWords),
                _maxHash(copy._maxHash),
                _ph(copy._ph),
                _hashWidth(copy._hashWidth),
                _hashBits(copy._hashBits),
                _ref(copy._ref) {};

            PsiPost(PsiPost&& src) :
                _maxOffset(src._maxOffset),
                _postings(std::move(src._postings)),
                _numPostings(src._numPostings),
                _maxHash(src._maxHash),
                _approximateWords(src._approximateWords),
                _ph(std::move(src._ph)),
                _hashWidth(src._hashWidth),
                _hashBits(std::move(src._hashBits)),
                _ref(std::move(src._ref)) {};
            virtual ~PsiPost()
            {
                clear();
            };

            PsiPost& operator=(PsiPost&& other)
            {
                if (this != &other)
                {
                    clear();

                    _maxOffset = other._maxOffset;
                    _postings = std::move(other._postings);
                    _hashBits = std::move(other._hashBits);
                    _ph = std::move(other._ph);
                    _ref = std::move(other._ref);
                    _hashWidth = other._hashWidth;
                    _maxHash = other._maxHash;
                    _numPostings = other._numPostings;
                    _approximateWords = other._approximateWords;

                    other._maxOffset = 0;
                    other._approximateWords = 0;
                    other._numPostings = 0;
                    other._hashWidth = 0;
                    other._maxHash = 0;
                }

                return *this;
            };

            PsiPost& operator=(const PsiPost& other)
            {
                if (this != &other)
                {
                    clear();

                    _maxOffset = other._maxOffset;
                    _postings = other._postings;
                    _hashBits = other._hashBits;
                    _ph = other._ph;
                    _ref = other._ref;
                    _hashWidth = other._hashWidth;
                    _maxHash = other._maxHash;
                    _numPostings = other._numPostings;
                    _approximateWords = other._approximateWords;
                }

                return *this;
            };

            void read(std::istream& file)
            {
                using namespace alex::io::binary;

                const std::string& hdr = readString(file);
                if (hdr != HEADER)
                    throw std::exception(("Unexpected Header: " + hdr).c_str());

                unsigned int version = readU8(file);
                if (version != VERSION)
                    throw std::exception("Unexpected Verion");

                // get reference (e.g., url) for encrypted document
                _ref = readString(file);

                _approximateWords = readVarU32(file);

                _maxOffset = readVarU32(file);

                // de-serialize perfect hash function
                _ph.read(file);

                // read hash entries
                // get number of bits per hash entry (false positive rate = 1/2^width)
                _hashWidth = readVarU32(file);

                // derive maximum hash value from bits allocated to hash entries
                _maxHash = (1 << _hashWidth);

                // read bit array for compactly (packed) storing hash entries
                readBitArray(file, _hashBits);

                _numPostings = readVarU32(file);
                _postings.resize(_numPostings);
                for (size_t i = 0; i < _numPostings; ++i)
                {
                    alex::io::binary::readVecVarU32(file, _postings[i]);
                }
            };
            void write(std::ostream& file)
            {
                using namespace alex::io::binary;

                writeString(file, HEADER);
                writeU8(file, VERSION);

                // write reference (e.g., url) for encrypted document
                writeString(file, _ref);

                writeVarU32(file, _approximateWords);

                writeVarU32(file, _maxOffset);

                // serialize perfect hash function
                _ph.write(file);

                // write hash entries
                writeVarU32(file, _hashWidth);

                // write bit array for compactly (packed) hash entries
                writeBitArray(file, _hashBits);

                writeVarU32(file, _numPostings);
                for (auto posting : _postings)
                    writeVecVarU32(file, posting);
            };

            void dump(int depth)
            {
                std::cout << std::setw(depth) << "ApproximateWords: " << _approximateWords << std::endl;
                std::cout << std::setw(depth) << "MaxHash: " << _maxHash << std::endl;
                std::cout << std::setw(depth) << "HashWidth: " << _hashWidth << std::endl;
                std::cout << std::setw(depth) << "MaxOffset: " << _maxOffset << std::endl;
                std::cout << std::setw(depth) << "Ref: " << _ref << std::endl;
                std::cout << std::setw(depth) << "NumPostings: " << _numPostings << std::endl;
                this->_ph.dump(depth+1);
                for (size_t i = _postings.size() - 20; i < _postings.size(); ++i)
                {
                    std::cout << _hashBits.get(i * _hashWidth, _hashWidth) << " -> ";
                    for (auto x : _postings[i])
                        std::cout << x << " ";
                    std::cout << std::endl;
                }
            };

            uint32_t getHashWidth() const { return _hashWidth; };
            alex::hash::ICmph getCmph() const { return _ph; };

            std::string getReference() const { return _ref; };
            void setReference(const std::string& ref) { _ref = ref; };
            TSecureIndex getType() const { return TSecureIndex::PsiPost; };
            unsigned int getApproximateWords() const { return _approximateWords; };
            bool hasProximity() const { return true; };
            bool hasFrequency() const { return true; };
            void read(const boost::filesystem::path& infile) { read(std::ifstream(infile.string(), std::ios::binary)); };
            void write(const boost::filesystem::path& outfile) { write(std::ofstream(outfile.string(), std::ios::binary)); };
            std::vector<uint32_t> getLocations(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                if (t.empty())
                    return std::vector<uint32_t>();

                const std::string suffix = "|" + _ref;
                if (t.size() == 1)
                {
                    const std::string& q = alex::crypt::hexdigest<alex::globals::digest_size>(t[0] + suffix);
                    const uint32_t idx = _ph.search(q);
                    const uint32_t value = alex::hash::jenkinsHash(q) % _maxHash;

                    if (_hashBits.get(_hashWidth * idx, _hashWidth) == value)
                        return _postings[idx];
                    else // a sub-term (i.e., word) is not in ph, so its conjunction can't be either
                        return std::vector<uint32_t>();
                }

                unsigned int max = std::numeric_limits<unsigned int>::max();
                std::vector<std::vector<uint32_t>> postings;
                for (const auto& x : t)
                {
                    const std::string& q = alex::crypt::hexdigest<alex::globals::digest_size>(x + suffix);
                    const uint32_t idx = _ph.search(q);
                    const uint32_t value = alex::hash::jenkinsHash(q) % _maxHash;

                    // a sub-term (i.e., word) is not in ph, so its conjunction can't be either
                    if (_hashBits.get(_hashWidth * idx, _hashWidth) == value)
                        max = std::min(max, (unsigned int)_postings[idx].size());
                    else
                        return std::vector<uint32_t>();

                    postings.push_back(_postings[idx]);
                }

                std::set<std::pair<uint32_t, uint32_t>> candidates;
                alex::utils::cover2(postings, 0, std::numeric_limits<uint32_t>::max(),
                                   std::numeric_limits<uint32_t>::min(), candidates, 2*_maxOffset + t.size(), max);

                std::set<uint32_t> results;
                for (const auto& p : candidates)
                {
                    for (unsigned int i = p.first - (p.first > 0 ? 1 : 0); i <= p.second; ++i)
                        results.insert(i);
                }

                return std::vector<uint32_t>(results.begin(), results.end());
            };           
            std::string getPreferredFileExtension() const { return HEADER; };
            std::string toString() const
            {
                std::stringstream ss;
                ss << _ref << std::endl;
                for (unsigned int i = 0; i < _hashBits.size() / _hashWidth; ++i)
                {
                    ss << "\t" << std::right << std::setw(8) << i << " -> hash={";
                    for (unsigned int j = 0; j < _hashWidth; ++j)
                       ss << _hashBits.getBit(i * _hashWidth + j);
                    ss << "} :: postings={ ";

                    for (const auto& place : _postings[i])
                        ss << place << " ";
                    ss << "}\n";
                }
                ss << std::endl;

                return ss.str();
            };
            void clear()
            {
                if (_hashBits.size() != 0)
                {
                    _maxOffset = 0;
                    _postings.clear();
                    _postings.shrink_to_fit();
                    _approximateWords = 0;
                    _numPostings = 0;
                    _hashWidth = 0;
                    _maxHash = 0;
                    _hashBits.clear();
                    _ref = "";
                    _ph.clear();
                }
            };
            unsigned int getFrequency(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                if (t.empty())
                    return 0;

                const std::string& suffix = "|" + _ref;

                // handle easy unigram/bigram case quickly
                // since unigrams and bigrams (and larger ngrams if desired)
                // are stored directly in index, and a bigram is
                // represented as a single hash in the index,
                // as is a unigram, we can just do a quick check
                // for them.

                if (t.size() == 1)
                {
                    const std::string& q = alex::crypt::hexdigest<alex::globals::digest_size>(t[0] + suffix);
                    const uint32_t idx = _ph.search(q);
                    const uint32_t value = alex::hash::jenkinsHash(q) % _maxHash;

                    if (_hashBits.get(_hashWidth * idx, _hashWidth) == value)
                        return _postings[idx].size();
                    else // a sub-term (i.e., word) is not in ph, so its conjunction can't be either
                        return 0;
                }

                // take care of more complicated case
                // we not only make sure that all the bigrams are present,
                // which must be true for a phrase to exist in an index,
                // but we also make sure that at least one of the candidates
                // is smaller than a certain number determined by the
                // expected maximum size of a term of a given size with
                // a random shuffling of posting positions of _maxOffset units max
                //
                // degenerate case is when _maxOffset = 0 -> exact matches.
                // having such precise information is, however, a security vulnerability.
                //
                // also, this will be very slow anyway -- not sure it scales to
                // docs with large postings. this research will reveal answer hopefully.
                size_t freq = std::numeric_limits<uint32_t>::max();
                std::vector<std::vector<uint32_t>> postings;
                for (const auto& x : t)
                {
                    const std::string& q = alex::crypt::hexdigest<alex::globals::digest_size>(x + suffix);
                    const uint32_t idx = _ph.search(q);
                    const uint32_t value = alex::hash::jenkinsHash(q) % _maxHash;

                    // a sub-term (i.e., word) is not in ph, so its conjunction can't be either
                    if (_hashBits.get(_hashWidth * idx, _hashWidth) == value)
                        freq = std::min(_postings[idx].size(), freq);
                    else
                        return 0;
                    postings.push_back(_postings[idx]);
                }

                std::set<std::pair<uint32_t, uint32_t>> candidates;
                alex::utils::cover2(postings, 0, std::numeric_limits<uint32_t>::max(),
                                   std::numeric_limits<uint32_t>::min(), candidates, 2*_maxOffset + t.size(), freq);

                return std::min(candidates.size(), freq);
            };

            bool contains(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                return getFrequency(t) != 0;
            };

            bool containsAny(const alex::index::HiddenQuery& q) const
            {
                for (const auto& t : q.terms)
                {
                    if (contains(t))
                        return true;
                }
                return false;
            };
            bool containsAll(const alex::index::HiddenQuery& q) const
            {
                for (const auto& t : q.terms)
                {
                    if (!contains(t))
                        return false;
                }

                return true;
            };

        private:
            static const unsigned int MAX_BYTE = (1 << CHAR_BIT);
        
            uint32_t _maxHash;
            alex::hash::ICmph _ph;
        
            alex::collections::BitArray _hashBits;
            uint32_t _hashWidth;
            uint32_t _approximateWords;

            uint32_t _numPostings;
            std::vector<std::vector<uint32_t>> _postings;
            
            std::string _ref;
            uint32_t _maxOffset;
    };

    const char PsiPost::HEADER[] = "psip";
}}

#endif