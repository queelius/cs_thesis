/**
 * @file BinaryIO
 * @author Alex Towell (atowell@siue.edu)
 *
 * A binary IO module for reading and writing to binary streams. (serialization interface)
 */

#ifndef __BINARY_IO_H__
#define __BINARY_IO_H__
#include <string.h>
#include <vector>
#include <cstdint>
#include <iostream>
#include <boost/filesystem.hpp>
#include <stdio.h>
#include <inttypes.h>
#include "BitArray.h"
using std::string;
using std::istream;
using std::ostream;
using alex::collections::BitArray;

namespace alex { namespace io { namespace binary
{
    /*************
     * Interface *
     *************/

    // could use a universal coder, like Elias omega coding,
    // if space is really an issue -- however,
    // i do not store the numbers in that representation
    // in memory for speed reasons (e.g., for bit arrays
    // i need constant width per entry for random access).
    // it's also slower to read, and file size is not
    // so much an issue as memory size -- both are of
    // course important though.
    //
    // i use varint in places, and i don't store them
    // in memory as that either, but it's a simple and
    // fast solution. i think i will have an option to
    // disable varint encodings for my secure indexes
    // so that i can quickly learn approximately how much
    // space is being consumed; file size <-> memory space
    // consumption holds when not using either
    // compression coders.
    //
    // also, it will slown down reading and writing a little.

    std::ifstream readBinaryFile(const boost::filesystem::path& file);
    std::ofstream createBinaryFile(const boost::filesystem::path& file, bool overwrite = true);
    std::ofstream appendBinaryFile(const boost::filesystem::path& file);

    std::string readHeader(const boost::filesystem::path& file);

    long double unpack754(uint64_t i, unsigned bits, unsigned expbits);
    uint64_t    pack754(long double f, unsigned bits, unsigned expbits);

    float       read754F(istream& file);
    void        write754F(ostream& file, float value);
    
    void        readBitArray(istream& file, BitArray& bitArray);
    void        writeBitArray(ostream& file, BitArray& bitArray);    

    uint8_t     readU8(istream& file);
    void        writeU8(ostream& file, uint8_t value);

    uint16_t    readU16(istream& file);
    void        writeU16(ostream& file, uint16_t value);

    uint32_t    readU32(istream& file);
    void        writeU32(ostream& file, uint32_t value);

    string      readString(istream& file);
    void        writeString(ostream& file, const string& str);

    void        writeVecU32(ostream& file, const std::vector<uint32_t>& values);
    void        readVecU32(istream& file, std::vector<uint32_t>& values);

    void        writeVecVarU32(ostream& file, const std::vector<uint32_t>& values);
    void        readVecVarU32(istream& file, std::vector<uint32_t>& values);

    void        writeArrayU8(ostream& file, uint32_t size, uint8_t* data);
    uint32_t    readArrayU8(istream& file, uint8_t*& data);

    void        writeVarU32(ostream& file, uint32_t value);
    uint32_t    readVarU32(istream& file);

    /******************
     * Implementation *
     ******************/
    void readBitArray(istream& file, BitArray& bitArray)
    {
        const uint32_t len = readVarU32(file);
    
        char* buf = new char[len];
        file.read(buf, len);

        bitArray.move(buf, len);
    }

    void writeBitArray(ostream& file, BitArray& bitArray)
    {
        const uint32_t len = bitArray.bytes();

        writeVarU32(file, len);
        file.write((char*)bitArray.getBytes(), len);
    }

    float read754F(istream& file)
    {
        return (float)alex::io::binary::unpack754(readU32(file), 32, 8);
    }

    void write754F(ostream& file, float value)
    {
        alex::io::binary::writeU32(file, alex::io::binary::pack754(value, 32, 8));
    }

    void writeVarU32(ostream& file, uint32_t value)
    {
        while (value > 127)
        {
            writeU8(file, (value & 127) | 128);
            value >>= 7;
        }
        writeU8(file, value & 127);
    }

    uint32_t readVarU32(istream& file)
    {
        uint32_t result = 0;
        for (uint8_t i = 0; i < 4; i++)
        {
            uint8_t tmp = readU8(file);
            result |= (tmp & 127) << (7*i);
            if(!(tmp & 128))
                break;
        }
        return result;
    }

    void writeArrayU8(ostream& file, uint32_t size, uint8_t* data)
    {
        writeVarU32(file, size);
        for (uint32_t i = 0; i < size; ++i)
        {
            writeU8(file, data[i]);
        }
    }

    uint32_t readArrayU8(istream& file, uint8_t*& data)
    {
        const uint32_t size = readVarU32(file);
        data = new uint8_t[size];
        for (uint32_t i = 0; i < size; ++i)
        {
            data[i] = readU8(file);
        }
        return size;
    }

    void writeVecU32(ostream& file, const std::vector<uint32_t>& values)
    {
        writeU32(file, (uint32_t)values.size());
        for (const auto& value : values)
        {
            writeU32(file, (uint32_t)value);
        }
    }

    void writeVecVarU32(ostream& file, const std::vector<uint32_t>& values)
    {
        writeVarU32(file, (uint32_t)values.size());
        for (const auto& value : values)
        {
            writeVarU32(file, (uint32_t)value);
        }
    }

    void readVecU32(istream& file, std::vector<uint32_t>& values)
    {
        const uint32_t sz = readU32(file);
        values.reserve(sz);
        for (uint32_t i = 0; i < sz; ++i)
        {
            values.push_back(readU32(file));
        }
    }

