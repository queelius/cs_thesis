#ifndef __BIT_ARRAY_H__
#define __BIT_ARRAY_H__

//#define __BIT_ARRAY_ENABLE_CHECKS__

#include <iostream>
#include <sstream>
#include <ostream>
#include <cstdint>
#include <string>
#include <cstring>

namespace alex { namespace collections
{
    /* position of bit within character */
    #define BIT_CHAR(bit)         ((bit) / CHAR_BIT)

    /* array index for character containing bit */
    #define BIT_IN_CHAR(bit)      (1 << (CHAR_BIT - 1 - ((bit)  % CHAR_BIT)))

    /* number of characters required to contain number of bits */
    #define BITS_TO_CHARS(bits)   ((((bits) - 1) / CHAR_BIT) + 1)

    /* most significant bit in a character */
    #define MS_BIT                (1 << (CHAR_BIT - 1))

    class BitArray;

    class BitArrayIndex
    {
    public:
        BitArrayIndex(BitArray* bits, unsigned int index);
        void operator=(bool src);

    private:
        BitArray* _bitArray;
        unsigned int _index;
    };

    class BitArray
    {
    public:
        BitArray(unsigned int bytes, bool zero = true);
        BitArray(void* bytes, unsigned int numBytes);
        BitArray() : _size(0), _bits(NULL) {};
        BitArray(BitArray&& other);
        BitArray(const BitArray& copy);

        static BitArray makeBits(unsigned int bits, bool val = false);
        static BitArray makeBytes(unsigned int bytes, unsigned char val = 0);

        void move(void* bytes, unsigned int numBytes);
        virtual ~BitArray();

        std::string toString() const;
        std::string toHexString() const;

        void dump(int depth = 0)
        {
            std::cout << std::setw(depth) << "" << "BitArray" << std::endl;
            std::cout << std::setw(depth) << "" << "size: " << _size << std::endl;
            std::cout << std::setw(depth) << "" << "bytes:" << std::endl;
            std::cout << std::setw(depth) << "";
            for (size_t i = 0; i < this->bytes(); ++i)
            {
                if (i != 0 && i % 10 == 0)
                {
                    std::cout << std::endl;
                    std::cout << std::setw(depth) << "";
                }
                std::cout << std::setw(4) << (uint32_t)this->_bits[i];
            }
            std::cout << std::endl;
        };

        void setByte(unsigned int byte, unsigned char x) { _bits[byte] = x; };
        unsigned char getByte(unsigned int byte) { return _bits[byte]; };
        unsigned int bytes() const { return (_size / CHAR_BIT); };
        unsigned char*& getBytes() { return _bits; };

        void set(unsigned int offset, uint32_t val, unsigned int bits);
        uint32_t get(unsigned int offset, unsigned int bits = 32) const;
        bool getBit(unsigned int bit) const;

        void resizeBytes(unsigned int bytes, bool zero = true);
        void resizeBits(unsigned int bits, bool zero = true);
        void clear();
        unsigned int size() const { return _size; };

        void setAll();
        void clearAll();
        void setBit(unsigned int bit);
        void clearBit(unsigned int bit);

        BitArrayIndex operator()(unsigned int bit);
        bool operator[](unsigned int bit) const;
        bool operator==(const BitArray& other) const;
        bool operator!=(const BitArray& other) const;
        bool operator<(const BitArray& other) const;
        bool operator<=(const BitArray& other) const;
        bool operator>(const BitArray& other) const;
        bool operator>=(const BitArray& other) const;
        BitArray operator&(const BitArray& other) const;
        BitArray operator^(const BitArray& other) const;
        BitArray operator|(const BitArray& other) const;
        BitArray operator~() const;
        BitArray operator<<(unsigned int shifts) const;
        BitArray operator>>(unsigned int shifts) const;
        BitArray& operator++();
        BitArray& operator++(int);
        BitArray& operator--();
        BitArray& operator--(int);
        BitArray& operator=(const BitArray& src);
        BitArray& operator=(BitArray&& src);
        BitArray& operator&=(const BitArray& src);
        BitArray& operator^=(const BitArray& src);
        BitArray& operator|=(const BitArray& src);
        BitArray& not();
        BitArray& operator<<=(unsigned int shifts);
        BitArray& operator>>=(unsigned int shifts);

    protected:
        unsigned int _size;
        unsigned char* _bits;
    };

    BitArray BitArray::makeBits(unsigned int bits, bool val)
    {
        int bytes = BITS_TO_CHARS(bits);

        BitArray b;
        b._size = bits;
        b._bits = new unsigned char[bytes];
            
        if (val)
            std::fill_n(b._bits, bytes, UCHAR_MAX);
        else
            std::fill_n(b._bits, bytes, 0);

        return b;
    }

