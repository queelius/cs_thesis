#include <string>
#include <random>
#include <iostream>
#include "Str.h"
#include "DiscreteDistribution.h"
#include "Entropy.h"
#include "TokenDistribution.h"
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
        size_t minTokens, maxTokens, minUniqueTokens,
            maxUniqueTokens, numDocs;

        po::options_description genericOptions("options");
        genericOptions.add_options()
            ("version,v", "show version")
            ("help,h", "show help")
            ("about,a", ("show description of " + boost::filesystem::path(av[0]).stem().string()).c_str())
            ("config,c", po::value<std::string>(&configFile), "load a configuration file")
            ;

        po::options_description configOptions("configuration");
        configOptions.add_options()
            ("max_unique_tokens", po::value<size_t>(&maxUniqueTokens)->default_value(0), "maximum unique tokens/doc")
            ("min_unique_tokens", po::value<size_t>(&minUniqueTokens)->default_value(0), "minimum unique tokens/doc")
            ("min_tokens", po::value<size_t>(&minTokens), "minimum tokens/doc")
            ("max_tokens,t", po::value<size_t>(&maxTokens)->required(), "maximum tokens/doc")
            ("model,m", po::value<std::string>(&modelFile)->required(), "unigram model file")
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
            std::cout << "make_unigram_doc 1.0" << std::endl;
            return EXIT_SUCCESS;
        }
        if (vm.count("about"))
        {
            std::cout << "about: construct artificial documents (corpus)" << std::endl;
            std::cout << "       from a specified unigram model." << std::endl;
            return EXIT_SUCCESS;
        }

        if (!configFile.empty())
        {
            std::ifstream infile(configFile.c_str());
            if (!infile.is_open())
            {
                std::cout << "Error: cannot open config file \"" << configFile << "\"" << std::endl;
                return EXIT_FAILURE;
            }
            else
                po::store(po::parse_config_file(infile, configOptions), vm);
        }
        notify(vm);

        alex::stochastic::TokenDistribution dist;
        try
        {
            dist.readBinary(std::ifstream(modelFile, std::ios::binary));
        }
        catch (const std::exception&)
        {
            dist.read(std::ifstream(modelFile));
        }

        if (maxUniqueTokens == 0 || maxUniqueTokens > dist.size())
            maxUniqueTokens = dist.size();
        if (minUniqueTokens == 0 || minUniqueTokens > maxUniqueTokens)
            minUniqueTokens = maxUniqueTokens;
        if (minTokens == 0 || minTokens > maxTokens)
            minTokens = maxTokens;

        alex::stochastic::Entropy<> entropy(seed);
        alex::corpus::makeRandomUnigramCorpus(numDocs, minUniqueTokens, maxUniqueTokens,
                                 minTokens, maxTokens, outputDir, dist, entropy);
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