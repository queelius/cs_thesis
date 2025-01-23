#include "stdafx.h"
#include "Benchmark.h"
#include "EvaluationTools.h"
#include "FileSystem.h"

namespace alex { namespace testing
{
    struct QueryResults
    {
        std::string label;
        double avgLag;
        double map;

        void print(ostream& outs = std::cout)
        {
            outs << "label " << label << endl;
            outs << "\tavg_query_lag_time " << avgLag << endl;
            outs << "\tmap " << map << std::endl;
        };
    };

	struct RecallPrecisionTag
	{
		std::string label;
		double avgLag;
		double avgRecall;
		double avgPrecision;

		void print(ostream& outs = std::cout)
		{
			outs << "label " << label << endl;
			outs << "\tavg_query_lag_time " << avgLag << endl;
			outs << "\tavg_precision " << avgPrecision << std::endl;
			outs << "\tavg_recall " << avgRecall << std::endl;
		};
	};

    std::map<std::string, QueryResults> minPairwiseDistanceScoreMAP(const std::vector<std::string> querySet,
                                  alex::index::HiddenQueryBuilder::BuilderParams hqParams,
                                  alex::index::QueryBuilder::BuilderParams qParams,
                                  alex::index::InvertedIndexDb::Params idxParams,
                                  alex::index::SecureIndexDb::Params sidxParams,
                                  const std::string dbDirIn,
                                  std::string label = "",
                                  unsigned int k = 0,
                                  std::vector<std::string> types = {"bsib", "psib", "psif", "psip", "psim"},
								  bool verbose = true,
                                  unsigned int trials = 1)
    {
        std::map<std::string, QueryResults> results;

        try
        {
            alex::index::HiddenQueryBuilder hq(hqParams);
            alex::index::QueryBuilder pq(qParams);

            auto idxDb = alex::index::InvertedIndexDb(idxParams);
            idxDb.read(dbDirIn + "\\canonical");
            alex::testing::Timer timer;

            for (auto type : types)
            {
				if (type == "psif")
					continue;

                auto sidxDb = alex::index::SecureIndexDb(sidxParams);
                sidxDb.read(dbDirIn + "\\" + type);
                
                long long totalLag = 0;
                double sumAvgPrec = 0;
                results[type].label = label;
				size_t nn = 0;
                for (auto q : querySet)
                {
                    auto query = pq.make(q);

					if (query.terms.size() <= 1)
						continue;

                    for (unsigned int trial = 0; trial < trials; ++trial)
                    {
					    ++nn;
                        auto canonical = idxDb.minPairwiseDistanceScore(query);
                        auto hiddenQuery = hq.make(q);

                        timer.reset();
                        auto actual = sidxDb.minPairwiseDistanceScore(hiddenQuery);
                        auto elapsed = timer.elapsed();

                        auto results = alex::eval::precisionAtK(canonical, actual, k);
                        sumAvgPrec += alex::eval::averagePrecisionAtK(results, k);
                        totalLag += elapsed.count();
                    }
                }

                results[type].label = type;
                if (nn == 0)
                {
                    results[type].avgLag = -1;
                    results[type].map = -1;
                }
                else
                {
                    results[type].avgLag = (double)totalLag / ((double)sidxDb.numIndexes() * (double)nn);
                    results[type].map = sumAvgPrec / ((double)nn);
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
        catch (const char* e)
        {
            std::cout << "Error: " << e << std::endl;
        }
        catch (...)
        {
            std::cout << "Error: Unknown cause" << std::endl;
        }

        return results;
    };

    std::map<std::string, QueryResults> bm25_MAP(const std::vector<std::string> querySet,
                                  alex::index::HiddenQueryBuilder::BuilderParams hqParams,
                                  alex::index::QueryBuilder::BuilderParams qParams,
                                  alex::index::InvertedIndexDb::Params idxParams,
                                  alex::index::SecureIndexDb::Params sidxParams,
                                  const std::string dbDirIn,
                                  std::string label = "",
                                  unsigned int k = 0,
                                  std::vector<std::string> types = {"bsib", "psib", "psif", "psip", "psim"},
								  bool verbose = true,
                                  unsigned int trials = 1)
    {
        std::map<std::string, QueryResults> results;

        try
        {
            alex::index::HiddenQueryBuilder hq(hqParams);
            alex::index::QueryBuilder pq(qParams);

            auto idxDb = alex::index::InvertedIndexDb(idxParams);
            idxDb.read(dbDirIn + "\\canonical");
            alex::testing::Timer timer;

            for (auto type : types)
            {
                auto sidxDb = alex::index::SecureIndexDb(sidxParams);
                sidxDb.read(dbDirIn + "\\" + type);
                
                long long totalLag = 0;
                double sumAvgPrec = 0;
				size_t nn = 0;
                results[type].label = label;
                for (auto q : querySet)
                {
                    auto query = pq.make(q);

					if (query.terms.size() < 1)
						continue;

                    for (unsigned int trial = 0; trial < trials; ++trial)
                    {
					    ++nn;
                        auto canonical = idxDb.bm25(query);
                        auto hiddenQuery = hq.make(q);

                        timer.reset();
                        auto actual = sidxDb.bm25(hiddenQuery);
                        auto elapsed = timer.elapsed();

                        auto results = alex::eval::precisionAtK(canonical, actual, k);
                        sumAvgPrec += alex::eval::averagePrecisionAtK(results, k);
                        totalLag += elapsed.count();
                    }
                }

                results[type].label = type;
                if (nn == 0)
                {
                    results[type].avgLag = -1;
                    results[type].map = -1;
                }
                else
                {
                    results[type].avgLag = (double)totalLag / (double)(sidxDb.numIndexes() * nn);
                    results[type].map = sumAvgPrec / (double)nn;
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
        catch (const char* e)
        {
            std::cout << "Error: " << e << std::endl;
        }
        catch (...)
        {
            std::cout << "Error: Unknown cause" << std::endl;
        }

        return results;
    };

	std::map<std::string, RecallPrecisionTag> recallPrecision(
		const std::vector<std::string> querySet,
		alex::index::HiddenQueryBuilder::BuilderParams hqParams,
		alex::index::QueryBuilder::BuilderParams qParams,
		alex::index::InvertedIndexDb::Params idxParams,
		alex::index::SecureIndexDb::Params sidxParams,
		const std::string dbDirIn,
		std::string label = "",
		std::vector<std::string> types = { "bsib", "psib", "psif", "psip", "psim" },
		bool verbose = true,
        unsigned int trials = 1)
	{
		std::map<std::string, RecallPrecisionTag> results;

		try
		{
			alex::index::HiddenQueryBuilder hq(hqParams);
			alex::index::QueryBuilder pq(qParams);

			auto idxDb = alex::index::InvertedIndexDb(idxParams);
			idxDb.read(dbDirIn + "\\canonical");
			alex::testing::Timer timer;

			for (auto type : types)
			{
				auto sidxDb = alex::index::SecureIndexDb(sidxParams);
				sidxDb.read(dbDirIn + "\\" + type);

				long long totalLag = 0;
				double sumAvgPrec = 0;
				double sumAvgRecall = 0;
				size_t nn = 0;
				results[type].label = label;
				for (auto q : querySet)
				{
					auto query = pq.make(q);
					if (query.terms.size() < 1)
						continue;

                    for (unsigned int trial = 0; trial < trials; ++trial)
                    {
					    ++nn;
					    auto canonical = idxDb.containsAll(query);
					    auto hiddenQuery = hq.make(q);

					    timer.reset();
					    auto actual = sidxDb.containsAll(hiddenQuery);
					    auto elapsed = timer.elapsed();

					    auto canonicalSet = std::unordered_set<alex::index::Index>(canonical.begin(), canonical.end());
					    auto actualSet = std::unordered_set<alex::index::SecureIndex>(actual.begin(), actual.end());
					    double precision = alex::eval::precision(canonicalSet, actualSet);
					    double recall = alex::eval::recall(canonicalSet, actualSet, q);

                        totalLag += elapsed.count();
					    sumAvgPrec += precision;
					    sumAvgRecall += recall;
                    }
				}

                results[type].label = type;
                if (nn == 0)
                {
				    results[type].avgLag = -1;
				    results[type].avgRecall = -1;
				    results[type].avgPrecision = -1;
                }
                else
                {
				    results[type].avgLag = (double)totalLag / (double)(sidxDb.numIndexes() * nn);
				    results[type].avgRecall = sumAvgRecall / (double)nn;
				    results[type].avgPrecision = sumAvgPrec / (double)nn;
                }
			}
		}
		catch (const std::exception& e)
		{
			std::cout << "Error: " << e.what() << std::endl;
		}
		catch (const char* e)
		{
			std::cout << "Error: " << e << std::endl;
		}
		catch (...)
		{
			std::cout << "Error: Unknown cause" << std::endl;
		}

		return results;
	};
}}