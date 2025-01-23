#ifndef __PSI_BUILDER_H__
#define __PSI_BUILDER_H__

#include "Psi.h"
#include "TextProcessor.h"
#include "SHA.h"
#include <vector>
#include <string>
#include <unordered_map>

namespace alex { namespace secure_index
{
    template <unsigned int N>
    std::vector<std::string> crypto(const std::string& term, const std::vector<std::string>& secretKeys, const std::string& salt)
    {
        std::vector<std::string> results;
        results.reserve(secretKeys.size());
        for (const auto& secret : secretKeys)
        {
            results.push_back(alex::crypt::hexdigest<N>(alex::crypt::hexdigest<N>(term + "|" + secret) + "|" + salt));
        }
        return results;
    }

    template <unsigned int N = 12>
    class PsiBuilder
    {
    public:
        class Config
        {
        public:
            double getFalsePositiveRate() const
            {
                return 1.0 / std::pow(2, getFalsePositiveBits());
            };

            unsigned int getFalsePositiveBits() const
            {
                return _fpBits;
            };

            void setFalsePositiveBits(unsigned int n = 10)
            {
                if (n == 0)
                    throw std::exception("False Positive Bits Must Be An Integer Greater Than Zero");
                _fpBits = n;
            };

            void setFalsePositiveRate(double rate = 0.0009765625)
            {
                if(rate <= 0 || rate >= 1)
                    throw std::exception("False Positive Rate Must Be In Range (0, 1)");
                _fpBits = (uint32_t)std::ceil(-std::log(rate) / std::log(2));
            };

            cmph_cpp::CMPH_ALGO getAlgorithm() const
            {
                return _algo;
            };

            void setAlgorithm(cmph_cpp::CMPH_ALGO algo = cmph_cpp::CMPH_CHD)
            {
                _algo = algo;
            };

            double getLoadFactor() const
            {
                return _load;
            };

            void setLoadFactor(double load = 1)
            {
                if(load <= 0 || load > 1)
                    throw std::exception("Load Factor Must Be In Range (0, 1]");
                _loadFactor = load;
            };

            const std::vector<std::string>& getSecretKeys() const
            {
                return secretKeys;
            };

            void readSecretKeys(std::istream& file)
            {
            };

            void writeSecretKeys(std::ostream& file)
            {
            };

            void addSecretKey(const std::string& key)
            {
            };

            void removeSecretKey(const std::string& key)
            {
            };

            Config()
            {
                defaults();
            };

            void readConfig(std::istream& file)
            {
            };

            void writeConfig(std::ostream& file)
            {
            };

            void defaults()
            {
                _kgrams = 2;
                _algo = cmph_cpp::CMPH_CHD_PH;
                _loadFactor = 1.0;
                _verbose = false;
                _secretKeys = std::vector<std::string>({"secret"});
                _fpBits = 10;
                // wordPdf = uniform
            };

            void setVerbosityLevel(unsigned int lvl = 0)
            {
                switch (lvl)
                {
                    case 0: _verbose = false; break;
                    case 1: _verbose = true; break;
                }
            };

            unsigned int getVerbosityLevel() const
            {
                return (unsigned int)_verbose;
            };

            void setKgrams(unsigned int k = 1)
            {
                if (k == 0)
                    throw std::invalid_argument("k Must Be In Range [1, 2, ...)");

                _kgrams = k;
            };

            unsigned int getKgrams() const
            {
                return _kgrams;
            };

            void setGaps(unsigned int g = 0)
            {
                _gaps = g;
            };

            unsigned int getGaps() const
            {
                return _gaps;
            };

        private:
            // CategoricalDistribution<std::string, float> _wordDist;
            unsigned int _kgrams;
            unsigned int _gaps;
            bool _verbose;
            uint32_t _fpBits;
            double _loadFactor;
            std::vector<std::string> _secretKeys;
            cmph_cpp::CMPH_ALGO _algo;
        };

        PsiBuilder() {}

        PsiBuilder(const Config& cfg) : _cfg(cfg) {}

        alex::secure_index::Psi<N> build(const std::vector<std::vector<std::string>>& words)
        {
            if (words.empty() || words[0].empty())
                throw std::exception("No Searchable Terms");

            auto terms = makeTerms(words);
            alex::secure_index::Psi<N> psi;
            uint32_t sz = psi.build(terms, i, _cfg.verbose, _cfg.loadFactor, _cfg.algo);
            psi._ref = _ref;
            psi._approximateWords = words.size();

            for (unsigned int j = 0; j < i; ++j)
                delete [] terms[j];
            delete [] terms;

            return psi;
        };

        void makeTerms(const std::vector<std::vector<std::string>>& words)
        {
            std::unordered_map<std::string, std::string> hashes;
            size_t n = 0;
            for (const auto& wordp : words)
            {
                n += wordp.size();
                for (const auto& x : wordp)
                {

                }
            }

            char** terms = new char*[words.size() * _cfg.getSecretKeys().size()];
            unsigned int i = 0;
            for (const auto& x = _terms.begin(); x != _terms.end(); ++x)
            {
                for (const auto& h : x->second.hash)
                {
                    terms[i] = new char[N + 1];
                    std::memcpy(terms[i], h.c_str(), N);
                    terms[i][N] = 0;
                    ++i;
                }
            }

            auto& word = _terms[_words[0]];
            auto crypts = alex::utils::cryptoTerms(_words[0], _cfg.getSecretKeys(), _ref);
            for (auto crypt : crypts)
                word.hash.push_back(std::move(crypt));                   

            std::string kgram;
            for (unsigned int i = 1; i < _words.size(); ++i)
            {
                if (_terms.count(_words[i]) == 0)
                {
                    auto& word = _terms[_words[i]];
                    word.freq = 0;
                    const auto& crypts = alex::utils::cryptoTerms(_words[i], _cfg.secrets, _ref);
                    for (auto crypt : crypts)
                        word.hash.push_back(std::move(crypt));                   
                }

                bigram = _words[i - 1] + " " + _words[i];
                if (_terms.count(bigram) == 0)
                {
                    auto& word = _terms[bigram];
                    const auto& crypts = alex::utils::cryptoTerms(bigram, _cfg.secrets, _ref);
                    for (auto crypt : crypts)
                        word.hash.push_back(std::move(crypt));                   
                }
            }
        };

        std::string getPreferredFileExtension() const { return Psi::HEADER; };

    private:
        Config _cfg;
        std::string _ref;
        alex::text::TextProcessor _proc;
        std::unordered_map<std::string, std::vector<std::string>> _terms;
    };
}}

#endif