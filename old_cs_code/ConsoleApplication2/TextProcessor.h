#ifndef __TEXT_PROCESSOR_H__
#define __TEXT_PROCESSOR_H__

#include <utility>
#include <set>
#include <string>
#include <unordered_set>
#include "Porter2Stemmer.h"
#include "picojson.h"
#include <utility>
#include <algorithm>
#include <cctype>
#include <map>

namespace alex { namespace text
{
    class TextProcessor
    {
    public:
        virtual TextProcessor* operator<<(const TextProcessor*) = 0;
        virtual size_t size() const = 0;
        virtual std::string get(size_t) const = 0;
        virtual std::vector<std::string> toVector() const = 0;
    };

    class CustomProcessor: public TextProcessor
    {
    public:
        void writeStopWords(std::ostream& file) const
        {
            if (!file)
                throw std::exception("Unable To Open Output Stream");

            for (const auto& stopWord : _stopWords)
            {
                file << stopWord << std::endl;
            }
        };

        void read(std::istream& file, bool verbose = true)
        {
            if (!file)
                throw std::exception("Unable To Open Input Stream");

            picojson::value v;
            std::string err = picojson::parse(v, file);

            if (!err.empty())
                throw std::exception(err.c_str());

            if (!v.is<picojson::value::object>())
            {
                std::string stopWord;
                while (file >> stopWord)
                {
                    if (!add(stopWord) && verbose)
                        std::cerr << "[WARNING] duplicate stop word \"" << stopWord << "\"" << std::endl;
                }
            }
            else
            {
                if (v.contains("stop_words"))
                {
                    if (v.get("stop_words").is<picojson::value::array>())
                    {
                        const auto& stops = v.get("stop_words").get<picojson::value::array>();
                        for (const auto& stop : stops)
                        {
                            if (!stop.is<std::string>() && verbose)
                            {
                                std::cerr << "[WARNING] stop word entry not a string: \"" << stop.to_str() << "\"" << std::endl;
                                continue;
                            }

                            if (!add(stop.get<std::string>()) && verbose)
                                std::cerr << "[WARNING] duplicate stop word \"" << stop.get<std::string>() << "\"" << std::endl;
                        }
                    }
                    else if (v.get("stop_words").is<std::string>())
                    {
                        std::ifstream stopFile(v.get("stop_words").get<std::string>());
                        if (stopFile.fail())
                        {

                        }

                        std::string stopWord;
                        while (stopFile >> stopWord)
                        {
                            if (!add(stopWord) && verbose)
                                std::cerr << "[WARNING] duplicate stop word \"" << stopWord << "\"" << std::endl;
                        }
                    }
                }
            }
        };

        bool add(const std::string& stopWord)
        {
            return _stopWords.insert(stopWord).second;
        };

        bool remove(const std::string& stopWord)
        {
            return _stopWords.erase(stopWord) != 0;
        };

        bool isStopWord(const std::string& stopWord) const
        {
            return _stopWords.count(stopWord) != 0;
        };

    private:
        std::unordered_set<std::string> _stopWords;
    };

    class StemmerProcesor: public TextProcessor
    {      
    public:
        enum class StemmerType { PORTER2, NONE };

        void read(std::istream& file, bool verbose = true)
        {
            if (!file)
                throw std::exception("Unable To Open Input Stream");

            picojson::value v;
            std::string err = picojson::parse(v, file);

            if (!err.empty())
                throw std::exception(err.c_str());

            if (v.contains("stemmer_type") && v.get("stemmer_type").is<std::string>())
            {
                const auto& stemObj = v.get("stemmer_type");
                if (stemObj.is<std::string>())
                    setStemmerType(stemObj.get<std::string>());
                else
                {
                    std::cerr << "[WARNING] stemming option expected a StemmingType value" << std::endl;
                    std::cerr << "          defaulting to none" << std::endl;

                    setStemmerType(StemmerType::NONE);
                }
            }
        };

        void write(std::istream& file)
        {
            if (!file)
                throw std::exception("Unable To Open Input Stream");
        };

        StemmerType getStemmerType() const
        {
            return _stemmerType;
        };

        void setStemmerType(std::string type)
        {
            porter2stemmer::trim(type);
            for (auto& x : type)
                std::tolower(x);

            if (_m.count("porter2") == 0)
            {
                _m["porter2"] = StemmerType::PORTER2;
                _m["none"] = StemmerType::NONE;
            }

            if (_m.count(type) == 0)
                throw std::exception("Invalid Argument");

            _stemmerType = _m[type];
        };

        void setStemmerType(StemmerType type)
        {
            _stemmerType = type;
        };

    private:
        StemmerType _stemmerType;
        static std::map<std::string, StemmerType> _m;
    };

    class WordSequence
    {
    public:
        void parse(std::istream& file)
        {
            std::string word;
            char c;

            while (file.good())
            {
                c = file.get();

                if (std::isalnum(c))
                {
                    word += std::tolower(c);
                }
                else if (c == '\'')
                {
                    // skip apostrophe's, but don't let them end the current word
                }
                else if (!word.empty())
                {
                    picojson::object o;
                    o["word"] = word;

                    _tags.push_back(
                }
            }
        };

    private:
        picojson::array _tags;
    };
}};














        //void parse(std::istream& file)
        //{
        //    std::string word;
        //    char c;

        //    while (file.good())
        //    {
        //        c = file.get();

        //        if (std::isalnum(c))
        //        {
        //            word += std::tolower(c);
        //        }
        //        else if (c == '\'')
        //        {
        //            // skip apostrophe's, but don't let them end the current word
        //        }
        //        else
        //        {
        //            if (!word.empty())
        //            {
        //                if (!isStopWord(word))
        //                {
        //                    std::string wd = word;

        //                    if (getStemming())
        //                        alex::text::porter2stemmer::stem(wd);
        //                    _words.push_back({ wd });

        //                    unsigned int t = getWildCards();
        //                    if (word.size() - 1 > getWildCards())
        //                        t = word.size() - 1;

        //                    for (unsigned int r = 1; r <= t; ++r)
        //                    {
        //                        std::vector<bool> v(word.size());
        //                        for (size_t i = 0; i < v.size(); ++i)
        //                            v[i] = (i >= (v.size() - r));
 
        //                        wd = word;
        //                        do
        //                        {
        //                            for (size_t i = 0; i < v.size(); ++i)
        //                            {
        //                                if (v[i])
        //                                    wd[i] = '*';
        //                            }
        //                        } while (std::next_permutation(v.begin(), v.end()));
        //                        _words[_words.size() - 1].push_back(wd);
        //                    }
        //                }
        //                word = "";
        //            }
        //        }
        //    }

#endif