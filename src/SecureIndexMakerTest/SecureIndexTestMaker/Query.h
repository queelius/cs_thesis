#ifndef __QUERY_H__
#define __QUERY_H__

#include <vector>
#include <string>
#include <algorithm>
#include <unordered_set>
#include "Porter2Stemmer.h"
#include "Globals.h"

namespace alex { namespace index
{
    class QueryBuilder;

    struct Query
    {
        typedef std::vector<std::string> Term;
        std::vector<Term> terms;

        std::string toString() const
        {
            std::string result;
            for (size_t i = 0; i < terms.size(); ++i)
            {
                if (terms[i].size() == 1)
                {
                    if (i != 0)
                        result += " ";
                    result += terms[i][0];
                }
                else
                {
                    if (i != 0)
                        result += " ";
                    result += "\"";
                    for (size_t j = 0; j < terms[i].size(); ++j)
                    {
                        if (j != 0)
                            result += " ";
                        result += terms[i][j];
                    }
                    result += "\"";
                }
            }

            return result;
        };

        Term operator[](size_t idx) const
        {
            if (idx >= terms.size())
                throw std::exception("Invalid Index");

            return terms[idx];
        };

        void clear()
        {
            terms.clear();
        };
    };

    class QueryBuilder
    {
        friend class HiddenQueryBuilder;

    public:
        struct BuilderParams
        {
            bool stemming;
            std::unordered_set<std::string> ignoreWords;

            BuilderParams()
            {
                defaults();
            };

            void defaults()
            {
                stemming = alex::globals::default_stemming;
                ignoreWords = alex::globals::default_stop_words;
            };
        };

        QueryBuilder() {};

        QueryBuilder(const BuilderParams& params) : _params(params) {};

        Query make()
        {
            return _query;
        };

        // <query> = <term> | <term> <query>
        // <term>  = <alphanumeric_string> | "<query>"
        
        void parse(std::string query)
        {
            _query.terms.clear();
            bool phraseMode = false;

            Query::Term term;
            std::string word;

            for (size_t i = 0; i < query.size(); ++i)
            {
                // ignore apostrophes, e.g., "isn't" -> "isnt"
                if (query[i] == '\'')
                    continue;
                else if (query[i] == '"')
                {
                    phraseMode = !phraseMode;

                    if (!word.empty())
                    {
                        term.push_back(word);
                        word = "";
                    }

                    if (!term.empty())
                    {
                        _query.terms.push_back(term);
                        term.clear();
                    }
                }
                else
                {
                    if (isalnum(query[i]))
                        word += tolower(query[i]);
                    else
                    {
                        if (!word.empty())
                        {
                            term.push_back(word);
                            word = "";
                        }
                        if (!phraseMode && !term.empty())
                        {
                            _query.terms.push_back(term);
                            term.clear();
                        }
                    }
                }
            }

            if (!word.empty())
                term.push_back(word);
            if (!term.empty())
                _query.terms.push_back(term);
        };

        void stem()
        {
            for (auto& term : _query.terms)
            {
                for (auto& word : term)
                    porter2stemmer::stem(word);
            }
        };

        void removeWords()
        {
            removeWords(_params.ignoreWords);
        };

        void removeWords(const std::unordered_set<std::string>& words)
        {
            for (auto& term : _query.terms)
            {
                term.erase(std::remove_if(term.begin(), term.end(), [&words](const std::string& x) -> bool
                    { return words.count(x) != 0; }), term.end());
            }
            _query.terms.erase(std::remove_if(_query.terms.begin(), _query.terms.end(), [](const std::vector<std::string>& v) {
                return v.empty(); }), _query.terms.end());
        };

        void removeRedundancies()
        {
            auto& ts = _query.terms;
            std::sort(ts.begin(), ts.end());
            auto newEnd = std::unique(ts.begin(), ts.end());
            ts.erase(newEnd, ts.end());       
        };

        Query make(const std::string& q)
        {
            parse(q);
            normalize();
            return make();
        }

        void normalize()
        {
            removeWords();
            if (_params.stemming)
                stem();
            removeRedundancies();
        };

        void clear()
        {
            _query.terms.clear();
        };

    private:
        Query _query;
        BuilderParams _params;
    };
}}

#endif