    void readVecVarU32(istream& file, std::vector<uint32_t>& values)
    {
        const uint32_t sz = readVarU32(file);
        values.reserve(sz);
        for (uint32_t i = 0; i < sz; ++i)
        {
            values.push_back(readVarU32(file));
        }
    }

    uint8_t readU8(istream& file)
    {
        char value;
        // read 1 byte from the file
        file.read(&value, 1);   
        return (uint8_t)value;
    }

    void writeU8(ostream& file, uint8_t value)
    {
        // write 1 byte to the file
        file.write((char*)(&value), 1);
    }

    uint16_t readU16(istream& file)
    {
        uint16_t value;
        uint8_t bytes[2];

        // read 2 bytes from the file
        file.read((char*)bytes, 2);
    
        // construct the 16-bit value from those bytes
        value = (bytes[0] << 8) | bytes[1];

        return value;
    }

    void writeU16(ostream& file, uint16_t value)
    {
      uint8_t bytes[2];

      // extract the individual bytes from our value
      bytes[0] = (value >> 8) & 0xFF;   // high byte
      bytes[1] = value & 0xFF;          // low byte

      // write those bytes to the file
      file.write((char*)bytes, 2);
    }

    uint32_t readU32(istream& file)
    {
        uint32_t value;
        uint8_t bytes[4];

        file.read((char*)bytes, 4); // read 4 bytes from the file
    
        // construct the 32-bit value from those bytes
        value = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];

        return value;
    }

    void writeU32(ostream& file, uint32_t value)
    {
      uint8_t bytes[4];

      // extract the individual bytes from our value
      bytes[0] = (value >> 24) & 0xFF;
      bytes[1] = (value >> 16) & 0xFF;
      bytes[2] = (value >> 8) & 0xFF;
      bytes[3] = value & 0xFF;

      // write those bytes to the file
      file.write((char*)bytes, 4);
    }

    string readString(istream& file)
    {
        const uint32_t len = readVarU32(file);

        char* buf = new char[len];
        file.read(buf, len);

        string str(std::move(buf), len);

        //delete[] buf;

        return str;
    }

    void writeString(ostream& file, const string& str)
    {
        const uint32_t len = str.length();

        writeVarU32(file, len);
        file.write(str.c_str(), len);
    }

    std::ifstream readBinaryFile(const boost::filesystem::path& file)
    {
        if (!boost::filesystem::exists(file))
            throw std::exception("File Not Found");
        else if (!boost::filesystem::is_regular_file(file))
            throw std::exception("Expected A Regular File");

       std::ifstream fs(file.string(), std::ios::binary);
       if (!fs.is_open())
            throw std::exception("Failed To Open File");

       return fs;
    }

    std::ofstream createBinaryFile(const boost::filesystem::path& file, bool overwrite)
    {
        if (file.empty() || file.filename().string() == ".")
            throw std::exception("No Filename Specified");

        if (boost::filesystem::exists(file))
        {
            if (boost::filesystem::is_directory(file))
                throw std::exception("Name Of Directory");
            if (!overwrite)
                throw std::exception("Filename Already Exists");
        }

       std::ofstream outfile(file.string(), std::ios::binary);
       if (!outfile.is_open())
            throw std::exception("Failed To Create File");

       return outfile;
    }

    std::ofstream appendBinaryFile(const boost::filesystem::path& file)
    {
        if (file.empty() || file.filename().string() == ".")
            throw std::exception("No Filename Specified");

        if (boost::filesystem::exists(file))
        {
            if (boost::filesystem::is_directory(file))
                throw std::exception("Name Of Directory");
        }

       std::ofstream outfile(file.string(), std::ios::binary | std::ios::app);
       if (!outfile.is_open())
            throw std::exception("Failed To Open File");

       return outfile;
    }

    std::string readHeader(const boost::filesystem::path& file)
    {
        return alex::io::binary::readString(readBinaryFile(file));
    }

    uint64_t pack754(long double f, unsigned bits, unsigned expbits)
    {
        long double fnorm;
        int shift;
        long long sign, exp, significand;
        unsigned significandbits = bits - expbits - 1; // -1 for sign bit

        if (f == 0.0) return 0; // get this special case out of the way

        // check sign and begin normalization
        if (f < 0) { sign = 1; fnorm = -f; }
        else { sign = 0; fnorm = f; }

        // get the normalized form of f and track the exponent
        shift = 0;
        while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
        while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
        fnorm = fnorm - 1.0;

        // calculate the binary form (non-float) of the significand data
        significand = fnorm * ((1LL<<significandbits) + 0.5f);

        // get the biased exponent
        exp = shift + ((1<<(expbits-1)) - 1); // shift + bias

        // return the final answer
        return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
    }

    long double unpack754(uint64_t i, unsigned bits, unsigned expbits)
    {
	    long double result;
	    long long shift;
	    unsigned bias;
	    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

	    if (i == 0) return 0.0;

	    // pull the significand
	    result = (i&((1LL<<significandbits)-1)); // mask
	    result /= (1LL<<significandbits); // convert back to float
	    result += 1.0f; // add the one back on

	    // deal with the exponent
	    bias = (1<<(expbits-1)) - 1;
	    shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
	    while(shift > 0) { result *= 2.0; shift--; }
	    while(shift < 0) { result /= 2.0; shift++; }

	    // sign it
	    result *= (i>>(bits-1))&1? -1.0: 1.0;

	    return result;
    }
}}}

#endif