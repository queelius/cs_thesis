#ifndef __TREE_GRAM_H__
#define __TREE_GRAM_H__

#include <map>
#include <algorithm>
#include <string>
#include <ostream>
#include "Str.h"

namespace alex
{

using std::vector;
using alex::Str;

struct TreeGram
{
    TreeGram()
    {
        this->_maxGramSize = 0;
        this->root = Node { 0, "" };
    };

    struct Node
    {
        double p;
        Str word;
        vector<Node> child;

        bool operator<(const Node& other) const { return word < other.word; };
    };

    size_t maxGramSize()
    {
        return _maxGramSize;
    };

    void print(std::ostream& os)
    {
        printHelper(root, 0, os);
    };

    void addGram(const vector<Str>& ngram)
    {
        _maxGramSize = std::max(_maxGramSize, ngram.size());
        addGramHelper(root, ngram, 0);
    };

    vector<Node> getNext()
    {
        return root.child;
    };

    vector<Node> getNext(const vector<Str>& ngram)
    {
        return getNextHelper(root, ngram, 0);
    };

    double probability(const std::vector<Str>& ngram)
    {

        // Markov assumption, second order:
        //
        //     P[x{i} | x{1}, x{2}, ..., x{i-2}, x{i-1}] = P[x{i} | x{i-2}, x{i-1}] 
        //
        // That is, x{i} is conditionally independent of x{1}, x{2}, ..., x{i-3}. This is the trigram model.
        //
        // Trigram model:
        // MLE: P[x{i} | x{i-2}, x{i-1}] = Count(x{i} AND x{i-1} AND x{i-2}) / Count(x{i-1} AND x{i-2})
        //                               = Count(trigrams) / Count(bigrams)
        //
        // Bigram model:
        // MLE: P[x{i} | x{i-1}] = Count(x{i} AND x{i-1}) / Count(x{i-1})
        //                       = Count(bigrams) / Count(unigrams)
        //
        // Unigram model:
        // MLE: P[x{i}] = Count(x{i}) / M
        //              = Count(unigrams) / Count(all examples)
        //
        // Linear interpolated MLE:
        //
        //      P_LI[x{i} | x{i-2}, x{i-1}] = a * P_MLE[x{i} | x{i-2}, x{i-1}] + b * P_MLE[x{i} | x{i-1}] + c * P_MLE[x{i}],
        //          a + b + c = 1; a >= 0, b >= 0, c >= 0
        //
        //      Avoids problems of sparse data.
        //
        // Can learn a, b, c parameters by optimizing them on a test set.
        //
        // That is, take the product of the trigrams in the test set; more conveniently, take log of product:
        //
        //     = sum of log P_LI[x{i} | x{i-2}, x{i-1}] for each trigram, and we wish to maximize this (only a, b, and c are
        //       can be changed.
    };

    void makePdfMLE()
    {
        makePdfMLEHelper(root);
    };

    void makeCdfMLE()
    {
        makeCdfMLEHelper(root);
    };

    Str inverseCdf(double p)
    {
        return std::lower_bound(root.child.begin(), root.child.end(), Node { p }, [](const Node& a, const Node& b) { return a.p <= b.p; })->word;
    };

    Str inverseCdf(double p, const vector<Str>& conditionalGram)
    {
        const auto& ns = getNext(conditionalGram);
        return std::lower_bound(ns.begin(), ns.end(), Node { p }, [](const Node& a, const Node& b) { return a.p <= b.p; })->word;
    };

private:
    void makePdfLinearInterpolationMLE(Node& cur, const std::vector<Str>& testSet)
    {
    };

    void makeCdfMLEHelper(Node& cur)
    {
        double sum = 0;
        for (auto& child : cur.child)
        {
            sum += child.p;
            child.p = sum;
        }
        for (auto& child : cur.child)
        {
            child.p /= sum;
            makeCdfMLEHelper(child);
        }
    };

    void makePdfMLEHelper(Node& cur)
    {
        double sum = 0;
        for (const auto& child : cur.child)
        {
            sum += child.p;
        }
        for (auto& child : cur.child)
        {
            child.p /= sum;
            makePdfMLEHelper(child);
        }
    };

    vector<Node> getNextHelper(const Node& cur, const vector<Str>& ngram, size_t index)
    {
        if (index == ngram.size())
        {
            return cur.child;
        }

        const auto& ns = cur.child;
        const auto newNode = Node { 0, ngram[index] };
        const auto i = std::lower_bound(ns.begin(), ns.end(), newNode);

        if (i == ns.end())
        {
            throw std::exception("NGram Too Large");
        }
        else if (i->word != ngram[index])
        {
            throw std::exception("Word Not Found");
        }
        else
        {
            return getNextHelper(*i, ngram, index + 1);
        }
    };

    void addGramHelper(Node& parent, const vector<Str>& ngram, size_t ngramIdx)
    {
        if (ngramIdx == ngram.size())
        {
            return;
        }

        addGramHelper(addChild(parent, ngram[ngramIdx]), ngram, ngramIdx + 1);
    };

    Node& addChild(Node& parent, const Str& word)
    {
        auto& ns = parent.child;
        const auto& newNode = Node { 1, word };
        auto i = std::lower_bound(ns.begin(), ns.end(), newNode);

        if (i != ns.end() && i->word == word)
        {
            ++i->p;
            return *i;
        }
        else
        {
            return *(ns.insert(i, newNode));
        }
    };

    void printHelper(const Node& n, int depth, std::ostream& os)
    {
        if (depth > 0)
        {
            for (int i = 1; i < depth; ++i)
            {
                os << "\t";
            }
            
            os << n.word << " => " << n.p << " [" << n.child.size() << "]" << std::endl;
        }

        for (const auto& c : n.child)
        {
            
            printHelper(c, depth + 1, os);
        }
    };

    Node root;
    size_t _maxGramSize;
};

};

#endif
