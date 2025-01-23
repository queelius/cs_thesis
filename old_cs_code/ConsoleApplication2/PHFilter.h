#ifndef __PH_FILTER_H__
#define __PH_FILTER_H__

#include <string>
#include <ostream>
#include <istream>
#include "CompactArray.h"
#include "PerfectHashFn.h"
#include "BinaryIO.h"
#include "Hash.h"
#include "Entropy.h"

namespace alex { namespace collections
{
    class PHFilter
    {
    public:
        const static uint8_t VERSION = 1;
        const static char HEADER[];

        template <class E>
        void build(char** elements, size_t size, E entropy = alex::statistics::Entropy<float>, float loadFactor = 1, uint32_t falsePositiveBits = 10, cmph_cpp::CMPH_ALGO algo = cmph_cpp::CMPH_CHD_PH, bool verbose = false)
        {
            size_t sz = _ph.build(elements, size, verbose, loadFactor, algo);

            _hashes.initialize(sz, 0, (1 << falsePositiveBits));
            if (loadFactor < 1)
            {
                auto& bitArray = _hashes.getBitArray();
                auto MAX_BYTE = (1 << CHAR_BIT);
                for (unsigned int i = 0; i < bitArray.bytes(); ++i)
                    bitArray.setByte(i, entropy.get() % MAX_BYTE);
            }

            for (size_t i = 0; i < size; ++i)
                _hashes.set(_ph.search(elements[i]), alex::fast_hash::jenkinsHash(elements[i]) % _hashes.max());
        };

        PHFilter() {};

        PHFilter(const PHFilter& copy) :
            _hashes(copy._hashes),
            _ph(copy._ph)
        {
        };

        PHFilter(PHFilter&& src) :
            _hashes(std::move(src._hashes)),
            _ph(std::move(src._ph))
        {};

        PHFilter& operator=(PHFilter&& other)
        {
            if (this != &other)
            {
                clear();
                _ph = std::move(other._ph);
                _hashes = std::move(other._hashes);
            }

            return *this;
        };

        PHFilter& operator=(const PHFilter& other)
        {
            if (this != &other)
            {
                clear();
                _ph = other._ph;
                _hashes = other._hashes;
            }

            return *this;
        };

        void readBinary(std::istream& file)
        {
            const auto& hdr = alex::io::binary::readString(file);
            if (hdr != HEADER)
                throw std::exception(("Unexpected Header: " + hdr).c_str());
            uint8_t version = alex::io::binary::readU8(file);
            if (version != VERSION)
                throw std::exception(("Unexpected Version: " + std::to_string(version)).c_str());

            _ph.readBinary(file); // de-serialize perfect hash function
            _hashes.readBinary(file);
        };

        void writeBinary(std::ostream& file)
        {
            alex::io::binary::writeString(file, HEADER);
            alex::io::binary::writeU8(file, VERSION);
            _ph.writeBinary(file); // serialize perfect hash function
            _hashes.writeBinary(file);
        };

        virtual ~PHFilter() { clear(); };

        void clear()
        {
            if (_hashes.size() != 0)
            {
                _hashes.clear();
                _ph.clear();
            }
        };

        std::pair<bool, uint32_t> contains(const std::string& key) const
        {
            uint32_t idx = label(key);
            return std::make_pair(_hashes.get(idx) == (alex::fast_hash::jenkinsHash(key) % _hashes.max()), idx);
        };

        uint32_t label(const std::string& key) const { return _ph.search(key); };
        double falsePositiveRate() const { return 1.0 / _hashes.max(); };
        
        uint32_t bitsPerHash() const { return _hashes.bitsPerIndex(); };
        uint32_t approximateSize() const { return _hashes.size(); };

        alex::hash::PerfectHashFn getPerfectHash() const { return _ph; };
        alex::collections::CompactArray getHashes() const { return _hashes; };
        alex::collections::CompactArray& getHashes() { return _hashes; };

    private:
        alex::hash::PerfectHashFn _ph;
        alex::collections::CompactArray _hashes;
    };

    const char PHFilter::HEADER[] = "phf";
}}

#endif