#ifndef __PSI_BLOCK_H__
#define __PSI_BLOCK_H__

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

namespace alex { namespace index
{
    // Perfect secure index (blocks)
    class PsiBlock: public ISecureIndex
    {
        friend class PsiBlockBuilder;

        public:
            static const uint8_t VERSION = 1;
            static const char HEADER[];

            PsiBlock() : _hashWidth(0), _maxHash(0), _blockWidth(0), _blockSize(0), _approximateWords(0) {};
            PsiBlock(const PsiBlock& copy) :
                _maxHash(copy._maxHash),
                _ph(copy._ph),
                _hashWidth(copy._hashWidth),
                _blockWidth(copy._blockWidth),
                _hashBits(copy._hashBits),
                _blockBits(copy._blockBits),
                _blockSize(copy._blockSize),
                _approximateWords(copy._approximateWords),
                _ref(copy._ref) {};
            PsiBlock(PsiBlock&& src) :
                _maxHash(src._maxHash),
                _ph(std::move(src._ph)),
                _hashWidth(src._hashWidth),
                _blockWidth(src._blockWidth),
                _hashBits(std::move(src._hashBits)),
                _blockBits(std::move(src._blockBits)),
                _ref(std::move(src._ref)),
                _blockSize(src._blockSize),
                _approximateWords(src._approximateWords) {};
            virtual ~PsiBlock()
            {
                clear();
            };

            PsiBlock& operator=(PsiBlock&& other)
            {
                if (this != &other)
                {
                    clear();

                    _blockBits = std::move(other._blockBits);
                    _hashBits = std::move(other._hashBits);                   
                    _ph = std::move(other._ph);
                    _ref = std::move(other._ref);

                    _blockSize = other._blockSize;
                    _blockWidth = other._blockWidth;
                    _hashWidth = other._hashWidth;
                    _maxHash = other._maxHash;
                    _approximateWords = other._approximateWords;

                    other._approximateWords = 0;
                    other._hashWidth = 0;
                    other._blockWidth = 0;
                    other._maxHash = 0;
                    other._blockSize = 0;
                }

                return *this;
            };

