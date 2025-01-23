#ifndef __TOKEN_DISTRIBUTION_H__
#define __TOKEN_DISTRIBUTION_H__

#include <random>
#include <iostream>
#include "BinaryIO.h"
#include "DiscreteDistribution.h"
#include "Entropy.h"
#include "FileSystem.h"
#include <set>
#include <unordered_set>
#include <vector>
#include <iterator>
#include <map>
#include <functional>

namespace
{
    std::string makeRandomToken(size_t minSize, size_t maxSize, const std::vector<char>& alphabet, alex::stochastic::Entropy<>& ent)
    {
        std::string word;
        size_t sz = ent.getInt(minSize, maxSize);
        word.reserve(sz);
        for (size_t i = 0; i < sz; ++i)
            word.push_back(alphabet[ent.get() % alphabet.size()]);
        return word;
    }
}

namespace alex { namespace stochastic
{
    class TokenDistribution
    {
    public:
        struct Params
        {
            static const unsigned int ZIPF = 0;
            static const unsigned int UNIFORM = 1;

            size_t uniqueTokens;
            size_t minTokenSize;
            size_t maxTokenSize;
            std::vector<char> alphabet;
            unsigned int type;
            unsigned int seed;
            
            bool isValid() const
            {
                return !(uniqueTokens == 0 ||
                        minTokenSize == 0 ||
                        maxTokenSize == 0 ||
                        maxTokenSize < minTokenSize ||
                        type > 1);
            };

            Params()
            {
                defaults();
            };

            void defaults()
            {
                uniqueTokens = 0;
                minTokenSize = 1;
                maxTokenSize = 12;
                alphabet = std::vector<char>{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
                    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
                    't', 'u', 'v', 'w', 'x', 'y', 'z' };
                seed = 0;
                type = ZIPF;
            };
        };

        static TokenDistribution makeRandom(const Params& params)
        {
            alex::stochastic::DiscreteDistribution dd;
            switch (params.type)
            {
                case Params::ZIPF: dd = alex::stochastic::DiscreteDistribution::zipf(params.uniqueTokens); break;
                case Params::UNIFORM: dd = alex::stochastic::DiscreteDistribution::uniform(params.uniqueTokens); break;
                default: throw std::exception("Invalid Model Type");
            }

            alex::stochastic::Entropy<> entropy(params.seed);
            return makeRandom(params.uniqueTokens, params.minTokenSize,
                              params.maxTokenSize, params.alphabet,
                              dd, entropy);
        };

        static TokenDistribution makeRandom(
            size_t uniqueTokens,
            size_t minTokenSize,
            size_t maxTokenSize,
            const std::vector<char>& alphabet,
            const alex::stochastic::DiscreteDistribution& dist,
            alex::stochastic::Entropy<>& entropy)
        {
            if (maxTokenSize < minTokenSize)
                maxTokenSize = minTokenSize;

            if (minTokenSize == 0)
                throw std::exception("Invalid Argument: minimum token size must be > 0");

            // a simple check to make sure its possible to find
            // specified number of unique words given the alphabet
            // and word size. note that this may be extremely slow
            // if |alphabet|^word_size is only somewhat larger than
            // number of unique words.
            if (std::pow(alphabet.size(), maxTokenSize) < uniqueTokens)
                throw std::exception("Invalid Argument: insufficient parameter space for uniqueWords");

            // verify that alphabet consists of unique characters
            std::set<char> chars;
            for (auto c : alphabet)
            {
                if (chars.count(c) != 0)
                    throw std::exception("Invalid Argument: alphabet must consist of unique characaters");
                chars.insert(c);
            }
            chars.clear();
            std::set<std::string> tokenSet;

            // let's make absolutely sure that the word is unique.
            // this could probably be skipped if sensible inputs
            // are used.
            while (tokenSet.size() < uniqueTokens)
                tokenSet.insert(makeRandomToken(minTokenSize, maxTokenSize, alphabet, entropy));

            TokenDistribution tokDist;
            std::move(tokenSet.begin(), tokenSet.end(), std::back_inserter(tokDist._tokens));
            std::random_shuffle(tokDist._tokens.begin(), tokDist._tokens.end());

            tokDist._dist = dist;
            return tokDist;
        };

        TokenDistribution() {};

        TokenDistribution(const std::vector<std::pair<std::string, float>>& weights)
        {
            _tokens.reserve(weights.size());
            for (size_t i = 0; i < weights.size(); ++i)
                _tokens.push_back(weights[i].first);

            std::vector<float> v;
            v.reserve(weights.size());
            for (const auto& p : weights)
                v.push_back(p.second);

            _dist = alex::stochastic::DiscreteDistribution::fromPDF(v.begin(), v.end());
        };

        std::string inverseCdf(float p)
        {
            return _tokens[_dist.inverseCdf(p)];
        };
        
        double pdf(const std::string& x) const
        {
            for (size_t i = 0; i < _tokens.size(); ++i)
                if (x == _tokens[i])
                    return _dist.pdf(i);
            return 0;
        };

        DiscreteDistribution getDistribution() const
        {
            return _dist;
        };

        size_t size() const
        {
            return _tokens.size();
        };

        void write(std::ostream& file)
        {
            file << _tokens.size() << std::endl;
            for (size_t i = 0; i < _tokens.size(); ++i)
                file << _tokens[i] << "\t" << _dist.pdf(i) << std::endl;
        };
        
        void read(std::istream& file)
        {
            _tokens.clear();
        
            size_t n; file >> n;
            _tokens.resize(n);
            std::vector<float> pdf(n);
            for (size_t i = 0; i < n; ++i)
            {
                //if (!file.good())
                    //throw std::exception("Error Reading File");
                    
                file >> _tokens[i];
                file >> pdf[i];
            }
            
            _dist = alex::stochastic::DiscreteDistribution::fromPDF(pdf.begin(), pdf.end());
        };       

        template <class T>
        TokenDistribution reduce(size_t n, alex::stochastic::Entropy<T>& entropy)
        {
            if (n > _tokens.size())
                throw std::exception("Invalid Argument");

            std::unordered_set<unsigned int> tokens;
            while (tokens.size() < n)
                tokens.insert(_dist.inverseCdf(entropy.get0_1()));

            std::vector<std::pair<std::string, float>> pdf;
            pdf.reserve(n);

            for (auto x : tokens)
                pdf.push_back(std::make_pair(_tokens[x], _dist.pdf(x)));

            return TokenDistribution(std::move(pdf));
        };       
        
        void writeBinary(std::ostream& file)
        {
            alex::io::binary::writeString(file, "tokdist");
            alex::io::binary::writeU8(file, 1);
            alex::io::binary::writeVarU32(file, (uint32_t)_tokens.size());
            for (const auto& token : _tokens)
                alex::io::binary::writeString(file, token);
            _dist.writeBinary(file);
        };

        void readBinary(std::istream& file)
        {
            const std::string hdr = alex::io::binary::readString(file);
            if (hdr != "tokdist")
                throw std::exception(("Unexpected Header: " + hdr).c_str());

            uint8_t version = alex::io::binary::readU8(file);
            if (version != 1)
                throw std::exception(("Unexpected Version: " + std::to_string(version)).c_str());

            _tokens.clear();
            uint32_t n = alex::io::binary::readVarU32(file);
            _tokens.reserve(n);
            for (uint32_t i = 0; i < n; ++i)
            {
                std::string token = alex::io::binary::readString(file);
                _tokens.push_back(token);
            }
            _dist.readBinary(file);
        };

    private:
        std::vector<std::string> _tokens;
        alex::stochastic::DiscreteDistribution _dist;
    };
}}

#endif