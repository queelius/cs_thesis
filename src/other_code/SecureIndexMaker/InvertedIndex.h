#ifndef __INVERTED_INDEX_H__
#define __INVERTED_INDEX_H__

#include <iostream>
#include <string>
#include <list>
#include <cstdint>
#include "Query.h"
#include "FileSystem.h"
#include "BinaryIO.h"
#include "Utils.h"
#include <map>

namespace alex { namespace index
{
    // this is a simple inverted index -- it's slow, as it
    // indexes documents (there is no global index). i didn't
    // bother with that since i wanted simplicity over
    // sophistication -- it must be correct, and it must
    // support any desirable relevancy scoring i need.
    //
    // i almost just made an "online" index, i.e.,
    // throw the words into a vector and scan it.
    // but, since i may be using large data sets, i
    // worried this would be too slow.

    typedef std::vector<std::pair<uint32_t, uint32_t>> PostingList;

    class InvertedIndex
    {
        friend class InvertedIndexBuilder;

    public:
        static const uint8_t VERSION = 1;
        static const char HEADER[];

        void read(const boost::filesystem::path& infile)
        {
            read(std::ifstream(infile.string(), std::ios::binary));
        };

        void write(const boost::filesystem::path& outfile)
        {
            write(std::ofstream(outfile.string(), std::ios::binary));
        };

        void read(std::istream& file)
        {
            using namespace alex::io::binary;

            const std::string& hdr = readString(file);
            if (hdr != HEADER)
                throw std::exception("Unexpected Header");

            unsigned int version = readU8(file);
            if (version != VERSION)
                throw std::exception("Unexpected Verion");

            // get reference (e.g., url) for document
            _ref = readString(file);

            _termCount = readVarU32(file);

            const unsigned int size = readVarU32(file);
            for (unsigned int i = 0; i < size; ++i)
            {
                const std::string& word = readString(file);
                _words[word] = PostingList();

                auto& w = _words[word];
                const unsigned int numPosts = readVarU32(file);
                w.reserve(numPosts);
                for (unsigned int j = 0; j < numPosts; ++j)
                {
                    const uint32_t x = readVarU32(file);
                    const uint32_t y = readVarU32(file);
                    w.push_back(std::make_pair(x, y));
                }
            }
        };

        void write(std::ostream& file)
        {
            using namespace alex::io::binary;

            writeString(file, HEADER);
            writeU8(file, VERSION);

            writeString(file, _ref);

            writeVarU32(file, _termCount);

            writeVarU32(file, (uint32_t)_words.size());

            for (const auto& word : _words)
            {
                writeString(file, word.first);
                writeVarU32(file, (uint32_t)word.second.size());
                for (const auto& x : word.second)
                {
                    writeVarU32(file, x.first);
                    writeVarU32(file, x.second);
                }
            }
        };

        unsigned int getWordCount() const
        {
            return _termCount;
        };

        std::string getReference() const
        {
            return _ref;
        };

        void setReference(const std::string& ref)
        {
            _ref = ref;
        };

        void clear()
        {
            _words.clear();
            _ref = "";
        };

        std::string getPreferredFileExtension() const
        {
            return HEADER;
        };

        std::vector<uint32_t> getLocations(const Query::Term& t) const
        {
            if (t.empty())
                return std::vector<uint32_t>();

            for (const auto& x : t)
            {
                if (_words.count(x) == 0)
                    return std::vector<uint32_t>();
            }

            const auto& first = _words.at(t[0]);
            std::vector<uint32_t> locs;
            for (const auto& x : first)
            {
                if (dfs(t, 1, x.second+1, locs))
                    locs.push_back(x.first);
            }

            std::sort(locs.begin(), locs.end());
            auto newEnd = std::unique(locs.begin(), locs.end());
            locs.erase(newEnd, locs.end());
            return locs;
        };

        // reasonable fast algorithm to retrieve minimum pairwise distances between terms
        // note that a term is anything from a single word to an n-gram, so you can find
        // the minimum distance between the phrases "tom sawyer and huck finn" and "this is another phrase".
        std::map<std::pair<Query::Term, Query::Term>, int> getMinPairWiseDistances(const Query& q) const
        {
            std::vector<std::vector<uint32_t>> postingList;
            std::vector<Query::Term> postingTerm;
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
            std::map<std::pair<Query::Term, Query::Term>, int> pairs;
            for (size_t i = 1; i < minPair.size(); ++i)
            {
                for (size_t j = 0; j < i; ++j)
                    pairs[std::make_pair(postingTerm[i], postingTerm[j])] = minPair[i][j];
            }
            return pairs;
        };

