#ifndef __ISECURE_INDEX_H__
#define __ISECURE_INDEX_H__

#include "HiddenQuery.h"
#include "Utils.h"
#include "Globals.h"
#include <vector>
#include <string>
#include <boost/filesystem.hpp>

namespace alex { namespace index
{
    enum class TSecureIndex
    {
        PsiBlock,
        PsiFrequency,
        BsiBlock,
        PsiPost,
        PsiMin
    };

    class ISecureIndex
    {
    public:
        virtual void read(const boost::filesystem::path& infile) = 0;
        virtual void write(const boost::filesystem::path& outfile) = 0;
        virtual void read(std::istream& infile) = 0;
        virtual void write(std::ostream& outfile) = 0;
        virtual TSecureIndex getType() const = 0;
        virtual unsigned int getApproximateWords() const = 0;
        virtual std::string getReference() const = 0;
        virtual void setReference(const std::string& ref) = 0;
        virtual void clear() = 0;
        virtual std::string getPreferredFileExtension() const = 0;
        virtual std::vector<uint32_t> getLocations(const HiddenQuery::HiddenTerm& t) const { throw std::exception("Not Implemented"); };
        virtual unsigned int getFrequency(const HiddenQuery::HiddenTerm& t) const = 0;
        virtual bool containsAny(const HiddenQuery& q) const = 0;
        virtual bool containsAll(const HiddenQuery& q) const = 0;
        virtual bool contains(const HiddenQuery::HiddenTerm& t) const = 0;
        virtual bool hasProximity() const { return false; };
        virtual bool hasFrequency() const { return false; };

        virtual double getMinPairwiseScore(const HiddenQuery& q) const
        {
            std::vector<std::vector<uint32_t>> postingList;
            for (const auto& t : q.terms)
            {
                auto posting = getLocations(t);
                if (!posting.empty())
                    postingList.push_back(std::move(posting));
            }

            if (postingList.size() <= 1)
                return getApproximateWords();

            return alex::utils::getMinPwDist(std::move(postingList), getApproximateWords()) / postingList.size();
        };

        virtual std::map<std::pair<HiddenQuery::HiddenTerm, HiddenQuery::HiddenTerm>, int> getMinPairWiseDistances(const HiddenQuery& q) const
        {
            std::vector<std::vector<uint32_t>> postingList;
            std::vector<HiddenQuery::HiddenTerm> postingTerm;
            for (auto t : q.terms)
            {
                auto posting = getLocations(t);
                if (!posting.empty())
                {
                    postingTerm.push_back(std::move(t));
                    postingList.push_back(std::move(posting));
                }
            }
            alex::utils::MatrixInt minPair = alex::utils::getMinPwDistances(std::move(postingList));
            std::map<std::pair<HiddenQuery::HiddenTerm, HiddenQuery::HiddenTerm>, int> pairs;
            for (size_t i = 1; i < minPair.size(); ++i)
            {
                for (size_t j = 0; j < i; ++j)
                    pairs[std::make_pair(postingTerm[i], postingTerm[j])] = minPair[i][j];
            }
            return pairs;
        };
        virtual void dump(int depth) { throw std::exception("Not Implemented"); };
        virtual std::string toString() const = 0;
    };

    typedef std::shared_ptr<alex::index::ISecureIndex> SecureIndex;
}}

#endif