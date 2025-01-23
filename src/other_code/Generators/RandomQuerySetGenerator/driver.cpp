#include <string>
#include <random>
#include <iostream>
#include "Entropy.h"
#include <boost/program_options.hpp>
#include <vector>
#include "Query.h"
#include "QuerySetBuilder.h"
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid.hpp>
#include <sstream>
namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main(int ac, char* av[])
{
    try
    {
        const std::string reference = fs::path(av[0]).stem().string();

        if (ac == 1)
        {
            std::cout << "No options were specified. For help, type: \"" << reference << " -h\"" << std::endl;
            return EXIT_SUCCESS;
        }

        std::string outFile, configFile, label;
        unsigned int seed = 0;
        alex::corpus::RandomQuerySetBuilder::Params params;

        po::options_description genericOptions("options");
        genericOptions.add_options()
            ("version,v", "show version")
            ("help,h", "show help")
            ("about,a", ("show description of " + boost::filesystem::path(av[0]).stem().string()).c_str())
            ("config,c", po::value<std::string>(&configFile), "load a configuration file")
            ;

        po::options_description configOptions("configuration");
        configOptions.add_options()
            ("outfile,o", po::value<std::string>(&outFile), "output file")
            ("num_queries,n", po::value<unsigned int>(&params.numQueries)->required(), "number of queries in the query set to generate")
            ("min_terms,i", po::value<unsigned int>(&params.minTerms)->default_value(params.minTerms), "minimum number of terms per query")
            ("max_terms,x", po::value<unsigned int>(&params.maxTerms)->default_value(params.maxTerms), "maximum number of terms per query")
            ("min_tokens,m", po::value<unsigned int>(&params.minTokens)->default_value(params.minTokens), "minimum number of tokens (words) per term")
            ("max_tokens,t", po::value<unsigned int>(&params.maxTokens)->default_value(params.maxTokens), "maximum number of tokens (words) per term")
            ("min_token_size", po::value<unsigned int>(&params.minTokenSize)->default_value(params.minTokenSize), "minimum number of characters per token")
            ("max_token_size", po::value<unsigned int>(&params.maxTokenSize)->default_value(params.maxTokenSize), "maximum number of characters per token")
            ("seed,s", po::value<unsigned int>(&seed)->default_value(seed), "random engine seed")
            ("label,l", po::value<std::string>(&label)->default_value(""), "label for query set; if none then random uuid")
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
            std::cout << reference << " 1.0" << std::endl;
            return EXIT_SUCCESS;
        }
        else if (vm.count("about"))
        {
            std::cout << "about: construct a query set to facilitate testing." << std::endl;
            std::cout << std::endl;
            std::cout << "       this program should be used in conjunction with other programs" << std::endl;
            std::cout << "       that will process this program's query set. notably, infuse the" << std::endl;
            std::cout << "       set of queries into an existing corpus or input the set of queries" << std::endl;
            std::cout << "       into a secure index tester." << std::endl;
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

        alex::corpus::RandomQuerySetBuilder qsb(params);
        alex::stochastic::Entropy<> entropy(seed);
        auto querySet = qsb.make(entropy);

        if (vm.count("outfile") != 0)
        {
            std::ofstream outs = std::ofstream(outFile);
            if (!outs.is_open())
                throw std::exception(("Cannot Open Output File \"" + outFile + "\"").c_str());

            std::cout << "\t* Writing QuerySet to file: " << outFile << std::endl;

            if (label.empty())
            {
                std::stringstream ss;
                ss << boost::uuids::random_generator()();
                label = ss.str();
            }

            outs << label << std::endl;
            for (const auto& query : querySet)
                outs << query.toString() << std::endl;
        }
        else
        {
            if (label.empty())
            {
                std::stringstream ss;
                ss << boost::uuids::random_generator()();
                label = ss.str();
            }

            std::cout << label << std::endl;
            for (const auto& query : querySet)
                std::cout << query.toString() << std::endl;
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