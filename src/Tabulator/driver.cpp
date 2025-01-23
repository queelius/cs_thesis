#include <string>
#include <random>
#include <iostream>
#include "Entropy.h"
#include <boost/program_options.hpp>
#include <vector>
#include "Tabulator.h"
#include <sstream>
namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main(int ac, char* av[])
{
    try
    {
        if (ac == 1)
        {
            std::cout << "No options were specified. For help, type: \"" << boost::filesystem::path(av[0]).stem().string().c_str() << " -h\"" << std::endl;
            return EXIT_SUCCESS;
        }

        unsigned int minTokens, maxTokens;
        std::vector<std::string> inDirs;

        po::options_description genericOptions("options");
        genericOptions.add_options()
            ("help,h", "show help")
            ("about", ("show description of " + boost::filesystem::path(av[0]).stem().string()).c_str())
            ;

        po::options_description configOptions("configuration");
        configOptions.add_options()
            ("in_dir,i", po::value<std::vector<std::string>>(&inDirs)->multitoken()->required())
            ("min_tokens_per_doc", po::value<unsigned int>(&minTokens)->required())
            ("max_tokens_per_doc", po::value<unsigned int>(&maxTokens)->required())
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
        else if (vm.count("about"))
        {
            std::cout << "about: tabulate raw data" << std::endl;
            return EXIT_SUCCESS;
        }       
        notify(vm);

        alex::corpus::Tabulator t;
        t.tabulate(inDirs, minTokens, maxTokens);
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