/********************************************
 * Class: Str                              *
 * (short for Regular Expression String)    *
 *                                          *
 * A facade for std::string class           *
 *                                          *
 * Provides a number of convenient mehods   *
 * and hides a lot of std::string methods   *
 * that the author infrequently uses.       *
 *                                          *
 * Author: Alex Towell (queelius@gmail.com) *
 ********************************************/

#ifndef __STR_H__
#define __STR_H__

#include <string>
#include <vector>
#include <functional>
#include <iostream>

namespace alex
{
class Str
{
    friend Str operator+(const Str& a, const Str& b);
    friend bool operator==(const Str& a, const Str& b);
    friend bool operator!=(const Str& a, const Str& b);
    friend bool operator<(const Str& a, const Str& b);
    friend bool operator<=(const Str& a, const Str& b);
    friend bool operator>(const Str& a, const Str& b);
    friend bool operator>=(const Str& a, const Str& b);
    friend std::ostream& operator<<(std::ostream& outs, const Str& a);
    friend std::istream& operator>>(std::istream& ins, Str& a);

public:
    Str(const std::string& copy);
    Str();
    Str(const char copy[]);
    Str(char copy);
    Str(int copy);
    Str(unsigned copy);
    Str(double copy);
    template <class Iter> Str(Iter begin, Iter end, const Str& sep = ", ");

    bool hasMatch(const Str& rx) const;
    std::vector<Str> split(const Str& rx) const;
    bool hasSubmatch(const Str& rx) const;
    std::vector<Str> matches(const Str& rx) const;
    std::vector<std::pair<size_t, Str>> find(const Str& rx) const;
    std::vector<size_t> findPosition(const Str& rx) const;
    std::pair<size_t, Str> findFirst(const Str& rx) const;
    size_t findFirstPosition(const Str& rx) const;
    Str repeat(size_t n);
    Str replace(const Str& matchRx, const Str& replaceRx) const;
    Str padLeft(size_t width, char c = ' ');
    Str padRight(size_t width, char c = ' ');
    Str ltrim() const;
    Str rtrim() const;
    Str trim() const;
    Str lower() const;
    Str upper() const;
    Str transform(std::function<int(int)> f) const;
    Str substr(size_t begin, size_t length = 0) const;
    char& operator[](size_t index);
    size_t size() const;
    bool empty() const;
    bool isLower() const;
    bool isUpper() const;
    bool isAlpha() const;
    bool isAlphaNum() const;
    bool isNumeric() const;
    bool isInt() const;
    bool isReal() const;
    bool isPositiveInt() const;
    int toInt() const;
    double toReal() const;
    unsigned toPositiveInteger() const;
    std::string& data();
    std::string data() const;
    char at(size_t index) const;
    Str& operator+=(const Str& s);

private:
    std::string s; // facade for this
};

};

#endif