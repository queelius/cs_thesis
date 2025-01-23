#include "Utils.h"
#include "porter2_stemmer.h"
#include <regex>

using namespace alex::utils;
using namespace alex;

std::vector<Str> alex::utils::stemmer(const std::vector<Str>& words)
{
    std::vector<Str> stems;
    Str stem;
    for (Str word : words)
    {
        stem = word;
        Porter2Stemmer::stem(stem.data());
        stems.push_back(stem);
    }
    return stems;
}

std::vector<Str> alex::utils::words(Str message)
{
    const std::regex r(WORD_BOUNDARY);
    const std::regex_iterator<std::string::iterator> end;
    const std::regex_iterator<std::string::iterator> begin(message.data().begin(),
            message.data().end(), r);
    std::vector<Str> words;
    for (auto i = begin; i != end; ++i)
    {
        words.push_back(Str((*i)[0]));
    }
    return words;
}

std::vector<std::vector<Str>> alex::utils::terms(Str message)
{
    // must see a "; then, can see whitespace, 0 or more;
    // then, must see at least one non-whitespace
    const std::regex r("\\\"[^\\\"]+\\\"");
    const std::regex_iterator<std::string::iterator> end;
    const std::regex_iterator<std::string::iterator> begin(message.data().begin(),
            message.data().end(), r);
    std::vector<std::vector<Str>> _terms;
    for (auto i = begin; i != end; ++i)
    {
        std::string s = (std::string)((*i)[0]);

        std::vector<Str> term;
        const std::regex r2(WORD_BOUNDARY);
        const std::regex_iterator<std::string::iterator> end2;
        const std::regex_iterator<std::string::iterator> begin2(s.begin(), s.end(), r2);

        for (auto j = begin2; j != end2; ++j)
        {
            term.push_back((Str)(*j)[0]);
        }
        if (!term.empty())
            _terms.push_back(term);
    }
    message = message.replace("\\\"[^\\\"]+\\\"", " ");

    auto wrds = words(message);
    for (auto wrd : wrds)
    {
        std::vector<Str> t;
        t.push_back(wrd);
        _terms.push_back(t);
    }

    return _terms;
}

std::vector<Str> alex::utils::removeStopWords(const std::vector<Str>& words,
                                        const std::vector<Str>& stopWords)
{
    std::vector<Str> results;

    for (const auto& word : words)
    {
        bool include = true;
        for (const auto& stopWord : stopWords)
        {
            if (word == stopWord)
            {
                include = false;
                break;
            }
        }

        if (include)
        {
            results.push_back(word);
        }
    }

    return results;
}

std::vector<Str> alex::utils::ngrams(const std::vector<Str>& words, size_t minGram,
                               size_t maxGram, bool lexographic)
{
    std::vector<Str> ngrams;
    for (size_t j = 0; j < words.size(); ++j)
    {
        std::vector<Str> kgram;
        for (size_t k = j; k < std::min(words.size(), j + maxGram); ++k)
        {
            if (lexographic)
            {
                kgram.insert(std::lower_bound(kgram.begin(), kgram.end(), words[k]),
                             words[k]);
            }
            else
            {
                kgram.push_back(words[k]);
            }

            if (kgram.size() < minGram)
            {
                continue;
            }

            Str gramStr = kgram[0];
            for (size_t i = 1; i < kgram.size(); ++i)
            {
                gramStr = gramStr + " " + kgram[i];
            }
            ngrams.push_back(gramStr);
        }
    }

    return ngrams;
}

std::vector<Str> alex::utils::lowerCase(const std::vector<Str>& words)
{
    std::vector<Str> results;
    for (const auto& word : words)
    {
        results.push_back(word.lower());
    }
    return results;
}
