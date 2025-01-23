#ifndef __PSI_FREQ_BSI_BLOCK_H__
#define __PSI_FREQ_BSI_BLOCK_H__

#include "PsiFreq.h"
#include "BsiBlock.h"

namespace alex { namespace index
{
    // Perfect secure index
    class PsiFreqBsiBlock : public ISecureIndex
    {
        friend class PsiFreqBsiBlockBuilder;

        public:
            const static uint8_t VERSION = 1;
            const static char HEADER[];

            PsiFreqBsiBlock() {};

            PsiFreqBsiBlock(const PsiFreqBsiBlock& copy) :
                _freq(copy._freq),
                _block(copy._block) {};

            PsiFreqBsiBlock(PsiFreqBsiBlock&& src) :
                _freq(std::move(src._freq)),
                _block(std::move(src._block)) {}

            PsiFreqBsiBlock& operator=(PsiFreqBsiBlock&& other)
            {
                if (this != &other)
                {
                    clear();
                    _freq = std::move(other._freq);
                    _block = std::move(other._block);
                }

                return *this;
            };

            PsiFreqBsiBlock& operator=(const PsiFreqBsiBlock& other)
            {
                if (this != &other)
                {
                    clear();
                    _freq = other._freq;
                    _block = other._block;
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

                _freq->read(file);
                _block->read(file);
            };
            void write(std::ostream& file)
            {
                using namespace alex::io::binary;

                writeString(file, HEADER);
                writeU8(file, VERSION);
                _freq->write(file);
                _block->write(file);
            };
            virtual ~PsiFreqBsiBlock() { clear(); };

            void dump(int depth = 0)
            {
                _freq->dump(depth+1);
                _block->dump(depth+1);
            };

            void clear()
            {
                _freq->clear();
                _block->clear();
            };
            unsigned int getApproximateWords() const { return _freq->getApproximateWords(); }
            bool hasFrequency() const { return true; }
            bool hasProximity() const { return true; }
            TSecureIndex getType() const { return TSecureIndex::PsiFreqBsiBlock; }
            void read(const boost::filesystem::path& infile)
            {
                _freq->read(infile);
                _block->read(infile);
            };
            void write(const boost::filesystem::path& infile)
            {
                _freq->write(infile);
                _block->write(infile);
            };
            double getMinPairwiseScore(const alex::index::HiddenQuery& q) const
            {
                return _block->getMinPairwiseScore(q);
            };
            std::vector<uint32_t> getLocations(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                //if (!_freq.contains(t))
                //    return std::vector<uint32_t>();

                return _block->getLocations(t);
            };
            std::map<std::pair<HiddenQuery::HiddenTerm, HiddenQuery::HiddenTerm>, int> getMinPairWiseDistances(const HiddenQuery& q) const
            {
                return _block->getMinPairWiseDistances(q);
            };
            unsigned int getFrequency(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                //if (!_block.contains(t))
               //     return 0;

                return _freq->getFrequency(t);
            };
            bool contains(const alex::index::HiddenQuery::HiddenTerm& t) const
            {
                return _freq->contains(t);// && _block.contains(t);
            };
            bool containsAny(const alex::index::HiddenQuery& q) const
            {
                return _freq->containsAny(q); // && _block.containsAny(q);
            };
            bool containsAll(const alex::index::HiddenQuery& q) const
            {
                return _freq->containsAll(q); // && _block.containsAll(q)
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
                ss << _block->toString() << std::endl;
                return ss.str();
            };
            std::string getPreferredFileExtension() const { return HEADER; };

        private:
            alex::index::SecureIndex _freq;
            alex::index::SecureIndex _block;
    };

    const char PsiFreqBsiBlock::HEADER[] = "bsif";
}}

#endif