    BitArray BitArray::makeBytes(unsigned int bytes, unsigned char val)
    {
        BitArray b;
        b._size = CHAR_BIT * bytes;
        b._bits = new unsigned char[bytes];
            
        if (val)
            std::fill_n(b._bits, bytes, val);
        else
            std::fill_n(b._bits, bytes, 0);

        return b;
    }

    BitArray::BitArray(void* bytes, unsigned int numBytes) :
        _size(numBytes * CHAR_BIT),
        _bits(new unsigned char[numBytes])
    {
        std::memcpy(_bits, bytes, numBytes);
    }

    BitArray::BitArray(const BitArray& copy) :
        _size(copy._size),
        _bits(new unsigned char[copy.bytes()])
    {
        std::memcpy(_bits, copy._bits, copy.bytes());
    };

    BitArray::BitArray(BitArray&& other) :
        _bits(other._bits),
        _size(other._size)
    {
        other._bits = NULL;
        other._size = 0;
    }

    BitArray::BitArray(unsigned int bytes, bool zero) :
        _size(bytes * CHAR_BIT),
        _bits(new unsigned char[bytes])
    {
        if (zero)
        {
            std::fill_n(_bits, bytes, 0);
        }
    }

    void BitArray::move(void* bytes, unsigned int numBytes)
    {
        clear();
        _bits = (unsigned char*)bytes;
        _size = numBytes * CHAR_BIT;
    };

    void BitArray::resizeBytes(unsigned int bytes, bool zero)
    {
        clear();
        _bits = new unsigned char[bytes];
        _size = bytes * CHAR_BIT;

        if (zero)
            std::fill_n(_bits, bytes, 0);
    }

    void BitArray::resizeBits(unsigned int bits, bool zero)
    {
        clear();

        unsigned int size = BITS_TO_CHARS(bits);
        _bits = new unsigned char[size];
        _size = bits;

        if (zero)
            std::fill_n(_bits, size, 0);
    }

    void BitArray::clear()
    {
        if (_bits != NULL)
        {
            delete [] _bits;
            _bits = NULL;
            _size = 0;
        }
    }

    std::string BitArray::toString() const
    {
        std::string result;
        for (unsigned int i = 0; i < _size; ++i)
        {
            if (getBit(i)) { result += "1"; }
            else           { result += "0"; }
        }
        return result;
    }

    BitArray::~BitArray()
    {
        clear();
    }

    std::string BitArray::toHexString() const
    {
        std::stringstream ss;

        int size = BITS_TO_CHARS(_size);

        ss.width(2);
        ss.fill('0');
        ss << std::hex << (int)(_bits[0]);

        for(int i = 1; i < size; i++)
        {
            ss << " ";
            ss.width(2);
            ss.fill('0');
            ss << (int)(_bits[i]);
        }

        return ss.str();
    }

    void BitArray::set(unsigned int offset, uint32_t val, unsigned int bits)
    {
        for(unsigned int i = 0; i < bits; ++i)
        {
            if(val % 2 != 0)
                setBit(offset + bits - i - 1);
            else
                clearBit(offset + bits - i - 1);
            val /= 2;
        }
    }

    uint32_t BitArray::get(unsigned int offset, unsigned int bits) const
    {
        uint32_t v = 0;
        for(unsigned int i = 0; i < bits; ++i)
        {
            v <<= 1;
            v += getBit(offset + i);
        }
        return v;
    }

    void BitArray::setAll()
    {
        int size = BITS_TO_CHARS(_size);
        std::fill_n(_bits, size, UCHAR_MAX);

        /* zero any spare bits so increment and decrement are consistent */
        int bits = _size % CHAR_BIT;
        if(bits != 0)
        {
            unsigned char mask = UCHAR_MAX << (CHAR_BIT - bits);
            _bits[BIT_CHAR(_size - 1)] = mask;
        }
    }

    void BitArray::clearAll()
    {
        int size = BITS_TO_CHARS(_size);
        std::fill_n(_bits, size, 0);
    }

    void BitArray::setBit(unsigned int bit)
    {
#ifdef __BIT_ARRAY_ENABLE_CHECKS__
        if(bit < _size)
        {
            _bits[BIT_CHAR(bit)] |= BIT_IN_CHAR(bit);
        }
#else
        _bits[BIT_CHAR(bit)] |= BIT_IN_CHAR(bit);
#endif
    }

    void BitArray::clearBit(unsigned int bit)
    {
#ifdef __BIT_ARRAY_ENABLE_CHECKS__
        if(bit < _size)
        {
            /* create a mask to zero out desired bit */
            unsigned char mask =  BIT_IN_CHAR(bit);
            mask = ~mask;

            _bits[BIT_CHAR(bit)] &= mask;
        }
#else
        unsigned char mask =  BIT_IN_CHAR(bit);
        mask = ~mask;

        _bits[BIT_CHAR(bit)] &= mask;
#endif
    }

