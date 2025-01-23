#ifndef __PSI_FREQ_H__
#define __PSI_FREQ_H__

#include <iostream>
#include <string>
#include <ostream>
#include "BitArray.h"
#include "ICmph.h"
#include "Hash.h"
#include "Utils.h"
#include "BinaryIO.h"
#include <iomanip>
#include <cstdint>
#include "HiddenQuery.h"
#include "ISecureIndex.h"
#include <vector>

namespace alex { namespace index
{
    // Perfect secure index
    class PsiFreq : public ISecureIndex
    {
        friend class PsiFreqBuilder;
        friend class PsiMin;
        friend class PsiFreqBsiBlockBuilder;

        public:
            const static uint8_t VERSION = 1;
            const static char HEADER[];

            PsiFreq() : _hashWidth(0), _maxHash(0) {};
            PsiFreq(const PsiFreq& copy) :
                _hashBits(copy._hashBits),
                _hashWidth(copy._hashWidth),
                _freqWidth(copy._freqWidth),
                _freqBits(copy._freqBits),
                _maxHash(copy._maxHash),
                _ph(copy._ph),
                _approximateWords(copy._approximateWords),
                _ref(copy._ref)
            {
            };
            PsiFreq(PsiFreq&& src) :
                _hashBits(std::move(src._hashBits)),
                _hashWidth(src._hashWidth),
                _freqWidth(src._freqWidth),
                _freqBits(std::move(src._freqBits)),
                _maxHash(src._maxHash),
                _ph(std::move(src._ph)),
                _approximateWords(src._approximateWords),
                _ref(std::move(src._ref)) {};

            PsiFreq& operator=(PsiFreq&& other)
            {
                if (this != &other)
                {
                    clear();

                    _freqBits = std::move(other._freqBits);
                    _hashBits = std::move(other._hashBits);                   
                    _ph = std::move(other._ph);
                    _ref = std::move(other._ref);

                    _freqWidth = other._freqWidth;
                    _hashWidth = other._hashWidth;
                    _maxHash = other._maxHash;
                    _approximateWords = other._approximateWords;
                    other._approximateWords = 0;
                    other._hashWidth = 0;
                    other._freqWidth = 0;
                    other._maxHash = 0;
                }

                return *this;
            };

