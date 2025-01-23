#include "stdafx.h"
#include <unordered_set>
#include <boost/program_options.hpp>
#include <iostream>
#include <iterator>
#include <algorithm>
using namespace std;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main(int ac, char* av[])
{
    try
    {
        alex::globals::initializeDefaults();
        std::string pattern = ".*";
        std::string stopWordsFile;
        std::string query;
        std::string iidxPath, sidxPath;
        size_t depth = std::numeric_limits<int>::max();
        bool verbose = alex::globals::default_verbose;
        bool stemming = alex::globals::default_stemming;

        double bm25_k1 = alex::globals::bm25_k1;
        double bm25_b = alex::globals::bm25_b;

        double alpha_weighted_avg = alex::globals::alpha_weighted_avg;
        //double beta_min_pair = alex::globals::beta_min_pair;
        //double alpha_min_pair = alex::globals::alpha_min_pair;

        std::unordered_set<std::string> stopWords = alex::globals::default_stop_words;
        std::vector<std::string> secrets = alex::globals::default_secrets;
        std::string configFile;
        unsigned int obfuscations = alex::globals::default_obfuscations;

        // 0 = locations, 1 = frequency, 2 = min pairs, 3 = boolean, 4 = bm25 score, 5 = mindist score, 6 = combination of 4/5, 7 = recall/precision (boolean)
        int queryType = 0;
        std::vector<std::string> types = {"psib", "psif", "psip", "psim", "bsib"};

        po::options_description desc("options");
        desc.add_options()
            ("help,h", "help message")
            ("config,c", po::value<string>(&configFile), "load a config file")
            ("secrets,s", po::value<vector<string>>(&secrets), "secrets to construct hidden terms with for query obfuscation")
            ("recursive,r", po::value<size_t>(&depth)->default_value(std::numeric_limits<size_t>::max()), "depth of indir recursion")
            ("verbose,v", po::value<bool>(&verbose)->default_value(verbose), "verbose mode")
            ("pattern,p",  po::value<string>(&pattern)->default_value(pattern), "file pattern to ")
            ("obfuscations",  po::value<unsigned int>(&obfuscations)->default_value(obfuscations), "number of noise terms/query")
            ("stop_words", po::value<string>(&stopWordsFile), "stop words file")
            ("stemming", po::value<bool>(&stemming)->default_value(stemming), "porter2stemmer")
            ("bm_25.k1", po::value<double>(&bm25_k1)->default_value(bm25_k1))
            ("bm_25.b", po::value<double>(&bm25_b)->default_value(bm25_b))
            ("mindist.beta", po::value<double>(&beta_min_pair)->default_value(beta_min_pair))
            ("mindist.alpha", po::value<double>(&alpha_min_pair)->default_value(alpha_min_pair))
            ("weighted_avg", po::value<double>(&alpha_weighted_avg)->default_value(alpha_weighted_avg))
            ("result", po::value<int>(&queryType)->default_value(queryType), "result type: 0=locations, 1=freq, 2=mindist pairs, 3=boolean, 4=bm25, 5=mindist, 6=comb 4/5")
            ("query", po::value<string>(&query)->required(), "query")
            ("iidx_path", po::value<string>(&iidxPath), "inverted index db index files")
            ("sidx_path", po::value<string>(&sidxPath), "secure index db index files")
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
        alex::index::HiddenQueryBuilder hqb(hqParams);

        alex::index::InvertedIndexDb::Params iidxParams;
        iidxParams.bm25_k1 = bm25_k1;
        iidxParams.bm25_b = bm25_b;
        iidxParams.alpha_min_pair = alpha_min_pair;
        iidxParams.beta_min_pair = beta_min_pair;
        iidxParams.alpha_weighted_avg = alpha_weighted_avg;

        alex::index::SecureIndexDb::Params sidxParams;
        sidxParams.bm25_k1 = bm25_k1;
        sidxParams.bm25_b = bm25_b;
        sidxParams.alpha_min_pair = alpha_min_pair;
        sidxParams.beta_min_pair = beta_min_pair;
        sidxParams.alpha_weighted_avg = alpha_weighted_avg;

        if (!iidxPath.empty())
        {
            alex::index::InvertedIndexDb db(iidxParams);
            db.read(iidxPath);
            cout << "inverted index" << endl;

            // 0 = locations, 1 = frequency, 2 = min pairs, 3 = boolean, 4 = bm25 score, 5 = mindist score, 6 = combination of 4/5
            if (queryType == 0)
            {
                auto rs = db.getLocations(qb.make(query)[0]);
                for (auto r : rs)
                {
                    if (r.second.empty())
                        continue;

                    cout << r.first->getReference() << " -> ";
                    for (auto l : r.second)
                        cout << l << " ";
                    cout << endl;
                }
            }
            else if (queryType == 1)
            {
                auto rs = db.getFrequencies(qb.make(query)[0]);
                for (auto r : rs)
                {
                    if (r.second > 0)
                        cout << r.first->getReference() << " -> " << r.second << std::endl;
                }
            }
            else if (queryType == 2)
            {
                auto rs = db.minPairwiseDistances(qb.make(query));
                for (auto r : rs)
                {
                    if (r.second.empty())
                        continue;

                    std::cout << r.first->getReference() << std::endl;
                    for (auto x : r.second)
                        std::cout << "\t" << alex::utils::toString(x.first.first) << " <-> " << alex::utils::toString(x.first.second) << " -> " << x.second << std::endl;
                }
            }
            else if (queryType == 3)
            {
                auto rs = db.containsAll(qb.make(query));
                for (auto r : rs)
                    std::cout << r->getReference() << std::endl;
            }
            else if (queryType == 4)
            {
                auto tmp = db.bm25(qb.make(query));
                std::vector<std::pair<alex::index::Index, double>> rs(tmp.begin(), tmp.end());
                std::sort(rs.begin(), rs.end(), [](std::pair<alex::index::Index, double> x1, std::pair<alex::index::Index, double> x2)
                {
                    return x2.second < x1.second;
                });
                for (auto r : rs)
                    std::cout << r.first->getReference() << " -> " << r.second << std::endl;
            }
            else if (queryType == 5)
            {
                auto tmp = db.minPairwiseDistanceScore(qb.make(query));
                std::vector<std::pair<alex::index::Index, double>> rs(tmp.begin(), tmp.end());
                std::sort(rs.begin(), rs.end(), [](std::pair<alex::index::Index, double> x1, std::pair<alex::index::Index, double> x2)
                {
                    return x2.second < x1.second;
                });

                for (auto r : rs)
                    std::cout << r.first->getReference() << " -> " << r.second << std::endl;
            }
            else if (queryType == 6)
            {
                auto tmp = db.getRanking2(qb.make(query));
                std::vector<std::pair<alex::index::Index, double>> rs(tmp.begin(), tmp.end());
                std::sort(rs.begin(), rs.end(), [](std::pair<alex::index::Index, double> x1, std::pair<alex::index::Index, double> x2)
                {
                    return x2.second < x1.second;
                });

                for (auto r : rs)
                    std::cout << r.first->getReference() << " -> " << r.second << std::endl;
            }
        }

        if (!sidxPath.empty())
        {
            alex::index::SecureIndexDb sdb(sidxParams);
            sdb.read(sidxPath);

            cout << "secure index" << endl;

            // 0 = locations, 1 = frequency, 2 = min pairs, 3 = boolean, 4 = bm25 score, 5 = mindist score, 6 = combination of 4/5
            if (queryType == 0)
            {
                auto rs = sdb.getLocations(hqb.make(query)[0]);
                for (auto r : rs)
                {
                    if (r.second.empty())
                        continue;

                    cout << r.first->getReference() << " -> ";
                    for (auto l : r.second)
                        cout << l << " ";
                    cout << endl;
                }
            }
            else if (queryType == 1)
            {
                auto rs = sdb.getFrequencies(hqb.make(query)[0]);
                for (auto r : rs)
                    if (r.second > 0)
                        cout << r.first->getReference() << " -> " << r.second << std::endl;
            }
            else if (queryType == 2)
            {
                auto rs = sdb.minPairwiseDistances(hqb.make(query));
                for (auto r : rs)
                {
                    if (r.second.empty())
                        continue;

                    std::cout << r.first->getReference() << std::endl;
                    for (auto x : r.second)
                    {
                        std::cout << "\t" << alex::utils::toString(x.first.first) << " <-> " << alex::utils::toString(x.first.second) << " -> " << x.second << std::endl;
                    }
                }
            }
            else if (queryType == 3)
            {
                auto rs = sdb.containsAll(hqb.make(query));
                for (auto r : rs)
                {
                    std::cout << r->getReference() << std::endl;
                }
            }
            else if (queryType == 4)
            {
                auto tmp = sdb.bm25(hqb.make(query));
                std::vector<std::pair<alex::index::SecureIndex, double>> rs(tmp.begin(), tmp.end());
                std::sort(rs.begin(), rs.end(), [](std::pair<alex::index::SecureIndex, double> x1, std::pair<alex::index::SecureIndex, double> x2)
                {
                    return x2.second < x1.second;
                });

                for (auto r : rs)
                {
                    std::cout << r.first->getReference() << " -> " << r.second << std::endl;
                }
            }
            else if (queryType == 5)
            {
                auto tmp = sdb.minPairwiseDistanceScore(hqb.make(query));
                std::vector<std::pair<alex::index::SecureIndex, double>> rs(tmp.begin(), tmp.end());
                std::sort(rs.begin(), rs.end(), [](std::pair<alex::index::SecureIndex, double> x1, std::pair<alex::index::SecureIndex, double> x2)
                {
                    return x2.second < x1.second;
                });

                for (auto r : rs)
                {
                    std::cout << r.first->getReference() << " -> " << r.second << std::endl;
                }
            }
            else if (queryType == 6)
            {
                auto tmp = sdb.getRanking2(hqb.make(query));
                std::vector<std::pair<alex::index::SecureIndex, double>> rs(tmp.begin(), tmp.end());
                std::sort(rs.begin(), rs.end(), [](std::pair<alex::index::SecureIndex, double> x1, std::pair<alex::index::SecureIndex, double> x2)
                {
                    return x2.second < x1.second;
                });

                for (auto r : rs)
                {
                    std::cout << r.first->getReference() << " -> " << r.second << std::endl;
                }
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
