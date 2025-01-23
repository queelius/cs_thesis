#include "stdafx.h"
#include "TestCases.h"
#include <unordered_set>
#include <boost/program_options.hpp>
#include <iostream>
#include <iterator>
using namespace std;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main(int ac, char* av[])
{
    try
    {
        alex::globals::initializeDefaults();
         
        std::string indir;
        std::string outdir;
        std::string label;
        std::string pattern = ".*";
        std::string stopWordsFile;
        std::string querySetFile;
        size_t stopWordsFileLimit = std::numeric_limits<size_t>::max();
        size_t depth = std::numeric_limits<int>::max();
        bool verbose = alex::globals::default_verbose;
        bool stemming = alex::globals::default_stemming;
        double bm25_k1 = alex::globals::bm25_k1;
        double bm25_b = alex::globals::bm25_b;
        std::vector<std::string> secrets = {"secret"};
        std::string configFile;
        unsigned int topK = 10;
        unsigned int obfuscations = alex::globals::default_obfuscations;

        po::options_description desc("options");
        desc.add_options()
            ("help", "help message")
            ("config", po::value<string>(&configFile), "load a config file")
            ("secrets", po::value<vector<string>>(&secrets), "secrets to construct hidden terms with for query obfuscation")
            ("recursive_depth", po::value<size_t>(&depth)->default_value(std::numeric_limits<size_t>::max()), "depth of indir recursion")
            ("verbose", po::value<bool>(&verbose)->default_value(verbose), "verbose mode")
            ("pattern",  po::value<string>(&pattern)->default_value(pattern), "file pattern to ")
            ("label",  po::value<string>(&label), "label of the construction experiment")
            //("outdir",  po::value<string>(&outdir)->required(), "base output dir for secure indexes, i.e., \"outdir\\psib\"")
            ("indir",  po::value<string>(&indir)->required(), "directories/files to index")
            ("top_k",  po::value<unsigned int>(&topK)->default_value(topK), "number of docs to include in map rank")
            ("obfuscations",  po::value<unsigned int>(&obfuscations)->default_value(obfuscations), "number of noise terms/query")
            ("stop_words", po::value<string>(&stopWordsFile), "stop words file")
            ("stop_words_limit", po::value<size_t>(&stopWordsFileLimit)->default_value(std::numeric_limits<size_t>::max()), "stop words file limit")
            ("bm_25_k1", po::value<double>(&bm25_k1)->default_value(bm25_k1))
            ("bm_25_b", po::value<double>(&bm25_b)->default_value(bm25_b))
            ("query_set", po::value<string>(&querySetFile)->required(), "query set")
            ;
            // iidx
            // nothing to set


        po::variables_map vm;        
        po::store(po::parse_command_line(ac, av, desc), vm);

        if (vm.count("help"))
        {
            cout << desc << "\n";
            return 0;
        }

        if (!stopWordsFile.empty())
        {
            alex::globals::loadStopWords(stopWordsFile, stopWordsFileLimit);
        }

        if (vm.count("config"))
        {
            std::ifstream cfgFile(configFile);
            if (cfgFile.fail())
            {
                cout << "Error loading config file: " << configFile << endl;
            }
            else
            {
                po::store(po::parse_config_file(cfgFile, desc), vm);
            }
        }

        po::notify(vm);

        alex::index::QueryBuilder::BuilderParams qParams;
        qParams.stemming = stemming;
        qParams.ignoreWords = alex::globals::default_stop_words;
        alex::index::QueryBuilder qb(qParams);

        alex::index::HiddenQueryBuilder::BuilderParams hqParams;
        hqParams.defaults();
        hqParams.obfuscations = obfuscations;
        hqParams.qb = qb;
        hqParams.secrets = secrets;

        alex::index::InvertedIndexDb::Params iidxParams;
        iidxParams.bm25_k1 = bm25_k1;
        iidxParams.bm25_b = bm25_b;

        alex::index::SecureIndexDb::Params sidxParams;
        sidxParams.bm25_k1 = bm25_k1;
        sidxParams.bm25_b = bm25_b;

        // load from querySetFile
        std::ifstream querySetStream(querySetFile);
        if (!querySetStream.is_open())
            throw std::exception(("Cannot Open Query Set File \"" + querySetFile + "\"").c_str());

        std::vector<std::string> querySet;
        std::string querySetLabel;
        std::getline(querySetStream, querySetLabel);
        while (querySetStream.good())
        {
            std::string queryLine;
            std::getline(querySetStream, queryLine);
            queryLine = alex::str::trim(queryLine);

            if (!queryLine.empty())
            {
                if (queryLine[0] == ';')
                    continue;
                querySet.push_back(queryLine);
            }
        }

        std::vector<std::string> types = {"psib", "psif", "psip", "psim", "bsib"};
        auto results1 = alex::testing::minPairwiseDistanceScoreMAP(querySet, hqParams, qParams, iidxParams, sidxParams, indir, outdir, topK, types);
        std::cout << "minPariseWiseDistance:" << std::endl;
        for (auto result : results1)
        {
            result.second.print();
        }
        std::cout << "bm25:" << std::endl;
        auto results2 = alex::testing::bm25_MAP(querySet, hqParams, qParams, iidxParams, sidxParams, indir, outdir, topK, types);
        for (auto result : results2)
        {
            result.second.print();
        }
        std::cout << "both:" << std::endl;
        auto results3 = alex::testing::both_MAP(querySet, hqParams, qParams, iidxParams, sidxParams, indir, outdir, topK, types);
        for (auto result : results3)
        {
            result.second.print();
        }
    }
    catch (const exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const char* e)
    {
        std::cerr << "Error: " << e << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Error: Unknown cause" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
