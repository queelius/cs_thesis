//#ifndef __PSI_FREQ_H__
//#define __PSI_FREQ_H__
//
//#include <iostream>
//#include <string>
//#include <ostream>
//#include "BitArray.h"
//#include "PHFilter.h"
//#include "Hash.h"
//#include "BinaryIO.h"
//#include "CompactArray.h"
//#include <cstdint>
//#include "HiddenQuery.h"
//#include "ISecureIndex.h"
//#include "SHA.h"
//#include <vector>
//#include <algorithm>
//
//namespace alex { namespace secure_index
//{
//    // Perfect hash-based secure index w/frequency information
//    template <unsigned int N = 10>
//    class PsiFreq: public ISecureIndex
//    {
//        friend class PsiFreqBuilder;
//
//        public:
//            const static uint8_t VERSION = 1;
//            const static char HEADER[];
//
//            PsiFreq() {};
//
//            PsiFreq(const PsiFreq& copy) :
//                _freqs(copy._freqs),
//                _filter(copy._filter),
//                _approximateWords(copy._approximateWords),
//                _ref(copy._ref) {};
//
//            PsiFreq(PsiFreq&& src) :
//                _freqs(std::move(src._freqs)),
//                _filter(std::move(src._filter)),
//                _approximateWords(src._approximateWords),
//                _ref(std::move(src._ref)) {};
//
//            PsiFreq& operator=(PsiFreq&& other)
//            {
//                if (this != &other)
//                {
//                    clear();
//                    _freqs = std::move(other._freqs);
//                    _filter = std::move(other._filter);
//                    _ref = std::move(other._ref);
//                    _approximateWords = other._approximateWords;
//                    other._approximateWords = 0;
//                }
//
//                return *this;
//            };
//
//            PsiFreq& operator=(const PsiFreq& other)
//            {
//                if (this != &other)
//                {
//                    clear();
//                    _freqs = other._freqs;
//                    _filter = other._filter;
//                    _ref = other._ref;
//                    _approximateWords = other._approximateWords;
//                }
//
//                return *this;
//            };
//
//            void readBinary(std::istream& file)
//            {
//                const std::string& hdr = alex::io::binary::readString(file);
//                if (hdr != HEADER)
//                    throw std::exception(("Unexpected Header: " + hdr).c_str());
//
//                unsigned int version = alex::io::binary::readU8(file);
//                if (version != VERSION)
//                    throw std::exception("Unexpected Version");
//
//                _ref = alex::io::binary::readString(file); // get reference (e.g., url) for encrypted document
//                _approximateWords = alex::io::binary::readVarU32(file);
//                _filter.readBinary(file); // de-serialize perfect hash filter
//                _freqs.readBinary(file);
//            };
//
//            void writeBinary(std::ostream& file)
//            {
//                alex::io::binary::writeString(file, HEADER);
//                alex::io::binary::writeU8(file, VERSION);
//
//                alex::io::binary::writeString(file, _ref); // write reference (e.g., url) for encrypted document
//                alex::io::binary::writeVarU32(file, _approximateWords); // write approximate number of words in index
//                _filter.writeBinary(file); // serialize perfect hash function
//                _freqs.writeBinary(file); // serialize frequency info
//            };
//
//            virtual ~PsiFreq() { clear(); };
//
//            void clear()
//            {
//                _filter.clear();
//                _freqs.clear();
//                _ref = "";
//                _approximateWords = 0;
//            };
//
//            unsigned int approximateWords() const { return _approximateWords; }
//
//            bool hasFrequency() const { return true; }
//
//            bool hasLocations() const { return false; }
//
//            /*
//            TSecureIndex getType() const { return TSecureIndex::PsiFrequency; }
//
//            unsigned int getFrequency(const alex::index::HiddenQuery::HiddenTerm& t) const
//            {
//                if (t.empty())
//                    throw std::exception("No Hidden Query Terms");
//
//                const std::string suffix = "|" + _ref;
//                uint32_t freq = std::numeric_limits<uint32_t>::max();
//
//                for (const auto& x : t)
//                {
//                    const auto& result = _filter.contains(alex::crypt::hexdigest<N>(x + suffix));
//                    if (!result.first)
//                        return 0;
//
//                    freq = std::min(freq, _freqs.get(result.second));
//                }
//
//                return freq;
//            };
//
//            bool contains(const alex::index::HiddenQuery::HiddenTerm& t) const
//            {
//                const std::string suffix = "|" + _ref;
//                for (const auto& x : t)
//                {
//                    if (!_filter.contains(alex::crypt::hexdigest<N>(x + suffix)).second)
//                        return false;
//                }
//                return true;
//            };
//
//            bool containsAny(const alex::index::HiddenQuery& q) const
//            {
//                for (const auto& t : q.terms)
//                {
//                    if (contains(t))
//                        return true;
//                }
//                return false;
//            };
//
//            bool containsAll(const alex::index::HiddenQuery& q) const
//            {
//                for (const auto& t : q.terms)
//                {
//                    if (!contains(t))
//                        return false;
//                }
//                return true;
//            };
//
//            std::string getReference() const
//            {
//                return _ref;
//            };
//
//            void setReference(const std::string& ref)
//            {
//                _ref = ref;
//            };
//
//            std::string getPreferredFileExtension() const { return HEADER; };
//            alex::collections::PHFilter getFilter() const { return _filter; };
//            alex::collections::CompactArray getFrequencies() const { return _freqs; };
//            uint32_t getApproximateIndices() const { return _filter.approximateSize(); };
//            */
//
//        private:
//            alex::collections::PHFilter _filter;
//            alex::collections::CompactArray _freqs;   
//            uint32_t _approximateWords;
//            std::string _ref;
//    };
//
//    template <unsigned int N>
//    const char PsiFreq<N>::HEADER[] = "psif";
//}}
//
//#endif