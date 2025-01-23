#ifndef __PSI_MIN_H__
#define __PSI_MIN_H__

#include <iostream>
#include <string>
#include <ostream>
#include "BitArray.h"
#include "ICmph.h"
#include "Hash.h"
#include "Utils.h"
#include "BinaryIO.h"
#include <iomanip>
#include <cstdint>
#include "PsiFreq.h"
#include "HiddenQuery.h"
#include "ISecureIndex.h"
#include "PsiFreq.h"
#include <vector>

namespace alex { namespace index
{
    class PsiMin : public ISecureIndex
    {
        friend class PsiMinBuilder;

        public:
            const static uint8_t VERSION = 1;
            const static char HEADER[];

            PsiMin() : _hashWidth(0), _maxHash(0) {};

            PsiMin(const PsiMin& copy) :
                _distThreshold(copy._distThreshold),
                _hashBits(copy._hashBits),
                _hashWidth(copy._hashWidth),
                _distWidth(copy._distWidth),
                _distBits(copy._distBits),
                _maxHash(copy._maxHash),
                _phMin(copy._phMin),
                _ref(copy._ref) {};

            PsiMin(PsiMin&& src) :
                _distThreshold(src._distThreshold),
                _hashBits(std::move(src._hashBits)),
                _hashWidth(src._hashWidth),
                _distWidth(src._distWidth),
                _distBits(std::move(src._distBits)),
                _maxHash(src._maxHash),
                _phMin(std::move(src._phMin)),
                _ref(std::move(src._ref)) {};

            PsiMin& operator=(PsiMin&& other)
            {
                if (this != &other)
                {
                    clear();

                    _distBits = std::move(other._distBits);
                    _hashBits = std::move(other._hashBits);                   
                    _phMin = std::move(other._phMin);
                    _ref = std::move(other._ref);
                    _distThreshold = other._distThreshold;
                    _distWidth = other._distWidth;
                    _hashWidth = other._hashWidth;
                    _maxHash = other._maxHash;
                    other._hashWidth = 0;
                    other._distWidth = 0;
                    other._maxHash = 0;
                }

                return *this;
            };

            PsiMin& operator=(const PsiMin& other)
            {
                if (this != &other)
                {
                    clear();

                    _distBits = other._distBits;
                    _hashBits = other._hashBits;
                    _phMin = other._phMin;
                    _ref == other._ref;
                    _distThreshold = other._distThreshold;
                    _distWidth = other._distWidth;
                    _hashWidth = other._hashWidth;
                    _maxHash = other._maxHash;
                }

                return *this;
            };

            double getMinPairwiseScore(const HiddenQuery& q) const
            {
                const std::string& suffix = "|" + _ref;

                double sum = 0;
                int k = 0;

                for (size_t i = 1; i < q.terms.size(); ++i)
                {
                    const std::string& x1 = alex::crypt::hexdigest<alex::globals::digest_size>(q.terms[i][0] + suffix);

                    for (size_t j = 0; j < i; ++j)
                    {
                        const std::string& x2 = alex::crypt::hexdigest<alex::globals::digest_size>(q.terms[j][0] + suffix);
                        std::string p = (x1 < x2 ? x1 + "|" + x2 : x2 + "|" + x1);
                        const std::string& qp = alex::crypt::hexdigest<alex::globals::digest_size>(std::move(p));
                        const uint32_t idx = _phMin.search(qp);
                        const uint32_t value = alex::hash::jenkinsHash(qp) % _maxHash;

                        if (_hashBits.get(_hashWidth * idx, _hashWidth) == value)
                            sum += _distBits.get(_distWidth * idx, _distWidth);
                        else
                        {
                            sum += _distThreshold + 1;
                        }

                        /*
                        // needs that keyword rep for hidden queries instead of bigram biwords
                        // can convert to biword as needed by a secure index

                        uint32_t max_d = 0;

                        for (int r = 0; r < q.terms[i].size() && max_d != std::numeric_limits<uint32_t>::max(); ++r)
                        {
                            const std::string& x1 = alex::crypt::hexdigest<alex::globals::digest_size>(q.terms[i][r] + suffix);

                            for (int s = 0; s < q.terms[j].size(); ++s)
                            {
                                const std::string& x2 = alex::crypt::hexdigest<alex::globals::digest_size>(q.terms[j][s] + suffix);
                                std::string p = (x1 < x2 ? x1 + "|" + x2 : x2 + "|" + x1);
                                const std::string& q = alex::crypt::hexdigest<alex::globals::digest_size>(std::move(p));
                                const uint32_t idx = _phMin.search(q);
                                const uint32_t value = alex::hash::jenkinsHash(q) % _maxHash;

                                if (_hashBits.get(_hashWidth * idx, _hashWidth) == value)
                                    max_d = std::max(max_d, _distBits.get(_distWidth * idx, _distWidth));
                                else
                                {
                                    max_d = std::numeric_limits<uint32_t>::max();
                                    break;
                                }
                            }
                        }

                        if (max_d == std::numeric_limits<uint32_t>::max())
                        {
                            sum += 100;
                        }
                        else
                        {
                            sum += max_d;
                        }
                        ++k;
                        */
                    }
                }

                //return sum / k;
                return sum;// / q.terms.size();
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

                _ref = readString(file);

                // de-serialize perfect hash function
                _phMin.read(file);

                _distThreshold = readVarU32(file);

                // read hash entries
                _hashWidth = readVarU32(file); // get number of bits per hash entry (false positive rate = 1/2^width)

                _maxHash = (1 << _hashWidth); // derive maximum hash value from fp bits

                readBitArray(file, _hashBits); // read bit array for compactly (packed) storing hash entries

                // read frequency entries
                _distWidth = readVarU32(file);
                readBitArray(file, _distBits);
            };
            void write(std::ostream& file)
            {
                using namespace alex::io::binary;

                writeString(file, HEADER);
                writeU8(file, VERSION);

                writeString(file, _ref);

                // serialize perfect hash function
                _phMin.write(file);

                writeVarU32(file, _distThreshold);

                // write hash entries
                writeVarU32(file, _hashWidth);

                // write bit array for compactly (packed) hash entries
                writeBitArray(file, _hashBits);

                // write block data
                writeVarU32(file, _distWidth);
                writeBitArray(file, _distBits);
            };
            virtual ~PsiMin() { clear(); };

