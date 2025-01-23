#ifndef __BSI_BLOCK_H__
#define __BSI_BLOCK_H__

#include <iostream>
#include <string>
#include <ostream>
#include <iomanip>
#include <cstdint>
#include <functional>
#include "BitArray.h"
#include "Cfg.h"
#include "Hash.h"
#include "Utils.h"
#include "BinaryIO.h"
#include "HiddenQuery.h"
#include "ISecureIndex.h"
#include "bloom_filter.hpp"

namespace alex { namespace index
{
    // Bloom filter secure index (blocks)
    class BsiBlock: public ISecureIndex
    {
        friend class BsiBlockBuilder;

        public:
            static const uint8_t VERSION = 1;
            static const char HEADER[];

            BsiBlock() : _blockSize(0), _approximateWords(0) {};
            BsiBlock(const BsiBlock& copy) :
                _approximateWords(copy._approximateWords),
                _blockSize(copy._blockSize),
                _ref(copy._ref),
                _blooms(copy._blooms) {};

            BsiBlock(BsiBlock&& src) :
                _approximateWords(src._approximateWords),
                _ref(std::move(src._ref)),
                _blockSize(src._blockSize),
                _blooms(std::move(src._blooms)) {};

            BsiBlock& operator=(BsiBlock&& other)
            {
                if (this != &other)
                {
                    clear();

                    _blooms = std::move(other._blooms);
                    _ref = std::move(other._ref);
                    _blockSize = other._blockSize;
                    _approximateWords = other._approximateWords;

                    other._approximateWords = 0;
                    other._blockSize = 0;
                }

                return *this;
            };

            BsiBlock& operator=(const BsiBlock& other)
            {
                if (this != &other)
                {
                    clear();

                    _blooms = other._blooms;
                    _ref = other._ref;
                    _blockSize = other._blockSize;
                    _approximateWords = other._approximateWords;
                }

                return *this;
            };

            virtual ~BsiBlock()
            {
                clear();
            };