            PsiBlock& operator=(const PsiBlock& other)
            {
                if (this != &other)
                {
                    clear();

                    _blockBits = other._blockBits;
                    _hashBits = other._hashBits;
                    _ph = other._ph;
                    _ref = other._ref;

                    _blockSize = other._blockSize;
                    _blockWidth = other._blockWidth;
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
                    throw std::exception("Unexpected Verion");

                // get reference (e.g., url) for encrypted document
                _ref = readString(file);

                _approximateWords = readVarU32(file);

                // get hash of encrypted document
                // _refHash = readString(file);

                // de-serialize perfect hash function
                _ph.read(file);

                // read hash entries
                // get number of bits per hash entry (false positive rate = 1/2^width)
                _hashWidth = readVarU32(file);

                // derive maximum hash value from fp bits
                _maxHash = (1 << _hashWidth);

                // read bit array for compactly (packed) storing hash entries
                readBitArray(file, _hashBits);

                // read block data
                _blockWidth = readVarU32(file);
                _blockSize = readVarU32(file);
                readBitArray(file, _blockBits);
            };
            void write(std::ostream& file)
            {
                using namespace alex::io::binary;

                writeString(file, HEADER);
                writeU8(file, VERSION);

                // write reference (e.g., url) for encrypted document
                writeString(file, _ref);

                writeVarU32(file, _approximateWords);

                // write hash of encrypted document
                // writeString(file, _refHash);

                // serialize perfect hash function
                _ph.write(file);

                // write hash entries
                writeVarU32(file, _hashWidth);

                // write bit array for compactly (packed) hash entries
                writeBitArray(file, _hashBits);

                // write block data
                writeVarU32(file, _blockWidth);
                writeVarU32(file, _blockSize);
                writeBitArray(file, _blockBits);
            };
            bool blocks(const alex::index::HiddenQuery::HiddenTerm& t, BitArray& blocks) const
            {
                blocks.resizeBits(_blockWidth);
                blocks.setAll();
                const std::string suffix = "|" + _ref;

                for (const auto& x : t)
                {
                    const std::string& q = alex::crypt::hexdigest<alex::globals::digest_size>(x + suffix);

                    const uint32_t idx = _ph.search(q);
                    const uint32_t value = alex::hash::jenkinsHash(q) % _maxHash;

                    if (_hashBits.get(_hashWidth * idx, _hashWidth) == value)
                    {
                        const unsigned int offset = _blockWidth * idx;
                        bool isAnyTrue = false;
                        for (unsigned int i = offset, count = 0; i < offset + _blockWidth; ++i, ++count)
                        {
                            const bool tmp = blocks.getBit(count) & _blockBits.getBit(i);
                            isAnyTrue |= tmp;
                            if (!tmp)
                                blocks.clearBit(count);
                        }
                        if (!isAnyTrue)     // a sub-term was in the hash, but did not occur in the same block
                            return false;   // as any other, thus term as a whole wasn't found (must be in same block)
                    }
                    else // a sub-term (i.e., word) is not in any block, so its conjunction can't be either
                        return false;
                }
                return true;
            };
            
            uint32_t getBlockSize() const { return _blockSize; };
            uint32_t getBlockWidth() const { return _blockWidth; };
            uint32_t getHashWidth() const { return _hashWidth; };
            uint32_t getApproximateIndexes() const { return _hashBits.size() / _hashWidth; };
            alex::hash::ICmph getCmph() const { return _ph; };

            std::string getReference() const { return _ref; };
            void setReference(const std::string& ref) { _ref = ref; };
            TSecureIndex getType() const { return TSecureIndex::PsiBlock; };
            unsigned int getApproximateWords() const { return _blockSize * _blockWidth; };
            bool hasProximity() const { return true; };
            bool hasFrequency() const { return true; };
            void read(const boost::filesystem::path& infile) { read(std::ifstream(infile.string(), std::ios::binary)); };
            void write(const boost::filesystem::path& outfile) { write(std::ofstream(outfile.string(), std::ios::binary)); };
            std::vector<uint32_t> getLocations(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                auto f = alex::stochastic::Entropy<>();
                std::vector<uint32_t> results;
                alex::collections::BitArray blocks;
                if (this->blocks(t, blocks))
                {
                    for (size_t i = 0, count = 0; i < blocks.size(); ++i, ++count)
                    {
                        if (blocks.getBit(i))
                            results.push_back(count * getBlockSize() + f.get() % getBlockSize());
                    }
                }
                return results;
            };

            double getMinPwDistScoreAlg2(const alex::index::HiddenQuery& q, double alpha = 0.3, double beta = 1) const
            {
                std::vector<alex::collections::BitArray> terms;
                for (const auto& t : q.terms)
                {
                    alex::collections::BitArray blocks;
                    // only consider terms present in at least one of the blocks
                    if (this->blocks(t, blocks))
                        terms.push_back(blocks);
                }

                const int n = (int)terms.size();
                auto m = alex::utils::makeMatrixInt(n, alex::globals::max_min_pair_dist);
                auto last = std::vector<int>(n, -1);

                for (int i = 0; i < _blockWidth; ++i)
                {
                    for (int j = 0; j < n; ++j)
                    {
                        if (terms[j].getBit(i) != 0)
                        {
                            last[j] = i;
                            for (int k = 0; k < n; ++k)
                            {
                                if (k == j || last[k] == -1)
                                    continue;

                                const int d = std::abs(i - last[k]);
                                if (d < m[j][k])
                                    m[j][k] = m[k][j] = d;
                            }
                        }            
                    }
                }

                if (n <= 1)
                    return std::log(alex::globals::alpha_min_pair + 1);
                else
                {
                    double sum = 0;
                    for (int i = 1; i < n; ++i)
                        for (size_t j = 0; j < i; ++j)
                            sum += m[i][j] * _blockSize;

                    return std::log(alex::globals::alpha_min_pair + std::exp(-alex::globals::beta_min_pair * sum));
                }
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
                    {
                       ss << _hashBits.getBit(i * _hashWidth + j);
                    }
                    ss << "} :: blocks={";

                    for (unsigned int j = 0; j < _blockWidth; ++j)
                    {
                        ss << _blockBits.getBit(i * _blockWidth + j);
                    }
                    ss << "}\n";
                }
                ss << std::endl;

                return ss.str();
            };
            void clear()
            {
                if (_hashBits.size() != 0)
                {
                    _blockWidth = 0;
                    _hashWidth = 0;
                    _maxHash = 0;
                    _hashBits.clear();
                    _blockBits.clear();
                    _ref = "";
                    _ph.clear();
                    _approximateWords = 0;
                }
            };
            unsigned int getFrequency(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                BitArray blocks = BitArray::makeBits(_blockWidth, true);
                const std::string suffix = "|" + _ref;

                for (const auto& x : t)
                {
                    const std::string q = alex::crypt::hexdigest<alex::globals::digest_size>(x + suffix);
                    const uint32_t idx = _ph.search(q);
                    const uint32_t value = alex::hash::jenkinsHash(q) % _maxHash;

                    if (_hashBits.get(_hashWidth * idx, _hashWidth) == value)
                    {
                        size_t offset = _blockWidth * idx;
                        bool isAnyTrue = false;
                        for (unsigned int i = offset, count = 0; i < offset + _blockWidth; ++i, ++count)
                        {
                            bool v = blocks.getBit(count) & _blockBits.getBit(i);
                            isAnyTrue |= v;
                            if (!v)
                                blocks.clearBit(count);
                        }
                        if (!isAnyTrue) // a sub-term was in the hash, but did not occur in the same block
                        {               // as any other, thus term as a whole wasn't found (must be in same block)
                            return 0;
                        }
                    }
                    else // a bigram in the term (phrase) is not in any block
                    {
                        return 0;
                    }
                }

                size_t freq = 0;
                for (size_t i = 0; i < blocks.size(); ++i)
                {
                    if (blocks.getBit(i))
                        ++freq;
                }
                return freq;
            };
            bool contains(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                if (t.empty())
                    throw std::exception("No Terms");

                const std::string suffix = "|" + _ref;

                // handle 1-grams and 2-grams quickly
                if (t.size() == 1)
                {
                    const std::string q = alex::crypt::hexdigest<alex::globals::digest_size>(t[0] + suffix);
                    const uint32_t idx = _ph.search(q);
                    const uint32_t value = alex::hash::jenkinsHash(q) % _maxHash;
                    return _hashBits.get(_hashWidth * idx, _hashWidth) == value;
                }

                BitArray blocks = BitArray::makeBits(_blockWidth, true);
                for (const auto& x : t)
                {
                    const std::string q = alex::crypt::hexdigest<alex::globals::digest_size>(x + suffix);
                    const uint32_t idx = _ph.search(q);
                    const uint32_t value = alex::hash::jenkinsHash(q) % _maxHash;

                    if (_hashBits.get(_hashWidth * idx, _hashWidth) == value)
                    {
                        const size_t offset = _blockWidth * idx;
                        bool isAnyTrue = false;
                        for (unsigned int i = offset, count = 0; i < offset + _blockWidth; ++i, ++count)
                        {
                            bool v = blocks.getBit(count) & _blockBits.getBit(i);
                            isAnyTrue |= v;
                            if (!v)
                                blocks.clearBit(count);
                        }
                        if (!isAnyTrue)
                            return false;
                    }
                    else // a sub-term (i.e., word) is not in any block, so its conjunction can't be either
                        return false;
                }

                return true;
            };
            bool containsAny(const alex::index::HiddenQuery& q) const
            {
                for (const auto& t : q.terms)
                {
                    if (contains(t))
                    {
                        return true;
                    }
                }
                return false;
            };
            bool containsAll(const alex::index::HiddenQuery& q) const
            {
                for (const auto& t : q.terms)
                {
                    if (!contains(t))
                    {
                        return false;
                    }
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

            alex::collections::BitArray _blockBits;
            uint32_t _blockWidth;
            uint32_t _blockSize;

            std::string _ref;
    };

    const char PsiBlock::HEADER[] = "psib";
}}

#endif