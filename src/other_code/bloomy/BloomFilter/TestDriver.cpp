#define DEBUG

#include <iostream>
#include <functional>
#include <vector>
#include <algorithm>
#include <set>
#include <sstream>
#include "BloomMessage.h"
#include "Str.h"
#include "AutoTune.h"
#include "HashStrGenerator.h"
#include "TextFile.h"
#include "porter2_stemmer.h"
#include <fstream>
#include "dirent.h"
#include "TreeGram.h"
#include "Entropy.h"
#include <regex>
#include <random>

void permute(const std::set<alex::Str>& xs, size_t k, alex::Str prefix,
             std::vector<alex::Str>& permutes);

double validateBloomFilter(const alex::bloom::BloomMessage& bloom,
                           const std::vector<alex::Str>& memberData,
                           const alex::Str validationData);

std::vector<alex::Str> getFiles(const alex::Str& directory,
                                const alex::Str& filesMatching = ".*", bool recursive = false,
                                const alex::Str& subdirMatching = ".*");

const size_t N_GRAM_SIZE = 3;
const double FALSE_POSITIVE_PROBABILITY = 0.005;

const std::string TRAINING_FILES[] =
{
    "corpus\\ch1.txt"
};

const std::string VALIDATION_FILES[] =
{
    "corpus\\ch2.txt"
};

const std::string MESSAGE_FILE = "corpus\\pg848.txt";

const auto& STOP_WORDS = alex::bloom::DEFAULT_STOP_WORDS;
const bool IGNORE_ORDER = true;
const bool STEMMING = true;
const size_t MIN_TRIALS = 250;
const size_t MAX_TRIALS = 500;

std::vector<alex::Str> stemmer(const std::vector<alex::Str>& words)
{
    std::vector<alex::Str> stems;
    for (alex::Str word : words)
    {
        Porter2Stemmer::stem(word.data());
        stems.push_back(word);
    }
    return stems;
}

alex::Str wordGenerator(alex::TreeGram treeGram, size_t numWords, size_t wordsPerLine = 10)
{
    treeGram.makeCdf();
    std::vector<alex::Str> slidingWindow;
    std::function<double(void)> get0_1 = alex::stats::Entropy<>::instance().get0_1;
    slidingWindow.push_back("<para>");
    slidingWindow.push_back(treeGram.inverseCdf(get0_1()));
    alex::Str word;
    alex::Str words;
    for (size_t i = 1; i < numWords; ++i)
    {
        if (slidingWindow.size() >= treeGram.maxGramSize())
        {
            slidingWindow.erase(slidingWindow.begin());
        }
        try
        {
            word = treeGram.inverseCdf(get0_1(), slidingWindow);
        }
        catch (const std::exception&)
        {
            slidingWindow.clear();
            slidingWindow.push_back("<para>");
            slidingWindow.push_back(treeGram.inverseCdf(get0_1()));
            word = treeGram.inverseCdf(get0_1(), slidingWindow);
        }

        slidingWindow.push_back(word);

        if (word == "<para>")
            words += "\n";
        else
            words += word + " ";
    }

    return words;
}

std::vector<alex::Str> tokenize(alex::Str message)
{
    try
    {
        message = message.trim().lower();
        //message = alex::Str("<para>" + message.
        //message = message.
            //replace("\\$\\d+(\\.\\d\\d)?", "<money>").
            //replace("[0-9]*\\.[0-9]+", "<dec>").
            //replace("[0-9]+", "<int>");
            //replace("(\\r\\n)+|(\\n+)", "<para>").
            //replace("\\s+", " ").
            //replace("[a-z](\\.[a-z])+", "<abbr>").
            //replace("--+", "--").
            //replace("\\.\\.\\.+", "...").
            //replace(";;+", ";").
            //replace("\\?\\?+", "?").
            //replace("!!+", "!");
    }
    catch (std::exception e)
    {
        std::cout << e.what() << std::endl;
    }
    //const std::regex r("(<[a-z]+>|[a-z]+|\\?|\\.|!|--|;|:)");
    //const std::regex r("<[a-z]+>|[a-z]+");
    const std::regex r("[a-z0-9]+");
    const std::regex_iterator<std::string::iterator> end;
    const std::regex_iterator<std::string::iterator> begin(message.data().begin(),
            message.data().end(), r);
    std::vector<alex::Str> tokens;
    for (auto i = begin; i != end; ++i)
    {
        tokens.push_back(alex::Str((*i)[0]));
    }
    return tokens;
}

alex::TreeGram makeGrams(const alex::Str& dir, const alex::Str& filePattern, size_t nGramSize, bool recursive = true, bool lexicalSort = true)
{
   alex::TreeGram grams;
    
    auto files = getFiles(dir, filePattern, recursive);

    std::vector<alex::Str> ngram(nGramSize);
    alex::TextFile text;
    std::vector<alex::Str> fileData;

    for (auto file : files)
    {
        std::cout << "\t* opening file: " << file << std::endl;
        text.open(file);
        fileData.clear();
        std::cout << "\t* tokenizing" << std::endl;
        fileData = tokenize(text.getContents().replace("'", "").trim().lower());
        fileData = alex::bloom::removeStopWords(fileData, alex::bloom::DEFAULT_STOP_WORDS);
        fileData = stemmer(fileData);
        std::cout << "\t* making treegram" << std::endl;

        for (size_t i = 0; i < fileData.size() - nGramSize; ++i)
        {
            for (size_t j = i; j < i + nGramSize; ++j)
            {
                ngram[j - i] = fileData[j];
            }
            if (lexicalSort)
                std::sort(ngram.begin(), ngram.end());

            grams.addGram(ngram);
        }
        text.close();
    }

    return grams;
}

