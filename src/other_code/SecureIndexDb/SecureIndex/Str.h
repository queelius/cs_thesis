/********************************************
 * @author Alex Towell (queelius@gmail.com) *
 ********************************************/

#ifndef __STR_H__
#define __STR_H__

#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <regex>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace alex { namespace str
{
    /*************
     * Interface *
     *************/
    bool hasMatch(const std::string& x, const std::string& rx);
    std::vector<std::string> split(const std::string& x, const std::string& rx);
    bool hasSubmatch(const std::string& x, const std::string& rx);
    std::vector<std::string> matches(const std::string& x, const std::string& rx);
    std::vector<std::pair<size_t, std::string>> find(const std::string& x, const std::string& rx);
    std::pair<size_t, std::string> findFirst(const std::string& x, const std::string& rx);
    std::string repeat(const std::string& x, size_t n);

    std::string replace(const std::string& x, const std::string& matchRx, const std::string& replaceRx);
    void replaceInPlace(std::string& x, const std::string& matchRx, const std::string& replaceRx);

    std::string ltrim(const std::string& x);
    void ltrimInPlace(std::string& x);

    std::string rtrim(const std::string& x);
    void rtrimInPlace(std::string& x);

    std::string trim(const std::string& x);
    void trimInPlace(std::string& x);

    std::string lower(const std::string& x);
    void lowerInPlace(std::string& x);

    std::string upper(const std::string& x);
    void upperInPlace(std::string& x);

    std::string transform(const std::string& x, std::function<int(int)> f);
    void transformInPlace(std::string& x, std::function<int(int)> f);

    bool isAlpha(const std::string& x);
    bool isAlphaNumeric(const std::string& x);
    bool isLower(const std::string& x);
    bool isNumeric(const std::string& x);
    bool isInt(const std::string& x);
    bool isReal(const std::string& x);
    bool isPositiveInt(const std::string& x);

    /******************
     * Implementation *
     ******************/
    bool hasMatch(const std::string& x, const std::string& rx)
    {
        const std::regex r(rx);
        return std::regex_match(x.begin(), x.end(), r);
    }

    std::vector<std::string> split(const std::string& x, const std::string& rx)
    {
        static const std::sregex_iterator theEnd;
        const std::regex r(rx.data());
        std::vector<std::string> result;
        size_t last = 0;

        for(auto i = std::sregex_iterator(x.begin(), x.end(), r); i != theEnd; ++i)
        {
            result.push_back(x.substr(last, (i->position() - last)));
            last = i->position() + i->length();
        }

        if(last != x.size())
        {
            result.push_back(x.substr(last, x.size()));
        }

        return result;
    }

    std::vector<std::string> matches(const std::string& x, const std::string& rx)
    {
        const std::regex r(rx);
        const std::regex_iterator<std::string::const_iterator> end;
        const std::regex_iterator<std::string::const_iterator> begin(x.begin(), x.end(), r);

        std::vector<std::string> results;
        for(auto i = begin; i != end; ++i)
        {
            results.push_back(std::string((*i)[0]));
        }

        return results;
    }

    bool hasSubmatch(const std::string& x, const std::string& rx)
    {
        const std::regex r(rx.data());
        return std::regex_search(x.begin(), x.end(), r);
    }

    std::vector<std::pair<size_t, std::string>> find(const std::string& x, 
                                         const std::string& rx)
    {
        static const std::sregex_iterator theEnd;
        const std::regex r(x);
        std::vector<std::pair<size_t, std::string>> results;

        for(auto i = std::sregex_iterator(x.begin(), x.end(), r); i != theEnd; ++i)
        {
            results.push_back(std::pair<size_t, std::string>(i->position(), std::string(i->str())));
        }

        return results;
    }

    std::pair<size_t, std::string> findFirst(const std::string& x, const std::string& rx)
    {
        static const std::sregex_iterator end;
        const std::regex r(rx.data());
        const auto i = std::sregex_iterator(x.begin(), x.end(), r);

        if(i == end)
        {
            throw std::exception("No Match");
        }

        return std::make_pair(i->position(), std::string(i->str()));
    }

    std::string repeat(const std::string& x, size_t n)
    {
        std::string result;

        for(size_t i = 0; i < n; ++i)
        {
            result.append(x);
        }

        return result;
    }

    std::string replace(const std::string& x,
                        const std::string& matchRx,
                        const std::string& replaceRx)
    {
        const std::regex r(matchRx.data());
        std::string result;
        std::regex_replace(std::back_inserter(result), x.begin(), x.end(), r, replaceRx);
        return result;
    }

    void replaceInPlace(std::string& x,
                        const std::string& matchRx,
                        const std::string& replaceRx)
    {
        const std::regex r(matchRx.data());
        x.erase(std::regex_replace(x.begin(), x.begin(), x.end(), r, replaceRx), x.end());
    }

    std::string ltrim(const std::string& x)
    {
        return alex::str::replace(x, "^\\s+", "");
    }

    void ltrimInPlace(std::string& x)
    {
        alex::str::replaceInPlace(x, "^\\s+", "");
    }

    std::string rtrim(const std::string& x)
    {
        return alex::str::replace(x, "\\s+$", "");
    }

    void rtrimInPlace(std::string& x)
    {
        alex::str::replaceInPlace(x, "\\s+$", "");
    }

    std::string trim(const std::string& x)
    {
        return alex::str::rtrim(ltrim(x));
    }

    void trimInPlace(std::string& x)
    {
        alex::str::replaceInPlace(x, "\\s+$|^\\s+", "");
    }

    std::string lower(const std::string& x)
    {
        return alex::str::transform(x, tolower);
    }

    void lowerInPlace(std::string& x)
    {
        return alex::str::transformInPlace(x, tolower);
    }

    std::string upper(const std::string& x)
    {
        return alex::str::transform(x, toupper);
    }

    void upperInPlace(std::string& x)
    {
        return alex::str::transformInPlace(x, toupper);
    }

    std::string transform(const std::string& x, std::function<int(int)> f)
    {
        std::string transformed;
        std::transform(x.begin(), x.end(), std::back_inserter(transformed), f);
        return transformed;
    }

    void transformInPlace(std::string& x, std::function<int(int)> f)
    {
        std::transform(x.begin(), x.end(), x.begin(), f);
    }

    bool isLower(const std::string& x)
    {
        return std::all_of(x.begin(), x.end(), islower);
    }

    bool isUpper(const std::string& x)
    {
        return std::all_of(x.begin(), x.end(), isupper);
    }

    bool isAlpha(const std::string& x)
    {
        return std::all_of(x.begin(), x.end(), isalpha);
    }

    bool isAlphaNum(const std::string& x)
    {
        return std::all_of(x.begin(), x.end(), isalnum);
    }

    bool isNumeric(const std::string& x)
    {
        return std::all_of(x.begin(), x.end(), isdigit);
    }

    bool isInt(const std::string& x)
    {
        static const std::regex r("[+-]?[0-9]+");
        return std::regex_match(x.begin(), x.end(), r);
    };

    bool isReal(const std::string& x)
    {
        static const std::regex r("[-+]?([0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?|inf(inity)?)");
        return std::regex_match(x.begin(), x.end(), r);
    }

    bool isPositiveInt(const std::string& x)
    {
        static const std::regex r("\\+?[0-9]+");
        return std::regex_match(x.begin(), x.end(), r);
    }
}}

#endif