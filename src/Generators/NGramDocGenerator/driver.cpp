#include <string>
#include <random>
#include <iostream>
#include "Str.h"
#include "Entropy.h"
#include "Corpus.h"
#include <boost/program_options.hpp>
namespace po = boost::program_options;

int main(int ac, char* av[])
{
    po::variables_map vm;

    try
    {
        std::string outputDir, modelFile, configFile;
        unsigned int seed;
        size_t minTokens, maxTokens, numDocs, nGramSize = 3;
        std::vector<float> w;

        po::options_description genericOptions("options");
        genericOptions.add_options()
            ("version,v", "show version")
            ("help,h", "show help")
            ("about,a", ("show description of " + boost::filesystem::path(av[0]).stem().string()).c_str())
            ("config,c", po::value<std::string>(&configFile), "load a configuration file")
            ;

        po::options_description configOptions("configuration");
        configOptions.add_options()
            ("min_tokens", po::value<size_t>(&minTokens), "minimum tokens/doc")
            ("max_tokens,t", po::value<size_t>(&maxTokens)->required(), "maximum tokens/doc")
            ("model,m", po::value<std::string>(&modelFile)->required(), "ngram model file")
            ("ngram_size,z", po::value<unsigned int>(&nGramSize)->default_value(3), "size of ngram model")
            ("output_dir,o", po::value<std::string>(&outputDir)->default_value("."), "output directory")
            ("num_docs,n", po::value<size_t>(&numDocs)->required(), "quantity of docs to make")
            ("seed,s", po::value<unsigned int>(&seed)->default_value(0), "value to seed random generator")
            ;

        po::options_description cmdlineOptions;
        cmdlineOptions.add(genericOptions).add(configOptions);
        po::store(po::parse_command_line(ac, av, cmdlineOptions), vm);
        
        if (vm.count("help"))
        {
            std::cout << cmdlineOptions << std::endl;
            return EXIT_SUCCESS;
        }
        if (vm.count("version"))
        {
            std::cout << "make_ngram_doc 1.0" << std::endl;
            return EXIT_SUCCESS;
        }
        if (vm.count("about"))
        {
            std::cout << "about: construct artificial documents (corpus)" << std::endl;
            std::cout << "       from a specified ngram model." << std::endl;
            return EXIT_SUCCESS;
        }

        if (!configFile.empty())
        {
            std::ifstream infile(configFile.c_str());
            if (!infile.is_open())
            {
                throw std::exception(("Cannot Open Config File \"" + configFile + "\"").c_str());
                return EXIT_FAILURE;
            }
            else
                po::store(po::parse_config_file(infile, configOptions), vm);
        }
        notify(vm);

        alex::ngrams::NGramModel model;
        try
        {
            model.readBinary(std::ifstream(modelFile, std::ios::binary));
        }
        catch (const std::exception&)
        {
            model.read(std::ifstream(modelFile));
        }

        if (minTokens == 0 || minTokens > maxTokens)
            minTokens = maxTokens;

        alex::stochastic::Entropy<> entropy(seed);
        alex::corpus::makeNGramCorpus(numDocs, minTokens, maxTokens, nGramSize, outputDir, model, entropy);
    }

    catch (const std::exception& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const char* e)
    {
        std::cout << "Error: " << e << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cout << "Error: Unknown cause." << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}