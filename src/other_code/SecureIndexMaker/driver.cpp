#include "stdafx.h"
#include "TestCases.h"
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

        alex::testing::BatchBuildPsibTest("corpus", "", 5);
        return 0;

        po::options_description desc("Options");
        desc.add_options()
            ("help", "help message")
            ("query", po::value<vector<string>>()->multitoken(), "search query to submit against the secure index files")
            ("config", po::value<string>(), "load a config file")
            ("loadsi", po::value<vector<string>>()->multitoken(), "load secure indexes into db")
            ("obfuscations", po::value<int>(), "number of obfuscations per query")
            ("bm25", "use bm25 term weighting measure")
            ("mindist", "use minimum distance pair proximity measure")
            ("rank", "uses a weighted average of bm25 and mindist")
            ("secrets", po::value<vector<string>>(), "secrets to construct hidden terms with--more than one for query obfucation")
            ("makesi", po::value<vector<string>>()->multitoken(), "make secure indexes from specified directory or files")
            ("typesi", po::value<string>(), "type of secure index {psim, psib, psimin, bisb, bsif, psif}")
            ("verbose", po::value<int>(), "verbose mode")
            ("blocksize", po::value<int>(), "only useful for block-based secure indexes; specifies the words per block")
            ("maxblocks", po::value<int>(), "only useful for block-based secure indexes; specifies the maximum number of blocks per doc")
            ("fp_rate", po::value<double>(), "specifies the false positive rate")
            ("ignore_words", po::value<string>(), "specifies ignore word / stop word file")
            ("stemming", "specifies stemming")
            ("algo", po::value<int>(), "specifies perfect hash algorithm used for when making secure indexes")
            ("noise_percent", po::value<double>(), "specifies amount of noise (fake junk terms) to add to secure index to obfuscate it more")
            ("maxfreq", po::value<int>(), "specified max freq for Psif and other like secure indexes")
            ("maxminpair", po::value<int>(), "maximum minimum pairwise distance to record")
            ("post_radius", po::value<int>(), "psi post radius")
            ("batch_build_test", "perform batch build test")
            ("time", "time operations")
            ("bm25_test", "run a pre-made test")
            ("min_pairwise_dist_test", "run a pre-made test")
            ("min_pairwise_dist_score_test", "run a pre-made test");

        alex::index::SecureIndexDb db;
        po::variables_map vm;        
        po::store(po::parse_command_line(ac, av, desc), vm);
        po::notify(vm);    
        alex::index::HiddenQueryBuilder::BuilderParams hqParams;
        hqParams.defaults();        
        double fpRate = alex::globals::default_fp_rate;
        int verbose = alex::globals::default_verbose;
        int maxblock = alex::globals::default_max_blocks;
        int postRadius = alex::globals::default_post_range;
        int blocksize = alex::globals::default_words_per_block;
        bool stemming = alex::globals::default_stemming;
        int maxminpair = alex::globals::max_min_pair_dist;

        if (vm.count("help"))
        {
            cout << desc << "\n";
            return 0;
        }

        string query;
        if (vm.count("config"))
        {
            ifstream configFile(vm["config"].as<string>());
            if (configFile.fail())
            {
                cout << "Error loading config file: " << vm["config"].as<string>() << endl;
            }
            else
            {
                po::store(po::parse_config_file(configFile, desc), vm);
            }
        }
        if (vm.count("stemming") != 0)
        {
            stemming = true;
        }
        if (vm.count("verbose") != 0)
        {
            verbose = 1;
        }
        if (vm.count("fp_rate") != 0)
        {
            fpRate = vm["fp_rate"].as<double>();
        }
        if (vm.count("blocksize") != 0)
        {
            blocksize = vm["blocksize"].as<int>();
        }
        if (vm.count("maxfreq") != 0)
        {
            blocksize = vm["maxfreq"].as<int>();
        }
        if (vm.count("maxminpair") != 0)
        {
            maxminpair = vm["maxminpair"].as<int>();
        }
        if (vm.count("post_radius") != 0)
        {
            postRadius = vm["post_radius"].as<int>();
        }
        if (vm.count("obfuscations") != 0)
        {
            cout << "obfucations: " << vm["obfuscations"].as<int>() << endl;
            hqParams.obfuscations = vm["obfuscations"].as<int>();
        }
        if (vm.count("makesi") != 0)
        {
            if (vm.count("typesi") == 0)
            {
                cout << "Must specify a secure index type" << endl;
            }
            
            alex::index::PsiBlockBuilder::BuilderParams params;
            alex::index::IndexBatchBuilder<alex::index::PsiBlockBuilder> builder(params);
            builder.build("outdir", "corpus");
        }
        if (vm.count("batch_build_test"))
        {
            alex::testing::BatchBuildTest("batch_build_test", "batch_build_test", 1);
        }
        if (vm.count("loadsi") != 0)
        {
            auto x = vm["loadsi"].as<vector<string>>();
            for (auto i : x)
            {
                if (fs::is_directory(i))
                {
                    db.read(alex::io::filesystem::getFiles(i, ".*", numeric_limits<size_t>::max()));
                }
                else
                {
                    db.read(i);
                }
            }
        }
        if (vm.count("query") != 0)
        {
            alex::index::HiddenQueryBuilder hq(hqParams);
            auto query = vm["query"].as<vector<string>>();
            for (auto i : query)
            {
                cout << "submitting query: " << i << endl;
                auto hquery = hq.make(i);
                cout << "(hidden query: " << hquery.toJson() << endl;

                if (vm.count("bm25_test") != 0)
                {
                    alex::testing::bm25Test("tom huck", "corpus", "bm25_test", "bm25_test");
                }
                else if (vm.count("min_pairwise_dist_score_test") != 0)
                {
                    alex::testing::MinPairwiseDistScoreTest("tom huck", "corpus", "min_pw_test");
                }
                else if (vm.count("min_pairwise_dist_test") != 0)
                {
                    alex::testing::MinPairwiseDistTest("tom huck", "corpus", "min_pw_dst_test");
                }
                else
                {
                    db.bm25(hquery);
                }
            }
        }
    }
    catch (const exception& e)
    {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    catch (...)
    {
        cerr << "Exception of unknown type!\n";
    }

    return 0;
}
