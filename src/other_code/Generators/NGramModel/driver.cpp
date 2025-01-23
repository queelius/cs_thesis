#include <string>
#include <iostream>
#include <boost/program_options.hpp>
#include <vector>
#include "NGramModel.h"
namespace po = boost::program_options;
namespace fs = boost::filesystem;


int main(int ac, char* av[])
{
    try
    {
        if (ac == 1)
        {
            std::cout << "No options were specified. For help, type: " << boost::filesystem::path(av[0]).stem().string() << " --help" << std::endl;
            return EXIT_SUCCESS;
        }

        std::vector<boost::filesystem::path> inputPaths;
        std::string outFile, configFile, pattern = ".*";
        unsigned int recursion = std::numeric_limits<unsigned int>::max();
        bool binary = true;
        bool ignorePunct = true;
        unsigned int nGramSize = 3;

        po::options_description genericOptions("options");
        genericOptions.add_options()
            ("version,v", "show version")
            ("help,h", "show help")
            ("about,a", ("show description of " + boost::filesystem::path(av[0]).stem().string()).c_str())
            ("config,c", po::value<std::string>(&configFile), "load a configuration file")
            ;

        po::options_description configOptions("configuration");
        configOptions.add_options()
            ("input_paths,i", po::value<std::vector<boost::filesystem::path>>(&inputPaths)->multitoken()->required(), "input paths/files that contain documents to model")
            ("outfile,o", po::value<std::string>(&outFile)->required(), "output file")
            ("binary,b", po::value<bool>(&binary)->implicit_value(binary), "write model as binary or as plaintext")
            ("pattern,p", po::value<std::string>(&pattern)->default_value(pattern), "regular expression filename pattern")
            ("recursion,r", po::value<unsigned int>(&recursion)->default_value(recursion), "depth of recursion")
            ("ngram_size,n", po::value<unsigned int>(&nGramSize)->default_value(nGramSize), "size of ngram")
            ("ingore_punctuation,g", po::value<bool>(&ignorePunct)->default_value(ignorePunct), "ignore punctuation, commas, etc")
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
            std::cout << "make_ngram_model 1.0" << std::endl;
            return EXIT_SUCCESS;
        }
        else if (vm.count("about"))
        {
            std::cout << "about: construct an ngram model of a collection of documents." << std::endl;
            std::cout << std::endl;
            std::cout << "       this program should be used in conjunction with other programs" << std::endl;
            std::cout << "       that will process this program's ngram distribution." << std::endl;
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

        if (nGramSize < 2)
            throw std::exception("Ngram Size Must Be Larger Than 1");

        notify(vm);

        alex::ngrams::NGramModel model;
        parse(model, inputPaths, ".*", nGramSize, recursion, ignorePunct);
        model.makeCdf();

        if (binary)
        {
            std::cout << "\t* Writing binary file: " << outFile << std::endl;
            model.writeBinary(std::ofstream(outFile, std::ios::binary));
        }
        else
        {
            std::cout << "\t* Writing non-binary file: " << outFile << std::endl;
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
