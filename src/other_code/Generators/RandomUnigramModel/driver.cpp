#include <string>
#include <random>
#include <iostream>
#include "Str.h"
#include "Entropy.h"
#include "TokenDistribution.h"
#include <boost/program_options.hpp>
namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main(int ac, char* av[])
{
    try
    {
        namespace st = alex::stochastic;

        if (ac == 1)
        {
            std::cout << "No options were specified. For help, type: " << boost::filesystem::path(av[0]).stem().string() << " --help" << std::endl;
            return EXIT_SUCCESS;
        }

        st::TokenDistribution::Params params;
        params.defaults();

        std::string outFile;
        std::string configFile;
        bool binary = true;

        po::options_description genericOptions("options");
        genericOptions.add_options()
            ("version,v", "show version")
            ("help,h", "show help")
            ("about", ("show description of " + boost::filesystem::path(av[0]).stem().string()).c_str())
            ("config,c", po::value<std::string>(&configFile), "load a configuration file")
            ;

        po::options_description configOptions("configuration");
        configOptions.add_options()
            ("unique_tokens,n", po::value<unsigned int>(&params.uniqueTokens)->required(), "total number of unique tokens")
            ("min_token_size", po::value<unsigned int>(&params.minTokenSize)->default_value(params.minTokenSize), "minimum number of characters in a token")
            ("max_token_size", po::value<unsigned int>(&params.maxTokenSize)->default_value(params.maxTokenSize), "maximum number of characters in a token")
            ("alphabet,a", po::value<std::vector<char>>(&params.alphabet)->multitoken(), "a sequence of characters; tokens are composed of them")
            ("model,m", po::value<unsigned int>(&params.type)->default_value(st::TokenDistribution::Params::ZIPF), "0 for zipf distribution, 1 for uniform distribution")
            ("outfile,o", po::value<std::string>(&outFile)->required(), "output file")
            ("binary,b", po::value<bool>(&binary)->implicit_value(binary), "write model as binary or as plaintext")
            ("seed,s", po::value<unsigned int>(&params.seed), "random engine seed")
            ;

        po::options_description cmdlineOptions;
        cmdlineOptions.add(genericOptions).add(configOptions);

        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, cmdlineOptions), vm);
        
        if (vm.count("help"))
        {
            std::cout << cmdlineOptions << std::endl;
            return EXIT_SUCCESS;
        }
        else if (vm.count("version"))
        {
            std::cout << "make_random_unigram_model 1.0" << std::endl;
            return EXIT_SUCCESS;
        }
        else if (vm.count("about"))
        {
            std::cout << "about: construct a random token unigram model." << std::endl;
            std::cout << std::endl;
            std::cout << "       this program should be used in conjunction with other programs" << std::endl;
            std::cout << "       that will process this program's unigram distribution." << std::endl;
            return EXIT_SUCCESS;
        }
        
        if (!configFile.empty())
        {
            std::ifstream infile(configFile.c_str());
            if (!infile.is_open())
                throw std::exception(("Cannot Open Config File \"" + configFile + "\"").c_str());
            else
                po::store(po::parse_config_file(infile, configOptions), vm);
        }
        notify(vm);

        auto model = st::TokenDistribution::makeRandom(params);

        if (binary)
        {
            std::cout << "Writing binary file: " << outFile << std::endl;
            model.writeBinary(std::ofstream(outFile, std::ios::binary));
        }
        else
        {
            std::cout << "Writing non-binary file: " << outFile << std::endl;
            model.write(std::ofstream(outFile));
        }
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