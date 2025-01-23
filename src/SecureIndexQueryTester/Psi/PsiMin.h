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
    // Perfect secure index
    class PsiMin : public ISecureIndex
    {
        friend class PsiMinBuilder;

        public:
            const static uint8_t VERSION = 1;
            const static char HEADER[];

            PsiMin() : _hashWidth(0), _maxHash(0), _freq(nullptr) {};
            PsiMin(const PsiMin& copy) :
                _hashBits(copy._hashBits),
                _hashWidth(copy._hashWidth),
                _distWidth(copy._distWidth),
                _distBits(copy._distBits),
                _maxHash(copy._maxHash),
                _phMin(copy._phMin),
                _freq(copy._freq) {};

            PsiMin(PsiMin&& src) :
                _hashBits(std::move(src._hashBits)),
                _hashWidth(src._hashWidth),
                _distWidth(src._distWidth),
                _distBits(std::move(src._distBits)),
                _maxHash(src._maxHash),
                _phMin(std::move(src._phMin)),
                _freq(std::move(src._freq)) {};

            PsiMin& operator=(PsiMin&& other)
            {
                if (this != &other)
                {
                    clear();

                    _distBits = std::move(other._distBits);
                    _hashBits = std::move(other._hashBits);                   
                    _phMin = std::move(other._phMin);
                    _freq = std::move(other._freq);

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
                    _freq = other._freq;
                    _distWidth = other._distWidth;
                    _hashWidth = other._hashWidth;
                    _maxHash = other._maxHash;
                }

                return *this;
            };

            std::map<std::pair<HiddenQuery::HiddenTerm, HiddenQuery::HiddenTerm>, int> getMinPairWiseDistances(const HiddenQuery& q) const
            {
                std::vector<HiddenQuery::HiddenTerm> present;
                for (const auto& t : q.terms)
                {
                    if (t.size() > 1)
                    {
                        // size == 1, then may be a unigram or bigram (bigram is a two word exact phrase)
                        // min pair distance measures on this works fine -- in theory, although here i
                        // won't be including the distance between two different bigrams, so if a term
                        // of size 1 is a bigram, it won't be found in the min index.
                        // but if size > 1, then it is a trigram or larger (three or more word exact phrase) -- proximity queries
                        // on these are not supported. thus, the decision here is to ignore it.
                        //
                        // we don't actually ignore it when computing the getMinPairwiseDistScore, but
                        // this is a diagnostic function where i want to see "real" values more than
                        // estimated values.
                        continue;
                    }

                    if (_freq->contains(t)) // in min pairs, only consider existent terms in the doc
                        present.push_back(t);
                }

                const std::string& suffix = "|" + _freq->getReference();
                std::map<std::pair<HiddenQuery::HiddenTerm, HiddenQuery::HiddenTerm>, int> pairs;
                for (size_t i = 1; i < present.size(); ++i)
                {
                    for (size_t j = 0; j < i; ++j)
                    {
                        const auto& x1 = alex::crypt::hexdigest<alex::globals::digest_size>(present[i][0] + suffix);
                        const auto& x2 = alex::crypt::hexdigest<alex::globals::digest_size>(present[j][0] + suffix);
                        std::string p = (x1 < x2 ? 
                                         x1 + "|" + x2 :
                                         x2 + "|" + x1);

                        const std::string& q = alex::crypt::hexdigest<alex::globals::digest_size>(std::move(p));
                        const uint32_t idx = _phMin.search(q);
                        const uint32_t value = alex::hash::jenkinsHash(q) % _maxHash;

                        if (_hashBits.get(_hashWidth * idx, _hashWidth) == value)
                            pairs[std::make_pair(present[i], present[j])] = _distBits.get(_distWidth * idx, _distWidth); // pair exists
                        else
                        {
                            // expected distance between two randomly chosen points in the interval 0, approximate_words-1.
                            pairs[std::make_pair(present[i], present[j])] = getApproximateWords() / 3;
                        }
                    }
                }
                return pairs;
            }

            double getMinPairwiseScore(const HiddenQuery& q) const
            {
                const std::string& suffix = "|" + _freq->getReference();

                std::vector<std::string> evaluateInPairs;
                for (const auto& t : q.terms)
                {
                    if (_freq->contains(t)) // in proximity, only consider existent terms in the doc
                    {
                        if (t.size() == 1)
                            evaluateInPairs.push_back(t[0]);
                    }
                }

                double sum = 0;
                for (size_t i = 1; i < evaluateInPairs.size(); ++i)
                {
                    for (size_t j = 0; j < i; ++j)
                    {
                        const auto& x1 = alex::crypt::hexdigest<alex::globals::digest_size>(evaluateInPairs[i] + suffix);
                        const auto& x2 = alex::crypt::hexdigest<alex::globals::digest_size>(evaluateInPairs[j] + suffix);
                        std::string p = (x1 < x2 ? 
                                         x1 + "|" + x2 :
                                         x2 + "|" + x1);

                        const std::string& q = alex::crypt::hexdigest<alex::globals::digest_size>(std::move(p));
                        const uint32_t idx = _phMin.search(q);
                        const uint32_t value = alex::hash::jenkinsHash(q) % _maxHash;

                        if (_hashBits.get(_hashWidth * idx, _hashWidth) == value)
                            sum += _distBits.get(_distWidth * idx, _distWidth); // pair exists
                    }
                }

                return sum;
            };

            double getMinPairwiseScore_old(const HiddenQuery& q) const
            {
                // NOTE: i need to redo this. all of those comments made me realize
                // my math is actually pessimistic. the average distance between N
                // pairs of points is L/(N^2-1), but i have it at L/3, insensitive
                // to N. For N = 2, average distance between two points, we get
                // L/(2^2-1)=L/3 as expected, but if N=3, L/(3^2-1) = L/8,
                // and so on.
                //
                // if have time, will redo. but it's not that important.

                const std::string& suffix = "|" + _freq->getReference();

                std::vector<std::string> evaluateInPairs;
                int k = 0;
                for (const auto& t : q.terms)
                {
                    if (_freq->contains(t)) // in proximity, only consider existent terms in the doc
                    {
                        if (t.size() > 1)
                        {
                            // if size == 1, then it may be a unigram or bigram (bigram is a two word exact phrase)
                            // min pair distance measures on this works fine -- in theory, although here i
                            // won't be including the distance between two different bigrams, so if a term
                            // of size 1 is a bigram, it won't be found in the index.
                            // but if size > 1, then it is a trigram or larger (three or more word exact phrase) -- proximity queries
                            // on these are not supported by PsiMin. Thus, the decision here is to
                            // assume that it will be randomly dsitributed somewhere in the document
                            // thus the expected distance between such a term and any other term
                            // is document_length / 3. we will assume this.
                            //
                            // if all terms are phrases, then all pairs will be L/3.
                            // that is, L*(t-1)/6, after normalizing by t matched terms.
                            // let's keep track of matching terms that can't be used
                            // in min pair wise calcs, call it k.
                            //
                            // in the end, we'll return sum of pairwise distances
                            // plus Lk*(e+k-1)/6, where e+k is total matched terms.
                            // if e = total, then k = 0 and this becomes zero.
                            // if e = 0, then Lk(k-1)/6, which is what we had
                            // above (all pairs will be L/3). it works for everything
                            // in between also.
                            ++k;
                        }
                        else
                            evaluateInPairs.push_back(t[0]);
                    }
                }

                const double L = getApproximateWords();
                // if only one term is matched in a multi-term query,
                // then return the length of the document.
                if (evaluateInPairs.size() + k <= 1)
                    return L;

                double sum = 0;
                int k2 = 0;
                for (size_t i = 1; i < evaluateInPairs.size(); ++i)
                {
                    for (size_t j = 0; j < i; ++j)
                    {
                        const auto& x1 = alex::crypt::hexdigest<alex::globals::digest_size>(evaluateInPairs[i] + suffix);
                        const auto& x2 = alex::crypt::hexdigest<alex::globals::digest_size>(evaluateInPairs[j] + suffix);
                        std::string p = (x1 < x2 ? 
                                         x1 + "|" + x2 :
                                         x2 + "|" + x1);

                        const std::string& q = alex::crypt::hexdigest<alex::globals::digest_size>(std::move(p));
                        const uint32_t idx = _phMin.search(q);
                        const uint32_t value = alex::hash::jenkinsHash(q) % _maxHash;

                        if (_hashBits.get(_hashWidth * idx, _hashWidth) == value)
                            sum += _distBits.get(_distWidth * idx, _distWidth); // pair exists
                        else
                        {
                            // if not found in distBits, then the two terms
                            // are farther apart than the maximum minimum
                            // pairwise distance to record -- in this case,
                            // assume the two words are chosen at random,
                            // as before, so expected distance of L/3.
                            sum += L / 3;
                        }
                    }
                }

                // normalize by number of matched terms
                // more matches means larger sums, so let's
                // not penalize that. there are other
                // ways to normalize this, but this seems
                // good enough.
                //
                // this looks a little complicated, but math
                // has been worked out. other papers propose
                // something like this, but they didn't have to
                // deal with the fact that some matching terms
                // can't be calculated in min pairs, which is
                // why i add the L*k*(e+k-1)/6, then normalize the whole
                // thing by number of matched terms, e+k.
                //
                // if only one term is matched, e+k-1=0, so
                // this formula doesn't work for that. in this case,
                // i return L, as mentioned earlier.
                //
                // if k>1 matches, but both are either phrases
                // or beyond max min dist, then this becomes L/6.
                // that's better than L. i wonder if it's TOO
                // much better though?
                //
                // either way, for most documents, this won't matter
                // much becaomse L will be much larger than 6,
                // and the function we pass the distance into to get
                // a proximity score will flatten out (asymptotically)
                // very quickly. but for small documents, this does
                // match the idea that if two or more terms are
                // present, they'll be pretty close even if we
                // can't do an actual min pairwise calculation and
                // must resort to expected pairwise distances.

                int t = k +- k2;
                int e = evaluateInPairs.size() - k2;
                

                return (sum + L*k*(e + t - 1) / 6) / (e + t);

                //return (sum + L*k*(evaluateInPairs.size() + k - 1) / 6) / (evaluateInPairs.size() + k);
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

                if (_freq == nullptr)
                    _freq = SecureIndex(new PsiFreq);
                else
                    _freq->clear();
                _freq->read(file);

                // get hash of encrypted document
                // _refHash = readString(file);

                // de-serialize perfect hash function
                _phMin.read(file);

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

                _freq->write(file);

                // write hash of encrypted document
                // writeString(file, _refHash);

                // serialize perfect hash function
                _phMin.write(file);

                // write hash entries
                writeVarU32(file, _hashWidth);

                // write bit array for compactly (packed) hash entries
                writeBitArray(file, _hashBits);

                // write block data
                writeVarU32(file, _distWidth);
                writeBitArray(file, _distBits);
            };
            virtual ~PsiMin() { clear(); };

            void dump(int depth = 0)
            {
                std::cout << std::setw(depth) << "" << "PsiMin" << std::endl;
                _freq->dump(depth+1);
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
                    _freq->clear();
                }
            };
            unsigned int getApproximateWords() const { return _freq->getApproximateWords(); };
            bool hasFrequency() const { return true; };
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
                return _freq->getFrequency(t);
            };
            bool contains(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                return _freq->contains(t);
            };
            bool containsAny(const alex::index::HiddenQuery& q) const
            {
                return _freq->containsAny(q);
            };
            bool containsAll(const alex::index::HiddenQuery& q) const
            {
                return _freq->containsAll(q);
            };
            std::string getReference() const
            {
                return _freq->getReference();
            };
            void setReference(const std::string& ref)
            {
                _freq->setReference(ref);
            };
            std::string toString() const
            {
                std::stringstream ss;
                ss << _freq->toString() << std::endl;
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

            SecureIndex _freq;
    };

    const char PsiMin::HEADER[] = "psim";
}}

#endif