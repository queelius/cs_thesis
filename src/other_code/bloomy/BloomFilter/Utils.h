#include <vector>
#include "Str.h"

namespace alex
{

const std::vector<Str> DEFAULT_STOP_WORDS = {
    "a", "able" "about", "across", "after", "all", "almost", "also", "am",
    "among", "an", "and", "any", "are", "as", "at", "be", "because",
    "been", "but", "by", "can", "cannot", "could", "dear", "did", "do",
    "does", "either", "else", "ever", "every", "for", "from", "get",
    "got", "had", "has", "have", "he", "her", "hers", "him", "his", "how",
    "however", "i", "if", "in", "into", "is", "it", "its", "just", "least",
    "let", "like", "likely", "may", "me", "might", "most", "must", "my",
    "neither", "no", "nor", "not", "of", "off", "often", "on", "only",
    "or", "other", "our", "own", "rather", "said", "say", "says", "she",
    "should", "since", "so", "some", "than", "that", "the", "their",
    "them", "then", "there", "these", "they", "this", "tis", "to", "too",
    "twas", "us", "wants", "was", "we", "were", "what", "when", "where",
    "which", "while", "who", "whom", "why", "will", "with", "would", "yet",
    "you", "your" };

const std::string WORD_BOUNDARY = "\\b[a-zA-Z0-9]+\\b";

namespace utils
{

size_t retrievePrime(size_t n);

const std::vector<size_t>& generatePrimes(size_t upto);

std::vector<Str> stemmer(const std::vector<Str>& words);

std::vector<Str> words(Str message);

std::vector<Str> lowerCase(const std::vector<Str>& words);

std::vector<Str> ngrams(const std::vector<Str>& words, size_t minGram,
                        size_t maxGram, bool lexographic);

std::vector<Str> removeStopWords(const std::vector<Str>& words,
                                 const std::vector<Str>& stopWords);

std::vector<std::vector<Str>> terms(Str message);

};
};