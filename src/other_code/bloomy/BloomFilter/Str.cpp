#include <regex>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <sstream>
#include "Str.h"
using namespace alex;

Str::Str(const std::string& copy) : s(copy) {}

Str::Str() {}

Str::Str(const char copy[]) : s(copy) {}

Str::Str(char copy) : s(std::string() + copy) {}

Str::Str(int copy)
{
    std::stringstream ss;
    ss << copy;
    s = ss.str();
}

Str::Str(unsigned copy)
{
    std::stringstream ss;
    ss << copy;
    s = ss.str();
}

Str::Str(double copy)
{
    std::stringstream ss;
    ss << copy;
    s = ss.str();
}

bool Str::hasMatch(const Str& rx) const
{
    const std::regex r(rx.s);
    return std::regex_match(s.begin(), s.end(), r);
}

std::vector<Str> Str::split(const Str& rx) const
{
    static const std::sregex_iterator theEnd;
    const std::regex r(rx.data());
    std::vector<Str> result;
    size_t last = 0;

    for (auto i = std::sregex_iterator(s.begin(), s.end(), r); i != theEnd; ++i)
    {
        result.push_back(s.substr(last, (i->position() - last)));
        last = i->position() + i->length();
    }

    if (last != s.size())
        result.push_back(s.substr(last, s.size()));

    return result;
}

std::vector<Str> Str::matches(const Str& rx) const
{
    const std::regex r(rx.data());
    const std::regex_iterator<std::string::iterator> end;
    const std::regex_iterator<std::string::iterator> begin(data().begin(),
            data().end(), r);
    std::vector<Str> results;

    for (auto i = begin; i != end; ++i)
        results.push_back(Str((*i)[0]));

    return results;
}

bool Str::hasSubmatch(const Str& rx) const
{
    const std::regex r(rx.data());
    return std::regex_search(s.begin(), s.end(), r);
}

std::vector<std::pair<size_t, Str>> Str::find(
        const Str& rx) const
{
    static const std::sregex_iterator theEnd;
    const std::regex r(rx.s);
    std::vector<std::pair<size_t, Str>> results;

    for (auto i = std::sregex_iterator(s.begin(), s.end(), r); i != theEnd; ++i)
        results.push_back(std::pair<size_t, Str>(i->position(), Str(i->str())));

    return results;
}

std::vector<size_t> Str::findPosition(const Str& rx) const
{
    static const std::sregex_iterator theEnd;
    const std::regex r(rx.data());
    std::vector<size_t> results;

    for (auto i = std::sregex_iterator(s.begin(), s.end(), r); i != theEnd; ++i)
        results.push_back(i->position());

    return results;
}

std::pair<size_t, Str> Str::findFirst(const Str& rx) const
{
    static const std::sregex_iterator end;
    const std::regex r(rx.data());
    const auto i = std::sregex_iterator(s.begin(), s.end(), r);

    if (i == end)
        throw std::exception("No Match");

    return std::pair<size_t, Str>(i->position(), Str(i->str()));
}

size_t Str::findFirstPosition(const Str& rx) const
{
    static const std::sregex_iterator theEnd;
    const std::regex r(rx.s);
    const auto i = std::sregex_iterator(s.begin(), s.end(), r);

    if (i == theEnd)
        throw std::exception("No Match");

    return i->position();
}

Str Str::repeat(size_t n)
{
    Str result;

    for (size_t i = 0; i < n; ++i)
        result.s.append(s);

    return result;
}

Str Str::replace(const Str& matchRx,
                             const Str& replaceRx) const
{
    const std::regex r(matchRx.data());
    Str result;
    std::regex_replace(std::back_inserter(result.s), s.begin(), s.end(), r,
                       replaceRx.s);
    return result;
}

Str Str::padLeft(size_t width, char c)
{
    std::stringstream ss;
    ss << std::setfill(c) << std::setw(width) << std::left << s;
    return Str(ss.str());
}

Str Str::padRight(size_t width, char c)
{
    std::stringstream ss;
    ss << std::setfill(c) << std::setw(width) << std::right << s;
    return Str(ss.str());
}

Str Str::ltrim() const
{
    return replace("^\\s+", "");
}

Str Str::rtrim() const
{
    return replace("\\s+$", "");
}

Str Str::trim() const
{
    return ltrim().rtrim();
}

Str Str::lower() const
{
    return transform(tolower);
}

Str Str::upper() const
{
    return transform(toupper);
}

Str Str::transform(std::function<int(int)> f) const
{
    Str transformed;
    std::transform(s.begin(), s.end(), std::back_inserter(transformed.s), f);
    return transformed;
}

Str Str::substr(size_t begin, size_t length) const
{
    if (length == 0)
        length = s.size() - begin;

    return s.substr(begin, length);
}

char Str::at(size_t index) const
{
    return s.at(index);
}

char& Str::operator[](size_t index)
{
    return s[index];
}

size_t Str::size() const
{
    return s.size();
}

bool Str::empty() const
{
    return s.empty();
}

bool Str::isLower() const
{
    return std::all_of(s.begin(), s.end(), islower);
}

bool Str::isUpper() const
{
    return std::all_of(s.begin(), s.end(), isupper);
}

bool Str::isAlpha() const
{
    return std::all_of(s.begin(), s.end(), isalpha);
}

bool Str::isAlphaNum() const
{
    return std::all_of(s.begin(), s.end(), isalnum);
}

bool Str::isNumeric() const
{
    return std::all_of(s.begin(), s.end(), isdigit);
}

bool Str::isInt() const
{
    static const std::regex r("[+-]?[0-9]+");
    return std::regex_match(s.begin(), s.end(), r);
};

bool Str::isReal() const
{
    static const std::regex
        r("[-+]?([0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?|inf(inity)?)");
    return std::regex_match(s.begin(), s.end(), r);
}

bool Str::isPositiveInt() const
{
    static const std::regex r("\\+?[0-9]+");
    return std::regex_match(s.begin(), s.end(), r);
}

int Str::toInt() const
{
    if (!isInt())
        throw std::exception("Not An Integer");

    int result;
    std::stringstream ss(s);
    ss >> result;
    return result;
}

double Str::toReal() const
{
    if (!isReal())
        throw std::exception("Not A Real Number");

    double result;
    std::stringstream ss(s);
    ss >> result;
    return result;
}

unsigned Str::toPositiveInteger() const
{
    if (!isPositiveInt())
        throw std::exception("Not A Positive Integer");

    unsigned result;
    std::stringstream ss(s);
    ss >> result;
    return result;
}

std::string& Str::data()
{
    return s;
}

std::string Str::data() const
{
    return s;
}

Str& Str::operator+=(const Str& s)
{
    this->s.append(s.s);
    return *this;
}

Str alex::operator+(const Str& a, const Str& b)
{
    return Str(a.s + b.s);
}

bool alex::operator==(const Str& a, const Str& b)
{
    return a.s == b.s;
}

bool alex::operator!=(const Str& a, const Str& b)
{
    return a.s != b.s;
}

bool alex::operator<(const Str& a, const Str& b)
{
    return a.s < b.s;
}

bool alex::operator<=(const Str& a, const Str& b)
{
    return a.s <= b.s;
}

bool alex::operator>(const Str& a, const Str& b)
{
    return a.s > b.s;
}

bool alex::operator>=(const Str& a, const Str& b)
{
    return a.s >= b.s;
}

std::ostream& alex::operator<<(std::ostream& outs, const Str& a)
{
    outs << a.s;
    return outs;
}

std::istream& alex::operator>>(std::istream& ins, Str& a)
{
    ins >> a.s;
    return ins;
}
