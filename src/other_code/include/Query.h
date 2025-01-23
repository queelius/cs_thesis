#ifndef __QUERY_H__
#define __QUERY_H__

#include <vector>
#include <string>
#include <algorithm>
#include <unordered_set>
#include "Porter2Stemmer.h"

namespace alex { namespace index { namespace new_query_interface
{
    class Query
    {
    public:
        class Term
        {
        public:
            std::string toString() const
            {
                if (tokens.empty())
                {
                    return "";
                }
                else if (tokens.size() == 1)
                {
                    return tokens[0];
                }
                else
                {
                    std::string result = "\"";
                    for (size_t i = 0; i < tokens.size(); ++i)
                    {
                        if (i != 0)
                            result += " ";
                        result += tokens[i];
                    }
                    result += "\"";
                    return result;
                }
            };

            void clear() { tokens.clear(); };

            bool empty() const { return tokens.empty(); };

            size_t size() const { return tokens.size(); };

            std::string& operator[](size_t index)
            {
                if (index >= tokens.size())
                    throw std::exception("Invalid Token-Term Index");

                return tokens[index];
            };

            std::string operator[](size_t index) const
            {
                if (index >= tokens.size())
                    throw std::exception("Invalid Token-Term Index");

                return tokens[index];
            };

            void addToken(const std::string& token)
            {
                tokens.push_back(token);
            };

            bool operator<(const Term& other) const
            {
                if (empty())
                    return true;
                else if (other.empty())
                    return false;

                for (size_t i = 0; i < std::min(size(), other.size()); ++i)
                {
                    if (tokens[i] != other.tokens[i])
                        return tokens[i] < other.tokens[i];
                }
                return size() < other.size();
            };

        private:
            std::vector<std::string> tokens;
        };

        std::string toString() const
        {
            std::string result;
            for (size_t i = 0; i < terms.size(); ++i)
            {
                const std::string t = terms[i].toString();
                if (t.empty())
                    continue;

                if (i != 0)
                    result += " ";
                result += t;
            }

            return result;
        };

        Term operator[](size_t index) const
        {
            if (index >= terms.size())
                throw std::exception("Invalid Term Index");

            return terms.at(index);
        };

        Term& operator[](size_t index)
        {
            if (index >= terms.size())
                throw std::exception("Invalid Term Index");

            return terms[index];
        };

        size_t size() const { return terms.size(); };

        bool empty() const { return terms.empty(); };

        bool operator<(const Query& other) const
        {
            if (empty())
                return true;
            else if (other.empty())
                return false;

            for (size_t i = 0; i < std::min(size(), other.size()); ++i)
            {
                if (terms[i] < other.terms[i])
                    return false;
                else if (other.terms[i] < terms[i])
                    return true;
            }
            return size() < other.size();
        };

        void addTerm(const Term& term)
        {
            terms.push_back(term);
        };

        void clear()
        {
            terms.clear();
        };

    private:
        std::vector<Term> terms;
    };

    class QueryBuilder
    {
        friend class HiddenQueryBuilder;

    public:
        struct Params
        {
            bool stemming;
            std::unordered_set<std::string> ignoreTokens;

            Params()
            {
                defaults();
            };

            void defaults()
            {
                stemming = false;
                ignoreTokens.clear();
            };
        };

        QueryBuilder() {};

        QueryBuilder(const Params& params) : _params(params) {};

        // <query> = <term> | <term> <query>
        // <term>  = <alphanumeric_string> | "<query>"

        Query make(std::string query)
        {
            bool phraseMode = false;
            Query::Term term;
            std::string token;
            std::set<Query::Term> terms;

            query += " ";
            for (size_t i = 0; i < query.size(); ++i)
            {
                if (query[i] == '\'')
                    continue;
                else if (isalnum(query[i]))
                    token += tolower(query[i]);
                else
                {
                    if (query[i] == '"')
                        phraseMode = !phraseMode;

                    if (!token.empty())
                    {
                         if (_params.ignoreTokens.count(token) == 0)
                         {// stem token
                            if (_params.stemming)
                                alex::porter2stemmer::stem(token);
                            term.addToken(std::move(token));
                         }
                        token.clear();    
                    }

                    if (!phraseMode || i == query.size() - 1)
                    {
                        if (!term.empty())
                        {
                            terms.insert(std::move(term));
                            term.clear();
                        }
                    }
                }
            }

            Query q;
            for (const auto& term : terms)
                q.addTerm(term);
            return q;
        };

        Params getParams() const
        {
            return _params;
        };

        Params& getParams()
        {
            return _params;
        };

    private:
        Params _params;
    };
}}}

#endif