            PsiFreq& operator=(const PsiFreq& other)
            {
                if (this != &other)
                {
                    clear();
                    _freqBits = other._freqBits;
                    _hashBits = other._hashBits;
                    _ph = other._ph;
                    _ref = other._ref;
                    _freqWidth = other._freqWidth;
                    _hashWidth = other._hashWidth;
                    _maxHash = other._maxHash;
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
                    throw std::exception("Unexpected Version");

                // get reference (e.g., url) for encrypted document
                _ref = readString(file);

                // get hash of encrypted document
                // _refHash = readString(file);

                // de-serialize perfect hash function
                _ph.read(file);

                // read hash entries
                _hashWidth = readVarU32(file); // get number of bits per hash entry (false positive rate = 1/2^width)
                _maxHash = (1 << _hashWidth); // derive maximum hash value from fp bits
                readBitArray(file, _hashBits); // read bit array for compactly (packed) storing hash entries

                _approximateWords = readVarU32(file);

                // read frequency entries
                _freqWidth = readVarU32(file);
                readBitArray(file, _freqBits);
            };
            void write(std::ostream& file)
            {
                using namespace alex::io::binary;

                writeString(file, HEADER);
                writeU8(file, VERSION);

                // write reference (e.g., url) for encrypted document
                writeString(file, _ref);

                // write hash of encrypted document
                // writeString(file, _refHash);

                // serialize perfect hash function
                _ph.write(file);

                // write hash entries
                writeVarU32(file, _hashWidth);

                // write bit array for compactly (packed) hash entries
                writeBitArray(file, _hashBits);

                // write approximate number of words in index
                writeVarU32(file, _approximateWords);

                // write block data
                writeVarU32(file, _freqWidth);
                writeBitArray(file, _freqBits);
            };
            virtual ~PsiFreq() { clear(); };

            void dump(int depth = 0)
            {
                std::cout << std::setw(depth) << "" << "PsiFreq" << std::endl;
                std::cout << std::setw(depth) << "" << "freqWidth: " << _freqWidth << std::endl;
                std::cout << std::setw(depth) << "" << "hashWidth: " << _hashWidth << std::endl;
                std::cout << std::setw(depth) << "" << "maxHash: " << _maxHash << std::endl;
                std::cout << std::setw(depth) << "" << "approximateWords: " << _approximateWords << std::endl;
                std::cout << std::setw(depth) << "" << "ref: " << _ref << std::endl;
                std::cout << std::setw(depth) << "" << "hashBits:" << std::endl;
                _hashBits.dump(depth+1);
                std::cout << std::setw(depth) << "" << "freqBits:" << std::endl;
                _freqBits.dump(depth+1);
                std::cout << std::setw(depth) << "" << "ph:" << std::endl;
                _ph.dump(depth+1);
            };

            double getMinPairwiseScore(const HiddenQuery& q) const
            {
                int k = 0;
                for (const auto& t : q.terms)
                {
                    if (contains(t)) 
                        ++k;
                }

                // this is just returning the expected min pairwise
                // distance for a doc of size L with k
                // matching terms (pairwise dist only
                // applies to matching terms) normalized
                // by number of matching terms.
                //
                // only for very small docs will this
                // have a practical effect, since for
                // large docs L(k-1)/6 is going to be
                // very large.
                const double L = getApproximateWords();
                if (k <= 1)
                    return L;
                else
                    return L * (k - 1) / 6;
            };

            void clear()
            {
                if (_hashBits.size() != 0)
                {
                    _freqWidth = 0;
                    _hashWidth = 0;
                    _maxHash = 0;
                    _hashBits.clear();
                    _freqBits.clear();
                    _ref = "";
                    _ph.clear();
                    _approximateWords = 0;
                }
            };
            unsigned int getApproximateWords() const { return _approximateWords; }
            bool hasFrequency() const { return true; }
            bool hasProximity() const { return false; }
            TSecureIndex getType() const { return TSecureIndex::PsiFrequency; }
            void read(const boost::filesystem::path& infile)
            {
                read(std::ifstream(infile.string(), std::ios::binary));
            };
            void write(const boost::filesystem::path& infile)
            {
                write(std::ofstream(infile.string(), std::ios::binary));
            };           
            unsigned int getFrequency(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                const std::string suffix = "|" + _ref;
                uint32_t freq = std::numeric_limits<uint32_t>::max();

                for (const auto& x : t)
                {
                    const std::string& q = alex::crypt::hexdigest<alex::globals::digest_size>(x + suffix);
                    const uint32_t idx = _ph.search(q);
                    const uint32_t value = alex::hash::jenkinsHash(q) % _maxHash;

                    if (_hashBits.get(_hashWidth * idx, _hashWidth) == value)
                        freq = std::min(_freqBits.get(_freqWidth * idx, _freqWidth), freq);
                    else // a sub-term (i.e., word) is not in ph, so its conjunction can't be either
                        return 0;
                }

                return freq;
            };
            bool contains(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                if (t.empty())
                    throw std::exception("No Terms");

                const std::string suffix = "|" + _ref;
                for (const auto& x : t)
                {
                    const std::string& q = alex::crypt::hexdigest<alex::globals::digest_size>(x + suffix);
                    const uint32_t idx = _ph.search(q);
                    const uint32_t value = alex::hash::jenkinsHash(q) % _maxHash;

                    // a sub-term (i.e., word) is not in ph, so its conjunction can't be either
                    if (_hashBits.get(_hashWidth * idx, _hashWidth) != value)
                        return false;
                }

                return true;
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
            std::string getReference() const
            {
                return _ref;
            };
            void setReference(const std::string& ref)
            {
                _ref = ref;
            };
            std::string toString() const
            {
                std::stringstream ss;
                ss << _ref << std::endl;
                for (unsigned int i = 0; i < _hashBits.size() / _hashWidth; ++i)
                {
                    ss << "    " << std::right << std::setw(8) << i << " -> hash={";
                    for (unsigned int j = 0; j < _hashWidth; ++j)
                    {
                       ss << _hashBits.getBit(i * _hashWidth + j);
                    }
                    ss << "} :: freq=" << _hashBits.get(i * _freqWidth, _freqWidth) << std::endl;
                }
                ss << std::endl;
                return ss.str();
            };
            std::string getPreferredFileExtension() const { return HEADER; };

            uint32_t getHashWidth() const { return _hashWidth; };
            uint32_t getFreqWidth() const { return _freqWidth; };
            uint32_t getApproximateIndexes() const { return _hashBits.size() / _hashWidth; };

        private:
            static const unsigned int MAX_BYTE = (1 << CHAR_BIT);
        
            alex::hash::ICmph _ph;

            uint32_t _maxHash;
            alex::collections::BitArray _hashBits;
            uint32_t _hashWidth;

            uint32_t _approximateWords;
            alex::collections::BitArray _freqBits;
            uint32_t _freqWidth;

            std::string _ref;
    };

    const char PsiFreq::HEADER[] = "psif";
}}

#endif