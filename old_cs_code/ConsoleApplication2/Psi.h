#ifndef __PSI_H__
#define __PSI_H__

#include <iostream>
#include <string>
#include <ostream>
#include "PHFilter.h"
#include "Hash.h"
#include "BinaryIO.h"
#include "CompactArray.h"
//#include <cstdint>
//#include "HiddenQuery.h"
//#include "ISecureIndex.h"
#include "SHA.h"
#include <algorithm>

namespace alex { namespace secure_index
{
    // Perfect hash-based secure index
    template <unsigned int N = 12>
    class Psi //: public ISecureIndex
    {
        friend class PsiBuilder;

        public:
            const static uint8_t VERSION = 1;
            const static char HEADER[];

            Psi() {};

            Psi(const Psi& copy) :
                _filter(copy._filter),
                _approximateWords(copy._approximateWords),
                _ref(copy._ref) {};

            Psi(Psi&& src) :
                _filter(std::move(src._filter)),
                _approximateWords(src._approximateWords),
                _ref(std::move(src._ref)) {};

            Psi& operator=(Psi&& other)
            {
                if (this != &other)
                {
                    clear();
                    _filter = std::move(other._filter);
                    _ref = std::move(other._ref);
                    _approximateWords = other._approximateWords;
                    other._approximateWords = 0;
                }

                return *this;
            };

            Psi& operator=(const Psi& other)
            {
                if (this != &other)
                {
                    clear();
                    _filter = other._filter;
                    _ref = other._ref;
                    _approximateWords = other._approximateWords;
                }

                return *this;
            };

            void readBinary(std::istream& file)
            {
                const std::string& hdr = alex::io::binary::readString(file);
                if (hdr != HEADER)
                    throw std::exception(("Unexpected Header: " + hdr).c_str());

                unsigned int version = alex::io::binary::readU8(file);
                if (version != VERSION)
                    throw std::exception("Unexpected Version");

                _ref = alex::io::binary::readString(file); // get reference (e.g., url) for encrypted document
                _approximateWords = alex::io::binary::readVarU32(file);
                _filter.readBinary(file); // de-serialize perfect hash filter
            };

            void writeBinary(std::ostream& file)
            {
                alex::io::binary::writeString(file, HEADER);
                alex::io::binary::writeU8(file, VERSION);

                alex::io::binary::writeString(file, _ref); // write reference (e.g., url) for encrypted document
                alex::io::binary::writeVarU32(file, _approximateWords); // write approximate number of words in index
                _filter.writeBinary(file); // serialize perfect hash function
            };

            virtual ~Psi() { clear(); };

            void clear()
            {
                _filter.clear();
                _ref = "";
                _approximateWords = 0;
            };

            unsigned int approximateWords() const { return _approximateWords; }

            bool hasFrequency() const { return false; }

            bool hasLocations() const { return false; }

            /*
            TSecureIndex getType() const { return TSecureIndex::PsiBool; }

            bool contains(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                const std::string suffix = "|" + _ref;
                for (const auto& x : t)
                {
                    if (!_filter.contains(alex::crypt::hexdigest<N>(x + suffix)).second)
                        return false;
                }
                return true;
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

            std::string getPreferredFileExtension() const { return HEADER; };
            alex::collections::PHFilter getFilter() const { return _filter; };
            uint32_t getApproximateIndices() const { return _filter.approximateSize(); };
            */

        private:
            alex::collections::PHFilter _filter;
            uint32_t _approximateWords;
            std::string _ref;
    };

    template <unsigned int N>
    const char Psi<N>::HEADER[] = "psi";
}}

#endif