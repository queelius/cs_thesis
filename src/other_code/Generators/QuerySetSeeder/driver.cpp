#include <string>
#include <random>
#include <iostream>
#include "Entropy.h"
#include <boost/program_options.hpp>
#include <vector>
#include "Query.h"
#include "QuerySetSeeder.h"
#include "DiscreteDistribution.h"
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

        std::string configFile, label, outDir = "seeded", pattern = ".*";
        std::string querySetFile;
        std::vector<std::string> inDirs;
        bool verbose = false;
        unsigned int seed = 0;
        unsigned int recursionDepth = std::numeric_limits<unsigned int>::max();
        alex::corpus::QuerySetSeeder::Params params;

        po::options_description genericOptions("options");
        genericOptions.add_options()
            ("version,v", "show version")
            ("help,h", "show help")
            ("about", ("show description of " + boost::filesystem::path(av[0]).stem().string()).c_str())
            ("config,c", po::value<std::string>(&configFile), "load a configuration file")
            ;

        po::options_description configOptions("configuration");
        configOptions.add_options()
            //("statfile", po::value<std::string>(&outStatFile), "output file for extra statistical data")
            ("verbose", po::value<bool>(&verbose)->default_value(verbose)->implicit_value(true), "verbose output")
            ("in_dir,i", po::value<std::vector<std::string>>(&inDirs)->multitoken()->required(), "directories that contain documents to seed")
            ("out_dir,o", po::value<std::string>(&outDir)->default_value(outDir), "directory to output seeded documents to")
            ("query_set,q", po::value<std::string>(&querySetFile)->required(), "query set filename; a query set is just a list of queries with a label on top")
            ("min_block_size,k", po::value<size_t>(&params.minBlockSize)->default_value(params.minBlockSize), "minimum block size to place tokens into (unless doc is smaller)")
            ("max_block_size,b", po::value<size_t>(&params.maxBlockSize)->default_value(params.maxBlockSize), "maximum block size to place tokens into (unless doc is smaller)")
            ("pattern,p", po::value<std::string>(&pattern)->default_value(pattern), "filename regex pattern to match (for documents)")
            ("recursion,r", po::value<unsigned int>(&recursionDepth)->default_value(recursionDepth), "how deeply to recurse into the specified directories")
            ("min_binomial,m", po::value<float>(&params.minBinomial)->default_value(params.minBinomial), "min probability a single term in a query will be seeded")
            ("max_binomial,x", po::value<float>(&params.maxBinomial)->default_value(params.maxBinomial), "max probability a single term in a query will be seeded")
            ("min_load_factor,d", po::value<float>(&params.minLoadFactor)->default_value(params.minLoadFactor), "for each included term, what is the min block load factor")
            ("max_load_factor,f", po::value<float>(&params.maxLoadFactor)->default_value(params.maxLoadFactor), "for each included term, what is the max block load factor")
            ("seed,s", po::value<unsigned int>(&seed)->default_value(seed), "random engine seed")
            ("label,l", po::value<std::string>(&label), "label; if none then random uuid")
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

        std::ifstream querySet(querySetFile);
        if (!querySet.is_open())
            throw std::exception(("Cannot Open Query Set File \"" + querySetFile + "\"").c_str());

        alex::stochastic::Entropy<> entropy(seed);
        alex::index::QueryBuilder qb;
        std::string querySetLabel;
        std::getline(querySet, querySetLabel);
        while (querySet.good())
        {
            std::string queryLine;
            std::getline(querySet, queryLine);
            auto query = qb.make(queryLine);
            if (!query.empty())
                params.queries.push_back(query);
        }
        alex::corpus::QuerySetSeeder qss(params);

        qss.seedDocuments(entropy, inDirs, outDir, pattern, recursionDepth, querySetLabel, verbose);
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