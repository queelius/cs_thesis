#ifndef __INVERTED_INDEX_BUILDER_H__
#define __INVERTED_INDEX_BUILDER_H__

#include "InvertedIndex.h"
#include "Globals.h"

namespace alex { namespace index
{
    class InvertedIndexBuilder
    {
    public:
        struct BuilderParams
        {
            std::unordered_set<std::string> ignoreWords;
            int verbose;
            bool stemming;

            BuilderParams()
            {
                defaults();
            };

            void defaults()
            {
                stemming = alex::globals::default_stemming;
                verbose = alex::globals::default_verbose;
                ignoreWords = alex::globals::default_stop_words;
            };
        };

        InvertedIndexBuilder(const BuilderParams& params) :
            _params(params) {};

        Index build()
        {
            return _idx;
        };

        void makeTerms()
        {
            return;
        };

        void parse(const boost::filesystem::path& file)
        {
            if (!boost::filesystem::is_regular_file(file))
            {
                throw std::exception(("Not A File: \"" + file.string() + "\"").c_str());
            }
            parse(file.string(), std::ifstream(file.string()));
        };

        void parse(const std::string& ref, std::istream& file)
        {
            _idx = Index(new InvertedIndex);
            _idx->_ref = ref;
            std::string word;
            char ch;

            unsigned int position = 0;
            _idx->_termCount = 0;

            while(file.good())
            {
                ch = file.get();
                if(isalnum(ch))
                {
                    word += tolower(ch);
                }
                else if (ch == '\'')
                {
                    // skip apostrophe's, but don't let them end the current word
                }
                else
                {
                    if (!word.empty())
                    {
                        if (_params.ignoreWords.count(word) == 0)
                        {
                            if(_params.stemming)
                                porter2stemmer::stem(word);

                            _idx->_words[word].push_back(std::make_pair(position, _idx->_termCount));
                            ++_idx->_termCount;
                        }
                        
                        ++position;
                        word = "";
                    }
                }
            }
        };

        void printTerms(std::ostream& outs = std::cout)
        {
            throw std::exception("Not Implemented");
        };

        boost::filesystem::path getPreferredFileExtension() const { return InvertedIndex::HEADER; };

        void printWords(std::ostream& outs = std::cout)
        {
            _idx->print(outs);
        };

    private:
        Index _idx;
        BuilderParams _params;
    };
}}

#endif