            double getMinPwDistScoreAlg2(const alex::index::HiddenQuery& q) const
            {
                std::vector<alex::collections::BitArray> terms;
                for (auto t : q.terms)
                {
                    alex::collections::BitArray blocks;
                    // only consider terms present in at least one of the blocks
                    if (this->blocks(t, blocks))
                        terms.push_back(blocks);
                }

                const int n = (int)terms.size();
                auto m = alex::utils::makeMatrixInt(n, _blooms.size());
                auto last = std::vector<int>(n, -1);

                for (int i = 0; i < _blooms.size(); ++i)
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

                int sum = 0;
                for (int i = 1; i < n; ++i)
                {
                    for (size_t j = 0; j < i; ++j)
                        sum += m[i][j] * _blockSize;
                }

                return sum;
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

                // get hash of encrypted document
                // _refHash = readString(file);

                _approximateWords = readVarU32(file);

                _blockSize = readVarU32(file);

                // de-serialize bloom filters
                const size_t bloomNum = readVarU32(file);
                _blooms.reserve(bloomNum);
                for (size_t i = 0; i < bloomNum; ++i)
                {
                    bf::bloom_filter bf;
                    alex::io::binary::readVecU32(file, bf.salt_);

                    bf.salt_count_ = bf.salt_.size();
                    bf.table_size_ = alex::io::binary::readU32(file);
                    
                    if (bf.bit_table_ != nullptr)
                        delete [] bf.bit_table_;
                    
                    bf.raw_table_size_ = alex::io::binary::readArrayU8(file, bf.bit_table_);

                    _blooms.push_back(bf);
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

                writeVarU32(file, _blockSize);

                // write hash of encrypted document
                // writeString(file, _refHash);

                writeVarU32(file, (uint32_t)_blooms.size());
                for (const auto& bloom : _blooms)
                {
                    alex::io::binary::writeVecU32(file, bloom.salt_);
                    alex::io::binary::writeU32(file, (uint32_t)bloom.table_size_);
                    alex::io::binary::writeArrayU8(file, (uint32_t)bloom.raw_table_size_, (uint8_t*)bloom.bit_table_);
                }
                writeU32(file, _blockSize);
            };

            bool blocks(const alex::index::HiddenQuery::HiddenTerm& t, BitArray& blocks) const
            {
                blocks.resizeBits(_blooms.size());
                blocks.setAll();
                const std::string suffix = "|" + _ref;

                for (const auto& x : t)
                {
                    const std::string q = alex::crypt::hexdigest<alex::globals::digest_size>(x + suffix);
                    bool isAnyTrue = false;
                    for (size_t i = 0; i < _blooms.size(); ++i)
                    {
                        bool v = _blooms[i].contains(q);
                        isAnyTrue |= v;
                        if (!v)
                            blocks.clearBit(i);
                    }
                    if (!isAnyTrue)
                        return false;
                }
                return true;
            };
            
            uint32_t getBlockSize() const { return _blockSize; };
            std::string getReference() const { return _ref; };
            void setReference(const std::string& ref) { _ref = ref; };
            TSecureIndex getType() const { return TSecureIndex::BsiBlock; };
            unsigned int getApproximateWords() const { return _approximateWords; };
            bool hasProximity() const { return true; };
            bool hasFrequency() const { return true; };
            void read(const boost::filesystem::path& inFile) { read(std::ifstream(inFile.string(), std::ios::binary)); };
            void write(const boost::filesystem::path& outFile) { write(std::ofstream(outFile.string(), std::ios::binary)); };
            std::vector<uint32_t> getLocations(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                auto& f = alex::stochastic::Entropy<>();
                std::vector<uint32_t> results;
                alex::collections::BitArray blocks;
                if (this->blocks(t, blocks))
                {
                    for (unsigned int i = 0, count = 0; i < blocks.size(); ++i, ++count)
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
                ss << "Reference: " << _ref << std::endl;
                ss << "ApproximateWords: " << _approximateWords << std::endl;
                ss << "BlockSize: " << _blockSize << std::endl;
                for (auto b : _blooms)
                {
                    
                    ss << "TableSize: " << b.table_size_ << std::endl;
                    ss << "RawTableSize: " << b.raw_table_size_ << std::endl;
                    ss << "SaltCount: " << b.salt_count_ << std::endl;
                    ss << "Salts: ";
                    for (size_t i = 0; i < b.salt_.size(); ++i)
                    {
                        ss << b.salt_[i] << " ";
                    }
                    ss << std::endl;
                    ss << "BitTable:\n";
                    for (size_t i = 0; i < b.raw_table_size_; ++i)
                    {
                        if (i != 0 && i % 64 == 0)
                        {
                            ss << std::endl;
                        }
                        ss << std::setw(6) << (uint32_t)b.bit_table_[i];
                    }
                    ss << std::endl;

                }
                return ss.str();
            };
            void clear()
            {
                if (!_blooms.empty())
                {
                    _approximateWords = 0;
                    _blooms.clear();
                    _blooms.shrink_to_fit();
                    _blockSize = 0;
                    _ref = "";
                }
            };
            unsigned int getFrequency(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                const std::string& suffix = "|" + _ref;
                auto places = alex::collections::BitArray::makeBits(_blooms.size(), true);

                for (const auto& w : t)
                {
                    bool isAnyTrue = false;
                    const auto& s = alex::crypt::hexdigest<alex::globals::digest_size>(w + suffix);
                    for (size_t i = 0; i < _blooms.size(); ++i)
                    {
                        bool v = _blooms[i].contains(s);
                        isAnyTrue |= v;
                        if (!v)
                            places.clearBit(i);
                    }
                    if (!isAnyTrue)
                        return 0;
                }

                size_t freq = 0;
                for (size_t i = 0; i < places.size(); ++i)
                {
                    if (places.getBit(i))
                        ++freq;
                }
                return freq;
            };

            bool contains(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                const std::string& suffix = "|" + _ref;
                auto places = alex::collections::BitArray::makeBits(_blooms.size(), true);

                for (const auto& w : t)
                {
                    bool isAnyTrue = false;
                    const auto& s = alex::crypt::hexdigest<alex::globals::digest_size>(w + suffix);
                    for (size_t i = 0; i < _blooms.size(); ++i)
                    {
                        bool v = _blooms[i].contains(s);
                        isAnyTrue |= v;
                        if (!v)
                            places.clearBit(i);
                    }
                    if (!isAnyTrue)
                        return false;
                }

                for (size_t i = 0; i < places.size(); ++i)
                {
                    if (places.getBit(i))
                        return true;
                }

                return false;
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

            std::vector<bf::bloom_filter> getBloomFilters() const
            {
                return _blooms;
            };

        private:
            static const unsigned int MAX_BYTE = (1 << CHAR_BIT);
        
            std::string _ref;
            uint32_t _blockSize;
            uint32_t _approximateWords;
            std::vector<bf::bloom_filter> _blooms;
    };

    const char BsiBlock::HEADER[] = "bsib";
}}

#endif