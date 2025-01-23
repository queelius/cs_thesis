#ifndef __PSI_BOOL_H__
#define __PSI_BOOL_H__

#include <iostream>
#include <string>
#include <ostream>
#include "CompactArray.h"
#include "ICmph.h"
#include "Hash.h"
#include "SHA.h"
#include "Utils.h"
#include "BinaryIO.h"
#include <iomanip>
#include <cstdint>
#include "HiddenQuery.h"
#include "ISecureIndex.h"
#include <vector>

namespace alex { namespace index
{
    class PsiBool : public ISecureIndex
    {
        public:
            const static uint8_t VERSION = 1;
            const static char HEADER[];

            PsiBool() {};
            PsiBool(const PsiBool& copy) :
                _ph(copy._ph),
                _approximateWords(copy._approximateWords),
                _ref(copy._ref)
            {
            };
            PsiBool(PsiBool&& src) :
                _ph(std::move(src._ph)),
                _approximateWords(src._approximateWords),
                _ref(std::move(src._ref)) {};

            PsiBool& operator=(PsiBool&& other)
            {
                if (this != &other)
                {
                    clear();

                    _ph = std::move(other._ph);
                    _ref = std::move(other._ref);
                    _approximateWords = other._approximateWords;
                    other._approximateWords = 0;
                }

                return *this;
            };

            PsiBool& operator=(const PsiBool& other)
            {
                if (this != &other)
                {
                    clear();
                    _ph = other._ph;
                    _ref = other._ref;
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

                // de-serialize perfect hash function
                _ph.read(file);

                // read hash entries

                _approximateWords = readVarU32(file);
            };

            void write(std::ostream& file)
            {
                using namespace alex::io::binary;

                writeString(file, HEADER);
                writeU8(file, VERSION);

                // write reference (e.g., url) for encrypted document
                writeString(file, _ref);

                // serialize perfect hash function
                _ph.write(file);

                // write approximate number of words in index
                writeVarU32(file, _approximateWords);
            };

            virtual ~PsiBool() { clear(); };

            void clear()
            {
                if (_car.size() != 0)
                {
                    _car.clear();
                    _ref = "";
                    _ph.clear();
                    _approximateWords = 0;
                }
            };
            unsigned int getApproximateWords() const { return _approximateWords; }
            bool hasFrequency() const { return false; }
            bool hasProximity() const { return false; }
            TSecureIndex getType() const { return TSecureIndex::PsiBool; }
            void read(const boost::filesystem::path& infile)
            {
                read(std::ifstream(infile.string(), std::ios::binary));
            };
            void write(const boost::filesystem::path& infile)
            {
                write(std::ofstream(infile.string(), std::ios::binary));
            };           
            bool contains(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                return getIndex(t) != -1;
            };

            int getIndex(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                const std::string suffix = "|" + _ref;
                for (const auto& x : t)
                {
                    const std::string& q = alex::crypt::hexdigest<16>(x + suffix);
                    const uint32_t idx = _ph.search(q);
                    //const uint32_t value = alex::hash::jenkinsHash2(q) % _maxHash;
                }

                return -1;
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
                return ss.str();
            };
            std::string getPreferredFileExtension() const { return HEADER; };

        private:
            static const unsigned int MAX_BYTE = (1 << CHAR_BIT);
        
            alex::hash::ICmph _ph;
            uint32_t _maxHash;

            alex::collections::CompactArray _car;


            uint32_t _approximateWords;
            std::string _ref;
    };

    const char PsiBool::HEADER[] = "psibool";
}}

#endif