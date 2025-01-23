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
#include "PsiBool.h"

namespace alex { namespace index
{
    // Perfect secure index (blocks)
    class PsiBlock: public ISecureIndex
    {
        friend class PsiBlockBuilder;

        public:
            static const uint8_t VERSION = 1;
            static const char HEADER[];

            PsiBlock() : _blockWidth(0), _blockSize(0) {};
            PsiBlock(const PsiBlock& copy) :
                _psiBool(copy._psiBool),
                _blockWidth(copy._blockWidth),
                _blockBits(copy._blockBits),
                _blockSize(copy._blockSize),
            PsiBlock(PsiBlock&& src) :
                _blockWidth(src._blockWidth),
                _blockBits(std::move(src._blockBits)),
                _blockSize(src._blockSize),
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
                    _blockSize = other._blockSize;
                    _blockWidth = other._blockWidth;
                    other._blockWidth = 0;
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
                    _blockSize = other._blockSize;
                    _blockWidth = other._blockWidth;
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

                _psiBool.read(file);

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

                _psiBool.write(file);

                // write block data
                writeVarU32(file, _blockWidth);
                writeVarU32(file, _blockSize);
                writeBitArray(file, _blockBits);
            };
            bool blocks(const alex::index::HiddenQuery::HiddenTerm& t, BitArray& blocks) const
            {
                blocks.resizeBits(_blockWidth);
                blocks.setAll();
                const std::string suffix = "|" + _psiBool.getReference();

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

            };

            bool containsAll(const alex::index::HiddenQuery& q) const
            {
                return
            };

        private:
            static const unsigned int MAX_BYTE = (1 << CHAR_BIT);
            alex::collections::BitArray _blockBits;
            uint32_t _blockWidth;
            uint32_t _blockSize;
            PsiBool _psiBool;
    };

    const char PsiBlock::HEADER[] = "psiblock";
}}

#endif