        double getMinPairwiseScore(const Query& q) const
        {
            std::vector<std::vector<uint32_t>> postingList;
            for (auto t : q.terms)
            {
                auto posting = getLocations(std::move(t));
                if (!posting.empty())
                    postingList.push_back(std::move(posting));
            }

            if (postingList.size() <= 1)
                return getWordCount();

            return alex::utils::getMinPwDist(postingList, getWordCount()) / postingList.size();

            //double d = alex::utils::getMinPwDist(std::move(postingList), alex::globals::max_min_pair_dist) / q.terms.size();
            //return std::log(alex::globals::alpha_min_pair + std::exp(-alex::globals::beta_min_pair * d));
        };

        unsigned int getFrequency(const Query::Term& t) const
        {
            if (t.empty())
                return false;

            for (const auto& x : t)
            {
                if (_words.count(x) == 0)
                    return false;
            }

            if (t.size() == 1)
                return _words.at(t[0]).size();

            size_t freq = 0;
            for (const auto& x : _words.at(t[0]))
            {
                if (dfs(t, 1, x.second+1))
                    ++freq;
            }

            return freq;
        };

        bool containsAny(const Query& q) const
        {
            for (const auto& t : q.terms)
            {
                if (contains(t))
                    return true;
            }
            return false;
        };

        bool containsAll(const Query& q) const 
        {
            for (const auto& t : q.terms)
            {
                if (!contains(t))
                    return false;
            }
            return true;
        };

        // contains term t, which can be an exact phrase
        // note: since t may be stopped/stemmed, and the
        // same may be true of index, "exact" is used
        // with some liberty here.
        bool contains(const Query::Term& t) const
        {
            if (t.empty())
                return false;

            for (const auto& x : t)
            {
                if (_words.count(x) == 0)
                    return false;
            }

            for (const auto& x : _words.at(t[0]))
            {
                if (dfs(t, 1, x.second+1))
                    return true;
            }
            return false;
        };

        bool hasProximity() const { return true; };
        bool hasFrequency() const { return true; };
        std::string toString() const { throw std::exception("Not Implemented"); };

        void print(std::ostream& outs = std::cout)
        {
            outs << "Reference: " << _ref << std::endl;

            for (const auto& word : _words)
            {
                outs << std::setw(20) << word.first << " -> ";
                for (unsigned int i = 0; i < word.second.size(); ++i)
                {
                    if (i != 0)
                        outs << ", ";
                    outs << word.second[i].first;
                }
                outs << std::endl;
            }
        };

    private:
        std::string _ref;
        std::map<std::string, PostingList> _words;
        unsigned int _termCount;

        bool dfs(const alex::index::Query::Term& t, unsigned int idx, unsigned int expect) const
        {
            if (idx == t.size())
                return true;

            for (const auto& x : _words.at(t[idx]))
            {
                if (x.second == expect)
                    return dfs(t, idx+1, expect+1);
            }
            return false;
        };

        bool dfs(const alex::index::Query::Term& t, unsigned int idx, unsigned int expect, std::vector<uint32_t>& locs) const
        {
            if (idx == t.size())
                return true;

            for (const auto& x : _words.at(t[idx]))
            {
                if (x.second == expect)
                {
                    if (dfs(t, idx+1, expect+1, locs))
                    {
                        locs.push_back(x.first);
                        return true;
                    }
                }
            }
            return false;
        };

        bool dfs2(const alex::index::Query::Term& t, unsigned int idx, unsigned int expect, std::vector<std::pair<uint32_t, uint32_t>>& locs) const
        {
            if (idx == t.size())
                return true;

            for (const auto& x : _words.at(t[idx]))
            {
                if (x.second == expect)
                {
                    if (dfs2(t, idx+1, expect+1, locs))
                    {
                        locs.push_back(x);
                        return true;
                    }
                }
            }
            return false;
        };
    };

    typedef std::shared_ptr<InvertedIndex> Index;

    const char InvertedIndex::HEADER[] = "idx";
}}

#endif