            void dump(int depth = 0) const
            {
                std::cout << std::setw(depth) << "" << "PsiMin" << std::endl;
                std::cout << std::setw(depth) << "" << "ref:" << _ref << std::endl;
                std::cout << std::setw(depth) << "" << "distThresh:" << std::endl;
                std::cout << std::setw(depth) << "" << "distWidth: " << _distWidth << std::endl;
                std::cout << std::setw(depth) << "" << "hashWidth: " << _hashWidth << std::endl;
                std::cout << std::setw(depth) << "" << "maxHash: " << _maxHash << std::endl;
                std::cout << std::setw(depth) << "" << "hashBits:" << std::endl;
                _hashBits.dump(depth+1);
                std::cout << std::setw(depth) << "" << "distBits:" << std::endl;
                _distBits.dump(depth+1);
                std::cout << std::setw(depth) << "" << "phMin:" << std::endl;
                _phMin.dump(depth+1);
            };

            void clear()
            {
                if (_hashBits.size() != 0)
                {
                    _distWidth = 0;
                    _hashWidth = 0;
                    _maxHash = 0;
                    _hashBits.clear();
                    _distBits.clear();
                    _phMin.clear();
                }
            };
            unsigned int getApproximateWords() const { throw std::exception("Operation Not Supported"); };
            bool hasFrequency() const { return false; };
            bool hasProximity() const { return true; };
            TSecureIndex getType() const { return TSecureIndex::PsiMin; };
            void read(const boost::filesystem::path& infile)
            {
                read(std::ifstream(infile.string(), std::ios::binary));
            };
            void write(const boost::filesystem::path& infile)
            {
                write(std::ofstream(infile.string(), std::ios::binary));
            };           
            unsigned int getFrequency(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                throw std::exception("Operation Not Supported");
            };
            bool contains(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                // incorrect for phrase terms
                // unless phrase terms are represented differently -- possible
                // e.g.: crypt(word1|s), crypt(word2|s), ...
                // biwords: crypt(word1|s) | " " | crypt(word2|s) | s
                // so just sent sequence of keywords
                // and construct bigrams as needed for biwords, if secure index needs

                if (t.size() < 2)
                    throw std::exception("Operation Not Supported: size(term) < 2");

                const std::string& suffix = "|" + _ref;
                for (int i = 0; i < t.size() - 1; ++i)
                {
                    for (int j = i + 1; j < std::min(i + _distThreshold + 1, t.size()); ++j)
                    {
                        // cryptographic hashes unnecessary here--hiddeen terms have already been cryptohashed so just use fast hashes
                        const std::string& x1 = alex::crypt::hexdigest<alex::globals::digest_size>(t[i] + suffix);
                        const std::string& x2 = alex::crypt::hexdigest<alex::globals::digest_size>(t[j] + suffix);
                        const std::string& q = alex::crypt::hexdigest<alex::globals::digest_size>(
                            (x1 < x2 ? x1 + "|" + x2 : x2 + "|" + x1));

                        const uint32_t idx = _phMin.search(q);
                        const uint32_t value = alex::hash::jenkinsHash(q) % _maxHash;

                        if (_hashBits.get(_hashWidth * idx, _hashWidth) == value)
                        {
                            if (_distBits.get(_distWidth * idx, _distWidth) > (j-i))
                                return false;
                        }
                        else
                            return false;
                    }
                }

                return true;
            };
            bool containsAny(const alex::index::HiddenQuery& q) const
            {
                throw std::exception("Operation Not Supported");
            };
            bool containsAll(const alex::index::HiddenQuery& q) const
            {
                throw std::exception("Operation Not Supported");
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
                for (unsigned int i = 0; i < _hashBits.size() / _hashWidth; ++i)
                {
                    ss << std::right << std::setw(8) << i << " -> hash={";
                    for (unsigned int j = 0; j < _hashWidth; ++j)
                       ss << _hashBits.getBit(i * _hashWidth + j);
                    ss << "} :: dist=" << _distBits.get(i * _distWidth, _distWidth) << std::endl;
                }
                ss << std::endl;
                return ss.str();
            };
            std::string getPreferredFileExtension() const { return HEADER; };

        private:
            static const unsigned int MAX_BYTE = (1 << CHAR_BIT);
        
            alex::hash::ICmph _phMin;

            uint32_t _maxHash;
            alex::collections::BitArray _hashBits;
            uint32_t _hashWidth;

            alex::collections::BitArray _distBits;
            uint32_t _distWidth;
            uint32_t _distThreshold;

            std::string _ref;
    };

    const char PsiMin::HEADER[] = "psim";
}}

#endif