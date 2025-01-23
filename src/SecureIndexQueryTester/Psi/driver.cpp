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
        std::string label;
        std::string pattern = ".*";
        std::string stopWordsFile;
        std::string querySetFile;
        size_t depth = std::numeric_limits<int>::max();
        bool verbose = alex::globals::default_verbose;
        bool stemming = alex::globals::default_stemming;

        double bm25_k1 = alex::globals::bm25_k1;
        double bm25_b = alex::globals::bm25_b;
        double alpha_weighted_avg = alex::globals::alpha_weighted_avg;
        double beta_min_pair = alex::globals::beta_min_pair;
        double alpha_min_pair = alex::globals::alpha_min_pair;

		bool bm25, mindist, recallPrec;
        std::unordered_set<std::string> stopWords = alex::globals::default_stop_words;
        std::vector<std::string> secrets = alex::globals::default_secrets;
        std::string configFile;
        unsigned int trials = 1;
        unsigned int topK = 100;
        unsigned int obfuscations = alex::globals::default_obfuscations;
        std::vector<std::string> types = {"psib", "psif", "psip", "psim", "bsib"};


        po::options_description desc("options");
        desc.add_options()
            ("help,h", "help message")
			("bm25", po::value<bool>(&bm25)->default_value(true), "bm25 scoring")
			("mindist", po::value<bool>(&mindist)->default_value(true), "mindist scoring")
			("types,t", po::value<vector<string>>(&types)->multitoken(), "types: psib, psif, psip, psim, bsib")
            ("config,c", po::value<string>(&configFile), "load a config file")
            ("secrets,s", po::value<vector<string>>(&secrets), "secrets to construct hidden terms with for query obfuscation")
            ("recursive,r", po::value<size_t>(&depth)->default_value(std::numeric_limits<size_t>::max()), "depth of indir recursion")
            ("verbose,v", po::value<bool>(&verbose)->default_value(verbose), "verbose mode")
            ("pattern,p",  po::value<string>(&pattern)->default_value(pattern), "file pattern to ")
            ("label,l",  po::value<string>(&label), "label of the construction experiment")
            ("indir,i",  po::value<string>(&indir)->required(), "directories/files to index")
            ("top_k",  po::value<unsigned int>(&topK)->default_value(topK), "max number of docs to include in rankings")
            ("obfuscations",  po::value<unsigned int>(&obfuscations)->default_value(obfuscations), "number of noise terms/query")
            ("stop_words", po::value<string>(&stopWordsFile), "stop words file")
            ("stemming", po::value<bool>(&stemming)->default_value(stemming), "porter2stemmer")
            ("bm_25.k1", po::value<double>(&bm25_k1)->default_value(bm25_k1))
            ("bm_25.b", po::value<double>(&bm25_b)->default_value(bm25_b))
            ("mindist.beta", po::value<double>(&beta_min_pair)->default_value(beta_min_pair))
            ("mindist.alpha", po::value<double>(&alpha_min_pair)->default_value(alpha_min_pair))
            ("weighted_avg", po::value<double>(&alpha_weighted_avg)->default_value(alpha_weighted_avg))
            ("query_set", po::value<string>(&querySetFile)->required(), "query set")
			("recall_precision", po::value<bool>(&recallPrec)->default_value(true))
            ("trials", po::value<unsigned int>(&trials)->default_value(trials))
            ;
            // iidx
            // nothing to set

        po::variables_map vm;        
        po::store(po::parse_command_line(ac, av, desc), vm);

        if (vm.count("help"))
        {
            cout << desc << "\n";
            return EXIT_SUCCESS;;
        }

        if (!stopWordsFile.empty())
        {
            alex::globals::loadStopWords(stopWordsFile, std::numeric_limits<int>::max());
            stopWords = alex::globals::default_stop_words;
        }

        if (vm.count("config"))
        {
            ifstream cfgFile(configFile);
            if (cfgFile.fail())
                throw std::exception(("Could Not Load Config File: " + configFile).c_str());
            else
                po::store(po::parse_config_file(cfgFile, desc), vm);
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

        // grab query stats
        unsigned int minTerms = std::numeric_limits<unsigned int>::max();
        unsigned int maxTerms = 0;
        unsigned int minWords = std::numeric_limits<unsigned int>::max();
        unsigned int maxWords = 0;

        alex::index::QueryBuilder pq(qParams);
        for (auto q : querySet)
        {
            auto query = pq.make(q);
            minTerms = std::min(minTerms, query.terms.size());
            maxTerms = std::max(maxTerms, query.terms.size());

            for (auto t : query.terms)
            {
                minWords = std::min(minWords, t.size());
                maxWords = std::max(maxWords, t.size());
            }
        }

        std::cout << "min_query_terms " << minTerms << std::endl;
        std::cout << "max_query_terms " << maxTerms << std::endl;
        std::cout << "min_term_tokens " << minWords << std::endl;
        std::cout << "max_term_tokens " << maxWords << std::endl;

        std::cout << "obfuscations " << obfuscations << std::endl;
		if (mindist)
		{
			std::cout << "mindist" << std::endl;
            auto results = alex::testing::minPairwiseDistanceScoreMAP(querySet, hqParams, qParams, iidxParams, sidxParams, indir, label, topK, types, verbose, trials);
			for (auto result : results)
			{
				result.second.print();
			}
		}

		if (bm25)
		{
			std::cout << "bm25" << std::endl;
			auto results2 = alex::testing::bm25_MAP(querySet, hqParams, qParams, iidxParams, sidxParams, indir, label, topK, types, verbose, trials);
			for (auto result : results2)
			{
				result.second.print();
			}
		}

		if (recallPrec)
		{
			std::cout << "recall_precision" << std::endl;
			auto results3 = alex::testing::recallPrecision(querySet, hqParams, qParams, iidxParams, sidxParams, indir, label, types, verbose, trials);
			for (auto result : results3)
			{
				result.second.print();
			}
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