    BitArrayIndex BitArray::operator()(unsigned int bit)
    {
        return BitArrayIndex(this, bit);
    }

    bool BitArray::getBit(unsigned int bit) const
    {
        return((_bits[BIT_CHAR(bit)] & BIT_IN_CHAR(bit)) != 0);
    }

    bool BitArray::operator[](unsigned int bit) const
    {
        return((_bits[BIT_CHAR(bit)] & BIT_IN_CHAR(bit)) != 0);
    }

    bool BitArray::operator==(const BitArray& other) const
    {
        if(_size != other._size)
        {
            return false;
        }

        int size = BITS_TO_CHARS(_size);
        for (int i = 0; i < size; ++i)
        {
            if (_bits[i] != other._bits[i])
                return false;
        }
        return true;
    }

    bool BitArray::operator!=(const BitArray& other) const
    {
        return !(*this == other);
    }

    BitArray BitArray::operator~() const
    {
        BitArray result(_size);
        result = *this;
        result.not();

        return result;
    }

    BitArray BitArray::operator&(const BitArray& other) const
    {
        BitArray result(_size);
        result = *this;
        result &= other;

        return result;
    }

    BitArray BitArray::operator^(const BitArray& other) const
    {
        BitArray result(_size);
        result = *this;
        result ^= other;

        return result;
    }

    BitArray BitArray::operator|(const BitArray& other) const
    {
        BitArray result(_size);
        result = *this;
        result |= other;

        return result;
    }

    BitArray BitArray::operator<<(unsigned int shifts) const
    {
        BitArray result(_size);
        result = *this;
        result <<= shifts;

        return result;
    }

    BitArray BitArray::operator>>(unsigned int shifts) const
    {
        BitArray result(_size);
        result = *this;
        result >>= shifts;

        return result;
    }

    BitArray& BitArray::operator++()
    {
        int i;
        unsigned char maxValue;     /* maximum value for current char */
        unsigned char one;          /* least significant bit in current char */

        if(_size == 0)
            return *this;

        /* handle arrays that don't use every bit in the last character */
        i = (_size % CHAR_BIT);
        if(i != 0)
        {
            maxValue = UCHAR_MAX << (CHAR_BIT - i);
            one = 1 << (CHAR_BIT - i);
        }
        else
        {
            maxValue = UCHAR_MAX;
            one = 1;
        }

        for(i = BIT_CHAR(_size - 1); i >= 0; i--)
        {
            if(_bits[i] != maxValue)
            {
                _bits[i] = _bits[i] + one;
                return *this;
            }
            else
            {
                /* need to carry to next byte */
                _bits[i] = 0;

                /* remaining characters must use all bits */
                maxValue = UCHAR_MAX;
                one = 1;
            }
        }

        return *this;
    }

    BitArray& BitArray::operator++(int)
    {
        ++(*this);
        return *this;
    }

    BitArray& BitArray::operator--()
    {
        int i;
        unsigned char maxValue;     /* maximum value for current char */
        unsigned char one;          /* least significant bit in current char */

        if(_size == 0)
            return *this;

        /* handle arrays that don't use every bit in the last character */
        i = (_size % CHAR_BIT);
        if(i != 0)
        {
            maxValue = UCHAR_MAX << (CHAR_BIT - i);
            one = 1 << (CHAR_BIT - i);
        }
        else
        {
            maxValue = UCHAR_MAX;
            one = 1;
        }

        for(i = BIT_CHAR(_size - 1); i >= 0; i--)
        {
            if(_bits[i] >= one)
            {
                _bits[i] = _bits[i] - one;
                return *this;
            }
            else
            {
                /* need to borrow from the next byte */
                _bits[i] = maxValue;

                /* remaining characters must use all bits */
                maxValue = UCHAR_MAX;
                one = 1;
            }
        }

        return *this;
    }

    BitArray& BitArray::operator--(int)
    {
        --(*this);
        return *this;
    }

    BitArray& BitArray::operator=(BitArray&& src)
    {
        if(this == &src)
            return *this;

        clear();

        _bits = std::move(src._bits);
        _size = src._size;
        src._size = 0;
        src._bits = nullptr;

        return *this;
    }

    BitArray& BitArray::operator=(const BitArray& copy)
    {
        if(this == &copy)
            return *this;

        clear();

        /* copy bits from source */
        int size = BITS_TO_CHARS(copy._size);
        _bits = new unsigned char[size];
        std::memcpy(_bits, copy._bits, size);
        _size = copy._size;
        return *this;
    }

