#ifndef __COMPACT_ARRAY_H__
#define __COMPACT_ARRAY_H__

#include "BitArray.h"
#include "BinaryIO.h"

namespace alex { namespace collections
{
    class CompactArray
    {
    public:
        const static uint8_t VERSION = 1;

        template <class Iter> static CompactArray make(Iter begin, Iter end)
        {
            CompactArray r;
            r._numIndexes = std::distance(begin, end);
            r._min = std::numeric_limits<uint32_t>::max();
            r._max = std::numeric_limits<uint32_t>::min();

            for (auto i = begin; i != end; ++i)
            {
                uint32_t x = *i;
                if (x > r._max)
                    r._max = x;
                else if (x < r._min)
                    r._min = x;
            }

            if (r._max != r._min)
            {
                if (r._max - r._min == 1)
                    r._bitsPerIndex = 1;
                else
                    r._bitsPerIndex = (uint32_t)std::ceil(std::log(r._max - r._min) / std::log(2));
                r._bitArray = alex::collections::BitArray::makeBits(r._numIndexes * r._bitsPerIndex);

                size_t idx = 0;
                for (auto i = begin; i != end; ++i)
                {
                    r.set(idx, (uint32_t)(*i));
                    ++idx;
                }
            }
            else
            {
                r._bitsPerIndex = 0;
            }

            return r;
        };

        CompactArray() {};

        CompactArray(unsigned int size, uint32_t min = std::numeric_limits<uint32_t>::min(), uint32_t max = std::numeric_limits<uint32_t>::max())
        {
            initialize(size, min, max);
        };

        void initialize(unsigned int size, uint32_t min, uint32_t max)
        {
            if (min > max)
                throw std::exception("Invalid Arguments");

            _numIndexes = size;
            _min = min;
            _max = max;

            if (min == max)
                _bitsPerIndex = 0;
            else
            {
                if (max - min == 1)
                    _bitsPerIndex = 1;
                else
                    _bitsPerIndex = (uint32_t)std::ceil(std::log(max - min) / std::log(2));
                _bitArray = alex::collections::BitArray::makeBits(size * _bitsPerIndex);
            }
        };

        void set(unsigned int idx, uint32_t value)
        {
            if (idx >= _numIndexes)
                throw std::exception("Invalid Index");

            if (value < _min || value > _max)
                throw std::exception("Invalid Value");

            if (_min != _max)
            {
                unsigned int offset = idx * _bitsPerIndex + _bitsPerIndex - 1;
                value -= _min;
                for(unsigned int i = 0; i < _bitsPerIndex; ++i)
                {
                    if(value % 2 == 0)
                        _bitArray.clearBit(offset - i);
                    else
                        _bitArray.setBit(offset - i);
                    value /= 2;
                }
            }
        };

        uint32_t get(unsigned int idx) const
        {
            if (idx >= _numIndexes)
                throw std::exception("Invalid Index");

            if (_min == _max)
                return _min;

            uint32_t v = 0;
            unsigned int offset = idx * _bitsPerIndex;
            for(unsigned int i = 0; i < _bitsPerIndex; ++i)
            {
                v <<= 1;
                v += _bitArray.getBit(offset + i);
            }
            return _min + v;
        };

        size_t size() const
        {
            return _numIndexes;
        };

        uint32_t min() const
        {
            return _min;
        };

        uint32_t max() const
        {
            return _max;
        };

        void read(std::istream& file)
        {
            alex::io::binary::readString(file);
            alex::io::binary::readU8(file);

            _min = alex::io::binary::readVarU32(file);
            _max = alex::io::binary::readVarU32(file);
            _numIndexes = alex::io::binary::readVarU32(file);
            _bitsPerIndex = alex::io::binary::readVarU32(file);

            if (_bitsPerIndex > 0)
                _bitArray.read(file);
        };

        void write(std::ostream& file) const
        {
            alex::io::binary::writeString(file, HEADER);
            alex::io::binary::writeU8(file, VERSION);
            alex::io::binary::writeVarU32(file, (uint32_t)_min);
            alex::io::binary::writeVarU32(file, (uint32_t)_max);
            alex::io::binary::writeVarU32(file, (uint32_t)_numIndexes);
            alex::io::binary::writeVarU32(file, (uint32_t)_bitsPerIndex);

            if (_bitsPerIndex > 0)
                _bitArray.write(file);
        };

    private:
        static const std::string HEADER;
        BitArray _bitArray;
        uint32_t _bitsPerIndex;
        uint32_t _numIndexes;
        uint32_t _min, _max;
    };

    const std::string CompactArray::HEADER = "car";
}}

#endif