void main()
{
    auto treeGram = makeGrams(".\\corpus", ".*", 2, true, true);
    std::cout << wordGenerator(treeGram, 2000) << std::endl;

    std::ofstream fs;
    fs.open("treegram.txt");
    treeGram.print(fs);

    exit(0);

    try
    {
        alex::TextFile trainingFile;
        if (!trainingFile.open(TRAINING_FILES[0]))
        {
            std::cerr << "Failed to open training file" << std::endl;
            exit(0);
        }
        std::vector<alex::Str> trainingData;

        trainingData = alex::bloom::words(trainingFile.getContents());
        trainingData = alex::bloom::lowerCase(trainingData);
        trainingData = alex::bloom::removeStopWords(trainingData, STOP_WORDS);
        trainingData = alex::bloom::stemmer(trainingData);
        trainingData = alex::bloom::ngrams(trainingData, 1, N_GRAM_SIZE, IGNORE_ORDER);

        alex::TextFile messageFile;
        if (!messageFile.open(MESSAGE_FILE))
        {
            std::cerr << "Failed to open message file" << std::endl;
            exit(0);
        }
        std::vector<alex::Str> memberData;
        memberData = alex::bloom::words(messageFile.getContents());
        memberData = alex::bloom::lowerCase(memberData);
        memberData = alex::bloom::removeStopWords(memberData, STOP_WORDS);
        memberData = alex::bloom::stemmer(memberData);
        memberData = alex::bloom::ngrams(memberData, 1, N_GRAM_SIZE, IGNORE_ORDER);

        for (size_t i = 0; i < memberData.size(); ++i)
        {
            std::cout << memberData[i] << std::endl;
        }
        system("pause");

        auto bloom = alex::bloom::tune(
                         memberData,
                         trainingData,
                         FALSE_POSITIVE_PROBABILITY,
                         STOP_WORDS,
                         MIN_TRIALS,
                         MAX_TRIALS);

        std::cout << "min grams: " << bloom.minGram() << std::endl;
        std::cout << "max grams: " << bloom.maxGram() << std::endl;

        alex::TextFile validationFile;
        if (!validationFile.open(VALIDATION_FILES[0]))
        {
            std::cerr << "Failed to open validation file" << std::endl;
            exit(0);
        }
        double fp = validateBloomFilter(bloom, memberData,
                                        validationFile.getContents());
        std::cout << "Proportion of false positives on validation set: " << fp <<
                  std::endl;

        alex::Str query;
        while (true)
        {
            std::getline(std::cin, query.data());
            query = query.lower().trim();
            if (query == "q" || query == "quit")
            {
                break;
            }
            std::cout << "result: " << (bloom.hasMatch(query) ? "yes" : "no") << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void permute(const std::set<alex::Str>& xs, size_t k, alex::Str prefix,
             std::vector<alex::Str>& permutes)
{
    if (k == 0)
    {
        permutes.push_back(prefix);
        return;
    }

    for (auto i = xs.begin(); i != xs.end(); ++i)
    {
        auto xsPrime = xs;
        xsPrime.erase(*i);
        permute(xsPrime, k - 1, prefix + *i, permutes);
    }
}

double validateBloomFilter(const alex::bloom::BloomMessage& bloom,
                           const std::vector<alex::Str>& memberData,
                           const alex::Str validationData)
{
    std::vector<alex::Str> validationDataSet;
    validationDataSet = alex::bloom::words(validationData);

    validationDataSet = alex::bloom::lowerCase(validationDataSet);
    validationDataSet = alex::bloom::removeStopWords(validationDataSet,
                        bloom.stopWords());

    if (bloom.stemmed())
    {
        validationDataSet = alex::bloom::stemmer(validationDataSet);
    }

    validationDataSet = alex::bloom::ngrams(validationDataSet, bloom.minGram(),
                                            bloom.maxGram(), bloom.ignoreOrder());

    std::set<alex::Str> actual(memberData.begin(), memberData.end());
    size_t failures = 0;
    for (size_t i = 0; i < validationDataSet.size(); ++i)
    {
        std::cout << validationDataSet[i] << std::endl;
        bool bloomResult = bloom.hasExactMatch(validationDataSet[i]);
        bool actualResult = actual.count(validationDataSet[i]) > 0;
        if (bloomResult != actualResult)
        {
            if (!bloomResult && actualResult)
            {
                throw std::exception("Unexpected error: False Negative");
                exit(0);
            }
            else
            {
                ++failures;
#ifdef DEBUG
                std::cout << "\tfalse positive: {" << validationDataSet[i] << "}" << std::endl;
#endif
            }
        }
    }

    return (double)failures / (double)validationDataSet.size();

}

std::vector<alex::Str> getFiles(const alex::Str& directory,
                                const alex::Str& filesMatching, bool recursive,
                                const alex::Str& subdirMatching)
{
    std::vector<alex::Str> files;
    DIR* dir;
    dirent* ent;

    if ((dir = opendir(directory.data().c_str())) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            const alex::Str name(ent->d_name);
            if (ent->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (recursive && name != "." && name != ".." && name.hasMatch(subdirMatching))
                {
                    auto more = getFiles(directory + "\\" + name, filesMatching, recursive,
                                         subdirMatching);
                    files.insert(files.end(), more.begin(), more.end());
                }
            }
            else
            {
                if (name.hasMatch(filesMatching))
                {
                    files.push_back(directory + "\\" + name);
                }
            }
        }
        closedir(dir);
        return files;
    }
    else
    {
        std::cerr << "ERROR: Could not open \"" << directory << "\"" << std::endl;
        return std::vector<alex::Str>();
    }
}