    BitArray& BitArray::operator&=(const BitArray& src)
    {
        int size = BITS_TO_CHARS(_size);

        if(_size != src._size)
            throw std::exception("Size Mismatch");

        /* AND array one unsigned char at a time */
        for(int i = 0; i < size; i++)
            _bits[i] = _bits[i] & src._bits[i];

        return *this;
    }

    BitArray& BitArray::operator^=(const BitArray& src)
    {
        int size = BITS_TO_CHARS(_size);

        if(_size != src._size)
            throw std::exception("Size Mismatch");

        /* XOR array one unsigned char at a time */
        for(int i = 0; i < size; i++)
            _bits[i] = _bits[i] ^ src._bits[i];

        return *this;
    }

    BitArray& BitArray::operator|=(const BitArray& src)
    {
        int size = BITS_TO_CHARS(_size);

        if(_size != src._size)
            throw std::exception("Size Mismatch");

        /* OR array one unsigned char at a time */
        for(int i = 0; i < size; i++)
            _bits[i] = _bits[i] | src._bits[i];

        return *this;
    }

    BitArray& BitArray::not()
    {
        int size = BITS_TO_CHARS(_size);
        if(_size == 0)
            return *this;

        /* NOT array one unsigned char at a time */
        for(int i = 0; i < size; i++)
            _bits[i] = ~_bits[i];

        /* zero any spare bits so increment and decrement are consistent */
        int bits = _size % CHAR_BIT;
        if(bits != 0)
        {
            unsigned char mask = UCHAR_MAX << (CHAR_BIT - bits);
            _bits[BIT_CHAR(_size - 1)] &= mask;
        }

        return *this;
    }

    BitArray& BitArray::operator<<=(unsigned int shifts)
    {
        int i;
        int chars = shifts / CHAR_BIT; /* number of whole byte shifts */

        if(shifts >= _size)
        {
            /* all bits have been shifted off */
            clearAll();
            return *this;
        }

        /* first handle big jumps of bytes */
        if(chars > 0)
        {
            int size = BITS_TO_CHARS(_size);

            for(i = 0; (i + chars) < size; i++)
                _bits[i] = _bits[i + chars];

            /* now zero out new bytes on the right */
            for(i = size; chars > 0; chars--)
                _bits[i - chars] = 0;
        }

        /* now we have at most CHAR_BIT - 1 bit shifts across the whole array */
        for(i = 0; i < (int)(shifts % CHAR_BIT); i++)
        {
            for(unsigned int j = 0; j < BIT_CHAR(_size - 1); j++)
            {
                _bits[j] <<= 1;

                /* handle shifts across byte bounds */
                if(_bits[j + 1] & MS_BIT)
                    _bits[j] |= 0x01;
            }

            _bits[BIT_CHAR(_size - 1)] <<= 1;
        }

        return *this;
    }

    BitArray& BitArray::operator>>=(unsigned int shifts)
    {
        int i;
        int chars = shifts / CHAR_BIT;  /* number of whole byte shifts */

        if(shifts >= _size)
        {
            /* all bits have been shifted off */
            clearAll();
            return *this;
        }

        /* first handle big jumps of bytes */
        if(chars > 0)
        {
            for(i = BIT_CHAR(_size - 1); (i - chars) >= 0; i--)
                _bits[i] = _bits[i - chars];

            /* now zero out new bytes on the right */
            for(; chars > 0; chars--)
                _bits[chars - 1] = 0;
        }

        /* now we have at most CHAR_BIT - 1 bit shifts across the whole array */
        for(i = 0; i < (int)(shifts % CHAR_BIT); i++)
        {
            for(unsigned int j = BIT_CHAR(_size - 1); j > 0; j--)
            {
                _bits[j] >>= 1;

                /* handle shifts across byte bounds */
                if(_bits[j - 1] & 0x01)
                    _bits[j] |= MS_BIT;
            }

            _bits[0] >>= 1;
        }

        i = _size % CHAR_BIT;
        if(i != 0)
        {
            unsigned char mask = UCHAR_MAX << (CHAR_BIT - i);
            _bits[BIT_CHAR(_size - 1)] &= mask;
        }

        return *this;
    }

    BitArrayIndex::BitArrayIndex(BitArray* bits, unsigned int index) : _index(index),
        _bitArray(bits)
    {
    }

    void BitArrayIndex::operator=(bool src)
    {
        if(_bitArray == NULL)
            return;     /* no array */

#ifdef __BIT_ARRAY_ENABLE_CHECKS__
        if(_bitArray->size() <= _index)
        {
            return;     /* index is out of bounds */
        }
#endif

        if(src)
            _bitArray->setBit(_index);
        else
            _bitArray->clearBit(_index);
    }
}}

#endif
