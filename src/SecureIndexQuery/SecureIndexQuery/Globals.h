#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <iostream>
#include <unordered_set>
#include <string>
#include <iostream>
#include <fstream>
#include <unordered_set>
#include <vector>
#include "cmph\cmph.h"

namespace alex { namespace globals
{
    int default_verbose;
    std::ostream* log = &std::cout;
    const size_t digest_size = 16;
    std::unordered_set<std::string> default_stop_words;
    std::vector<std::string> default_secrets;
    bool default_stemming;
    double default_fp_rate;
    size_t default_words_per_block;
    size_t default_max_blocks;
    size_t default_post_range;
    cmph_cpp::CMPH_ALGO default_ph;
    double default_load_factor;
    double alpha_min_pair;
    double beta_min_pair;
    int max_min_pair_dist;
    int default_obfuscations;
    double alpha_weighted_avg;
    double bm25_k1;
    double bm25_b;

    void loadSecrets(const std::string& filename, int numSecrets);
    void output(const std::string& s, int verbosityLevel = 0);
    void loadStopWords(const std::string& filename, int numStopWords);

    void initializeDefaults()
    {
        alex::globals::bm25_k1 = 1.2;
        alex::globals::bm25_b = 0.75;
        alex::globals::alpha_weighted_avg = 0.25;
        alex::globals::default_obfuscations = 0;
        alex::globals::max_min_pair_dist = 100;
        alex::globals::beta_min_pair = 1;
        alex::globals::alpha_min_pair = 0.3;
        alex::globals::default_load_factor = 1;
        alex::globals::default_ph = cmph_cpp::CMPH_CHD;
        alex::globals::default_fp_rate = 0.001;
        alex::globals::log = &std::cout;
        alex::globals::default_verbose = 0;
        alex::globals::default_words_per_block = 250;
        alex::globals::default_max_blocks = 64;
        alex::globals::default_post_range = 0;

        //loadSecrets("Secrets.txt", 1);
        //loadStopWords("StopWords.txt", std::numeric_limits<int>::max());
    };

    void output(const std::string& s, int verbosityLevel)
    {
        if (alex::globals::default_verbose >= verbosityLevel)
            *alex::globals::log << s;
    };

    void loadSecrets(const std::string& filename, int numSecrets)
    {
        std::ifstream file(filename);
        if (!file.is_open())
            alex::globals::output("Failed to load secrets from \"" + filename + "\".\n");
        else
        {
            default_secrets.clear();
            std::string secret;
            int i = 0;
            while (i++ < numSecrets && file >> secret)
                default_secrets.push_back(secret);
        }
    };

    void loadStopWords(const std::string& filename, int numStopWords)
    {
        std::ifstream file(filename);
        if (!file.is_open())
            alex::globals::output("Failed to load stop words from \"" + filename + "\".\n");
        else
        {
            default_stop_words.clear();
            std::string stopWord;
            int i = 0;
            while (i++ < numStopWords && file >> stopWord)
                default_stop_words.insert(stopWord);
        }
    };
}}

#endif