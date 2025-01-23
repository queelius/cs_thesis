/**
 * @file PerfectHashFn.h
 * @author Alex Towell
 * @date March 2014
 *
 * C++ interface for cmph, a C library for constructing perfect hash functions.
 */

#ifndef __PERFECT_HASH_FN_H__
#define __PERFECT_HASH_FN_H__

#include "cmph\cmph.h"
#include "BinaryIO.h"
#include <cstdint>
#include <string>

namespace alex { namespace hash
{
    class PerfectHashFn
    {
    public:
        const static uint8_t VERSION = 1;
        const static char HEADER[];

        PerfectHashFn() : _buffer(nullptr), _bufferSz(0) {};

        PerfectHashFn(const PerfectHashFn& copy) :
            _bufferSz(copy._bufferSz),
            _buffer(new unsigned char[copy._bufferSz])
        {
            std::memcpy(_buffer, copy._buffer, copy._bufferSz);
        };

        PerfectHashFn(PerfectHashFn&& src) :
            _bufferSz(src._bufferSz),
            _buffer(src._buffer)
        {
            src._bufferSz = 0;
            src._buffer = nullptr;
        };

        PerfectHashFn(char** vec, size_t size, bool verbose = true, double loadFactor = 0.9, cmph_cpp::CMPH_ALGO algo = cmph_cpp::CMPH_CHD_PH) :
            _bufferSz(0),
            _buffer(nullptr)
        {
            build(vec, size, verbose, loadFactor, algo);
        };

        PerfectHashFn& operator=(const PerfectHashFn& other)
        {
            if (this != &other)
            {
                clear();

                _bufferSz = other._bufferSz;
                _buffer = new unsigned char[_bufferSz];
                std::memcpy(_buffer, other._buffer, other._bufferSz);
            }

            return *this;
        };

        PerfectHashFn& operator=(PerfectHashFn&& other)
        {
            if (this != &other)
            {
                clear();

                _bufferSz = other._bufferSz;
                _buffer = other._buffer;
                other._buffer = nullptr;
                other._bufferSz = 0;
            }

            return *this;
        };

        template <class Iter>
        uint32_t build(Iter begin, Iter end, bool verbose = true, double loadFactor = 0.9, cmph_cpp::CMPH_ALGO aglo = cmph_cpp::CMPH_CHD_PH)
        {
            size_t numTerms = std::distance(begin, end);
            char** terms = new char*[numTerms];
            unsigned int i = 0;
            for (auto x = begin; x != end; ++x)
            {
                terms[i] = new char[x->second.hash.size()+1];
                terms[i][x->second.hash.size()] = 0;
                std::memcpy(terms[i], x->second.hash.c_str(), x->second.hash.size());
                ++i;
            }

            uint32_t sz = build(terms, numTerms, verbose, loadFactor, algo);

            for (i = 0; i < numTerms; ++i)
                delete [] terms[i];
            delete [] terms;

            return sz;
        }

        uint32_t build(char** vec, size_t size, bool verbose = true, double loadFactor = 0.9, cmph_cpp::CMPH_ALGO algo = cmph_cpp::CMPH_CHD_PH)
        {
            const auto source = cmph_cpp::cmph_io_vector_adapter(vec, size);
            auto config = cmph_cpp::cmph_config_new(source);
            cmph_cpp::cmph_config_set_verbosity(config, verbose);
            cmph_cpp::cmph_config_set_graphsize(config, loadFactor);
            cmph_cpp::cmph_config_set_algo(config, algo);
            const auto hash = cmph_cpp::cmph_new(config);

            clear();
            _bufferSz = cmph_cpp::cmph_packed_size(hash);
            _buffer = new unsigned char[_bufferSz];

            cmph_cpp::cmph_pack(hash, _buffer);
            cmph_cpp::cmph_config_destroy(config);
            const uint32_t hashSize = cmph_cpp::cmph_size(hash);

            cmph_cpp::cmph_io_vector_adapter_destroy(source);
            cmph_cpp::cmph_destroy(hash);

            return hashSize;
        };

        void clear()
        {
            if (_buffer != nullptr)
            {
                delete [] _buffer;
                _buffer = nullptr;
                _bufferSz = 0;
            }        
        };

        uint32_t bufferSize() const
        {
            return _bufferSz;
        };

        void* buffer()
        {
            return (void*)_buffer;
        };

        unsigned int search(const std::string& key) const
        {
            return cmph_cpp::cmph_search_packed(_buffer, key.c_str(), key.size());
        };

        unsigned int search(const char* key, unsigned int len) const
        {
            return cmph_cpp::cmph_search_packed(_buffer, key, len);
        };

        void writeBinary(std::ostream& file) const
        {
            if (_buffer == nullptr)
            {
                throw std::exception("Empty Perfect Hash Function");
            }

            alex::io::binary::writeString(file, HEADER);
            alex::io::binary::writeU8(file, VERSION);
            alex::io::binary::writeVarU32(file, _bufferSz);
            file.write((char*)_buffer, _bufferSz);
        };

        void readBinary(std::istream& file)
        {
            const auto& hdr = alex::io::binary::readString(file);
            if (hdr != HEADER)
                throw std::exception(("Unexpected Header: " + hdr).c_str());
            uint8_t version = alex::io::binary::readU8(file);
            if (version != VERSION)
                throw std::exception(("Unexpected Version: " + std::to_string(version)).c_str());

            clear();   
            _bufferSz = alex::io::binary::readVarU32(file);
            _buffer = new unsigned char[_bufferSz];
            file.read((char*)_buffer, _bufferSz);
        };
        
        virtual ~PerfectHashFn()
        {
            clear();
        };

    private:
        unsigned char* _buffer;
        uint32_t _bufferSz;
    };

    const char PerfectHashFn::HEADER[] = "ph";
}}

#endif