/**
 * @file BinaryIO
 * @author Alex Towell (atowell@siue.edu)
 *
 * A binary IO module for reading and writing to binary streams. (serialization interface)
 */

#ifndef __BINARY_IO_H__
#define __BINARY_IO_H__

#include <vector>
#include <cstdint>
#include <iostream>
#include "BitArray.h"
using std::string;
using alex::collections::BitArray;
using std::istream;
using std::ostream;

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

    uint8_t     readU8(istream& file);
    void        writeU8(ostream& file, uint8_t value);

    uint16_t    readU16(istream& file);
    void        writeU16(ostream& file, uint16_t value);

    uint32_t    readU32(istream& file);
    void        writeU32(ostream& file, uint32_t value);

    string      readString(istream& file);
    void        writeString(ostream& file, const string& str);

    void        readBitArray(istream& file, BitArray& bitArray);
    void        writeBitArray(ostream& file, BitArray& bitArray);

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
        for (size_t i = 0; i < size; ++i)
        {
            writeU8(file, data[i]);
        }
    }

    uint32_t readArrayU8(istream& file, uint8_t*& data)
    {
        const uint32_t size = readVarU32(file);
        data = new uint8_t[size];
        for (size_t i = 0; i < size; ++i)
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

        string str(buf, len);
        delete[] buf;

        return str;
    }

    void writeString(ostream& file, const string& str)
    {
        const uint32_t len = str.length();

        writeVarU32(file, len);
        file.write(str.c_str(), len);
    }

    void readBitArray(istream& file, BitArray& bitArray)
    {
        const uint32_t len = readVarU32(file);
    
        if (len > 0)
        {
            char* buf = new char[len];
            file.read(buf, len);

            bitArray.move(buf, len);
        }
    }

    void writeBitArray(ostream& file, BitArray& bitArray)
    {
        const uint32_t len = bitArray.bytes();

        writeVarU32(file, len);
        if (len > 0)
            file.write((char*)bitArray.getBytes(), len);
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
}}}

#endif