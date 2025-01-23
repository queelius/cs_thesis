#include "stdafx.h"
#include "Benchmark.h"
#include "EvaluationTools.h"

namespace alex { namespace testing
{
    struct ExperimentParameters
    {
        alex::index::BsiBlockBuilder::BuilderParams bsibParams;
        alex::index::PsiBlockBuilder::BuilderParams psibParams;
        alex::index::PsiFreqBuilder::BuilderParams psifParams;
        alex::index::PsiPostBuilder::BuilderParams psipParams;
        alex::index::PsiFreqBsiBlockBuilder::BuilderParams bsifParams;

        bool bsib = false;
        bool psib = false;
        bool psif = false;
        bool psip = false;
        bool bsif = false;
    };


    template <class IndexBuilder>
    alex::index::InvertedIndexDb readCorpusIndexDb(
        const boost::filesystem::path& corpus,
        const boost::filesystem::path& db,
        const typename IndexBuilder::BuilderParams& params = alex::index::InvertedIndexBuilder::BuilderParams(),
        const typename alex::index::InvertedIndexDb::Params& dbParams = alex::index::InvertedIndexDb::Params(),
        unsigned int recursionDepth = 1,
        bool refresh = false);

    template <class IndexBuilder>
    alex::index::SecureIndexDb readCorpusSecureIndexDb(
        const boost::filesystem::path& corpus,
        const boost::filesystem::path& db,
        const typename IndexBuilder::BuilderParams& builderParams = IndexBuilder::BuilderParams(),
        const typename alex::index::SecureIndexDb::Params& dbParams = alex::index::SecureIndexDb::Params(),
        unsigned int recursionDepth = 1,
        bool refresh = false);

    template <class IndexBuilder>
    alex::index::SecureIndexDb readCorpusSecureIndexDb(
        const boost::filesystem::path& corpus,
        const boost::filesystem::path& db,
        const typename IndexBuilder::BuilderParams& builderParams,
        const typename alex::index::SecureIndexDb::Params& dbParams,
        unsigned int recursionDepth,
        bool refresh)
    {
        if (refresh || !boost::filesystem::exists(db))
        {
            if (boost::filesystem::exists(db))
                boost::filesystem::remove_all(db);

            alex::index::IndexBatchBuilder<IndexBuilder> batch(builderParams);
            batch.build(db, corpus, recursionDepth);
        }

        alex::index::SecureIndexDb siDb(dbParams);
        siDb.read(db);
        return siDb;
    }

    template <class IndexBuilder>
    alex::index::InvertedIndexDb readCorpusIndexDb(
        const boost::filesystem::path& corpus,
        const boost::filesystem::path& db,
        const typename IndexBuilder::BuilderParams& params,
        const typename alex::index::InvertedIndexDb::Params& dbParams,
        unsigned int recursionDepth,
        bool refresh)
    {
        if (refresh || !boost::filesystem::exists(db))
        {
            if (boost::filesystem::exists(db))
                boost::filesystem::remove_all(db);

            alex::index::IndexBatchBuilder<IndexBuilder> batch(params);
            batch.build(db, corpus, recursionDepth);
        }

        alex::index::InvertedIndexDb invDb(dbParams);
        invDb.read(db);
        return invDb;
    }

    void TermTestHelper(alex::index::Index idx, alex::index::Query::Term term, std::string label = "")
    {
        if (!label.empty())
            std::cout << "label: " << label << std::endl;

        std::cout << idx->getPreferredFileExtension() << std::endl;
        std::cout << "-----------------------------" << std::endl;
        std::cout << "\treference\t-> " << idx->getReference() << std::endl;
        std::cout << "\tlocations\t-> ";
        auto locs = idx->getLocations(term);
        for (auto l : locs)
            std::cout << l << " ";
        std::cout << std::endl;
        std::cout << "\tfrequency\t-> " << idx->getFrequency(term) << std::endl;
        std::cout << "\tcontains\t-> " << (idx->contains(term) ? "true" : "false") << std::endl;
        std::cout << std::endl;
    };

    void MinPairTestHelper(alex::index::Index idx, alex::index::Query q, std::string label = "")
    {
        if (!label.empty())
            std::cout << "label: " << label << std::endl;

        std::cout << idx->getPreferredFileExtension() << std::endl;
        std::cout << "-----------------------------" << std::endl;
        std::cout << "\treference\t-> " << idx->getReference() << std::endl;
        auto minPairs = idx->getMinPairWiseDistances(q);
        for (auto p : minPairs)
        {
            std::cout << alex::utils::toString(p.first.first) << " <-> "
                << alex::utils::toString(p.first.second) << p.second << std::endl;
        }
        std::cout << std::endl;
    };

    void TermTestHelper(alex::index::SecureIndex sidx, alex::index::HiddenQuery::HiddenTerm term, std::string label = "", bool refresh = false, int depth = 1)
    {
        if (!label.empty())
            std::cout << "label: " << label << std::endl;

        std::cout << sidx->getPreferredFileExtension() << std::endl;
        std::cout << "-----------------------------" << std::endl;
        std::cout << "\treference\t-> " << sidx->getReference() << std::endl;

        try
        {
            auto locs = sidx->getLocations(term);
            std::cout << "\tlocations\t-> ";
            for (auto l : locs)
                std::cout << l << " ";
            std::cout << std::endl;
        }
        catch (std::exception e)
        {
            std::cout << "\tlocations\t-> " << e.what() << std::endl;
        }

        try
        {
            auto freq = sidx->getFrequency(term);
            std::cout << "\tfrequency\t-> " << freq << std::endl;
        }
        catch (std::exception e)
        {
            std::cout << "\tfrequency\t-> " << e.what() << std::endl;
        }

        try
        {
            bool contains = sidx->contains(term);
            std::cout << "\tcontains\t-> " << (contains ? "true" : "false") << std::endl;
        }
        catch (std::exception e)
        {
            std::cout << "\tcontains\t-> " << e.what() << std::endl;
        }   
    };

    void MinPairwiseDistTest(const std::string query, const std::string indir, const std::string outdir, std::string label = "", bool refresh = false, int depth = 1)
    {
        std::cout << "-----------------------------------------" << std::endl;
        std::cout << "MinPairwiseDistTest: " << label << std::endl;
        std::cout << "-----------------------------------------" << std::endl;
        try
        {
            alex::globals::initializeDefaults();
            bool stemming = false;
            double fpRate = 0.001;
            int wordsPerBlock = 250;
            int maxBlocks = 64;
            int maxFreq = 500;
            int verbose = 0;
            int offsetRadius = 125;
            int maxMinPairDist = 15;
            int obfuscations = 0;
            std::vector<std::string> secrets = {"secret"};
            std::unordered_set<std::string> ignoreWords; // = ({ "the", "and", "is", "we" });

            alex::index::InvertedIndexDb::Params dbParams;
            alex::index::SecureIndexDb::Params sdbParams;
            dbParams.defaults();
            sdbParams.defaults();

            // make queries
            alex::index::HiddenQueryBuilder::BuilderParams qParams;
            qParams.defaults();
            qParams.obfuscations = obfuscations;
            qParams.secrets = secrets;
            alex::index::HiddenQueryBuilder q(qParams);
            alex::index::QueryBuilder::BuilderParams pqParams;
            pqParams.defaults();
            alex::index::QueryBuilder pq(pqParams);

            alex::index::InvertedIndexBuilder::BuilderParams params0;
            params0.defaults();
            params0.ignoreWords = ignoreWords;
            params0.stemming = stemming;
            params0.verbose = verbose;       

            alex::index::BsiBlockBuilder::BuilderParams params1;
            params1.falsePositiveRate = fpRate;
            params1.verbose = verbose;
            params1.wordsPerBlock = wordsPerBlock;
            params1.maxBlocks = maxBlocks;
            params1.stemming = stemming;
            params1.ignoreWords = ignoreWords;
            params1.secrets = secrets;

            alex::index::PsiBlockBuilder::BuilderParams params2;
            params2.falsePositiveRate(fpRate);
            params2.verbose = verbose;
            params2.wordsPerBlock = wordsPerBlock;
            params2.maxBlocks = maxBlocks;
            params2.stemming = stemming;
            params2.ignoreWords = ignoreWords;
            params2.secrets = secrets;

            alex::index::PsiFreqBuilder::BuilderParams params3;
            params3.falsePositiveRate(fpRate);
            params3.verbose = verbose;
            params3.stemming = stemming;
            params3.maxFreq = maxFreq;
            params3.ignoreWords = ignoreWords;
            params3.secrets = secrets;

            alex::index::PsiPostBuilder::BuilderParams params4;
            params4.falsePositiveRate(fpRate);
            params4.verbose = verbose;
            params4.stemming = stemming;
            params4.offsetRadius = offsetRadius;
            params4.ignoreWords = ignoreWords;
            params4.secrets = secrets;

            alex::index::PsiFreqBsiBlockBuilder::BuilderParams params5;
            params5.falsePositiveRate = fpRate;
            params5.verbose = verbose;
            params5.stemming = stemming;
            params5.maxBlocks = maxBlocks;
            params5.maxFreq = maxFreq;
            params5.wordsPerBlock = wordsPerBlock;
            params5.ignoreWords = ignoreWords;
            params5.secrets = secrets;

            alex::index::PsiMinBuilder::BuilderParams params6;
            params6.freqParams.falsePositiveRate(fpRate);
            params6.maxMinPairMaxDist = maxMinPairDist; // note: params for max min pair dist is misnamed
            params6.verbose = verbose;
            params6.freqParams.verbose = verbose;
            params6.freqParams.stemming = stemming;
            params6.freqParams.maxFreq = maxFreq;
            params6.freqParams.ignoreWords = ignoreWords;
            params6.freqParams.secrets = secrets;
        
            auto idxDb = alex::testing::readCorpusIndexDb<alex::index::InvertedIndexBuilder>(indir, outdir + "/idx", params0, dbParams, depth, refresh);
            auto bsibDb = alex::testing::readCorpusSecureIndexDb<alex::index::BsiBlockBuilder>(indir, outdir + "/bsib", params1, sdbParams, depth, refresh);
            auto psibDb = alex::testing::readCorpusSecureIndexDb<alex::index::PsiBlockBuilder>(indir, outdir + "/psib", params2, sdbParams, depth, refresh);
            auto psifDb = alex::testing::readCorpusSecureIndexDb<alex::index::PsiFreqBuilder>(indir, outdir + "/psif", params3, sdbParams, depth, refresh);
            auto psipDb = alex::testing::readCorpusSecureIndexDb<alex::index::PsiPostBuilder>(indir, outdir + "/psip", params4, sdbParams, depth, refresh);
            auto bsifDb = alex::testing::readCorpusSecureIndexDb<alex::index::PsiFreqBsiBlockBuilder>(indir, outdir + "/bsif", params5, sdbParams, depth, refresh);
            auto psimDb = alex::testing::readCorpusSecureIndexDb<alex::index::PsiMinBuilder>(indir, outdir + "/bsim", params6, sdbParams, depth, refresh);

            auto r0 = idxDb.minPairwiseDistances(pq.make(query));
            auto r1 = bsibDb.minPairwiseDistances(q.make(query));
            auto r2 = psibDb.minPairwiseDistances(q.make(query));
            auto r3 = psifDb.minPairwiseDistances(q.make(query));
            auto r4 = psipDb.minPairwiseDistances(q.make(query));
            auto r5 = bsifDb.minPairwiseDistances(q.make(query));
            auto r6 = psimDb.minPairwiseDistances(q.make(query));

            std::cout << "idx" << std::endl;
            std::cout << "----" << std::endl;
            for (auto r : r0)
            {
                std::cout << r.first->getReference() << std::endl;
                for (auto x : r.second)
                    std::cout << "\t" << alex::utils::toString(x.first.first) << " <-> " << alex::utils::toString(x.first.second) << " = " << x.second << std::endl;
            }
            std::cout << "bsib" << std::endl;
            std::cout << "----" << std::endl;
            for (auto r : r1)
            {
                std::cout << r.first->getReference() << std::endl;
                for (auto x : r.second)
                {
                    
                    std::cout << "\t" << alex::utils::toString(x.first.first) << " <-> " << alex::utils::toString(x.first.second) << " = " << x.second << std::endl;
                }
            }
            std::cout << "psib" << std::endl;
            std::cout << "----" << std::endl;
            for (auto r : r2)
            {
                std::cout << r.first->getReference() << std::endl;
                for (auto x : r.second)
                    std::cout << "\t" << alex::utils::toString(x.first.first) << " <-> " << alex::utils::toString(x.first.second) << " = " << x.second << std::endl;
            }
            std::cout << "psif" << std::endl;
            std::cout << "----" << std::endl;
            for (auto r : r3)
            {
                std::cout << r.first->getReference() << std::endl;
                for (auto x : r.second)
                    std::cout << "\t" << alex::utils::toString(x.first.first) << " <-> " << alex::utils::toString(x.first.second) << " = " << x.second << std::endl;
            }
            std::cout << "psip" << std::endl;
            std::cout << "----" << std::endl;
            for (auto r : r4)
            {
                std::cout << r.first->getReference() << std::endl;
                for (auto x : r.second)
                    std::cout << "\t" << alex::utils::toString(x.first.first) << " <-> " << alex::utils::toString(x.first.second) << " = " << x.second << std::endl;
            }
            std::cout << "bsif" << std::endl;
            std::cout << "----" << std::endl;
            for (auto r : r5)
            {
                std::cout << r.first->getReference() << std::endl;
                for (auto x : r.second)
                    std::cout << "\t" << alex::utils::toString(x.first.first) << " <-> " << alex::utils::toString(x.first.second) << " = " << x.second << std::endl;
            }
            std::cout << "psim" << std::endl;
            std::cout << "----" << std::endl;
            for (auto r : r6)
            {
                std::cout << r.first->getReference() << std::endl;
                for (auto x : r.second)
                    std::cout << "\t" << alex::utils::toString(x.first.first) << " <-> " << alex::utils::toString(x.first.second) << " = " << x.second << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "Error Running MinPairWiseDistTest: " << e.what() << std::endl;
        }
    }

    void MinPairwiseDistScoreTest(const std::string query, const std::string indir, const std::string outdir, std::string label = "", bool refresh = false, int depth = 1)
    {
        std::cout << "-----------------------------------------" << std::endl;
        std::cout << "MinPairwiseDistScoreTest: " << label << std::endl;
        std::cout << "-----------------------------------------" << std::endl;
        try
        {
            using namespace alex::eval;
            alex::globals::initializeDefaults();
            bool stemming = false;
            double fpRate = 0.001;
            int wordsPerBlock = 250;
            int maxBlocks = 64;
            int maxFreq = 500;
            int verbose = 1;
            int offsetRadius = 0;
            int maxMinPairDist = 15;
            int obfuscations = 0;
            std::vector<std::string> secrets = {"secret"};
            std::unordered_set<std::string> ignoreWords; // = ({ "the", "and", "is", "we" });

            alex::index::InvertedIndexDb::Params dbParams;
            alex::index::SecureIndexDb::Params sdbParams;
            dbParams.defaults();
            dbParams.beta_min_pair = 0.25;
            sdbParams.defaults();
            sdbParams.beta_min_pair = 0.25;

            alex::index::InvertedIndexBuilder::BuilderParams params0;
            params0.defaults();
            params0.ignoreWords = ignoreWords;
            params0.stemming = stemming;
            params0.verbose = verbose;       

            alex::index::BsiBlockBuilder::BuilderParams params1;
            params1.falsePositiveRate = fpRate;
            params1.verbose = verbose;
            params1.wordsPerBlock = wordsPerBlock;
            params1.maxBlocks = maxBlocks;
            params1.stemming = stemming;
            params1.ignoreWords = ignoreWords;
            params1.secrets = secrets;

            alex::index::PsiBlockBuilder::BuilderParams params2;
            params2.falsePositiveRate(fpRate);
            params2.verbose = verbose;
            params2.wordsPerBlock = wordsPerBlock;
            params2.maxBlocks = maxBlocks;
            params2.stemming = stemming;
            params2.ignoreWords = ignoreWords;
            params2.secrets = secrets;

            alex::index::PsiFreqBuilder::BuilderParams params3;
            params3.falsePositiveRate(fpRate);
            params3.verbose = verbose;
            params3.stemming = stemming;
            params3.maxFreq = maxFreq;
            params3.ignoreWords = ignoreWords;
            params3.secrets = secrets;

            alex::index::PsiPostBuilder::BuilderParams params4;
            params4.falsePositiveRate(fpRate);
            params4.verbose = verbose;
            params4.stemming = stemming;
            params4.offsetRadius = offsetRadius;
            params4.ignoreWords = ignoreWords;
            params4.secrets = secrets;

            alex::index::PsiFreqBsiBlockBuilder::BuilderParams params5;
            params5.falsePositiveRate = fpRate;
            params5.verbose = verbose;
            params5.stemming = stemming;
            params5.maxBlocks = maxBlocks;
            params5.maxFreq = maxFreq;
            params5.wordsPerBlock = wordsPerBlock;
            params5.ignoreWords = ignoreWords;
            params5.secrets = secrets;

            alex::index::PsiMinBuilder::BuilderParams params6;
            params6.freqParams.falsePositiveRate(fpRate);
            params6.maxMinPairMaxDist = maxMinPairDist; // note: params for max min pair dist is misnamed
            params6.verbose = verbose;
            params6.freqParams.verbose = verbose;
            params6.freqParams.stemming = stemming;
            params6.freqParams.maxFreq = maxFreq;
            params6.freqParams.ignoreWords = ignoreWords;
            params6.freqParams.secrets = secrets;
        
            auto idxDb = readCorpusIndexDb<alex::index::InvertedIndexBuilder>(indir, outdir + "/idx", params0, dbParams, depth, refresh);
            auto bsibDb = readCorpusSecureIndexDb<alex::index::BsiBlockBuilder>(indir, outdir + "/bsib", params1, sdbParams, depth, refresh);
            auto psibDb = readCorpusSecureIndexDb<alex::index::PsiBlockBuilder>(indir, outdir + "/psib", params2, sdbParams, depth, refresh);
            auto psifDb = readCorpusSecureIndexDb<alex::index::PsiFreqBuilder>(indir, outdir + "/psif", params3, sdbParams, depth, refresh);
            auto psipDb = readCorpusSecureIndexDb<alex::index::PsiPostBuilder>(indir, outdir + "/psip", params4, sdbParams, depth, refresh);
            auto bsifDb = readCorpusSecureIndexDb<alex::index::PsiFreqBsiBlockBuilder>(indir, outdir + "/bsif", params5, sdbParams, depth, refresh);
            auto psimDb = readCorpusSecureIndexDb<alex::index::PsiMinBuilder>(indir, outdir + "/bsim", params6, sdbParams, depth, refresh);

            alex::index::HiddenQueryBuilder::BuilderParams qParams;
            qParams.defaults();
            qParams.obfuscations = obfuscations;
            qParams.secrets = secrets;
            alex::index::HiddenQueryBuilder q(qParams);

            alex::index::QueryBuilder::BuilderParams pqParams;
            pqParams.defaults();
            alex::index::QueryBuilder pq(pqParams);
            std::cout << "query: " << pq.make(query).toString() << std::endl;

            auto r0score = idxDb.minPairwiseDistanceScore(pq.make(query));
            auto r1score = bsibDb.minPairwiseDistanceScore(q.make(query));
            auto r2score = psibDb.minPairwiseDistanceScore(q.make(query));
            auto r3score = psifDb.minPairwiseDistanceScore(q.make(query));
            auto r4score = psipDb.minPairwiseDistanceScore(q.make(query));
            auto r5score = bsifDb.minPairwiseDistanceScore(q.make(query));
            auto r6score = psimDb.minPairwiseDistanceScore(q.make(query));

            absoluteError(r0score, r1score);
            absoluteError(r0score, r2score);
            absoluteError(r0score, r3score);
            absoluteError(r0score, r4score);
            absoluteError(r0score, r5score);
            absoluteError(r0score, r6score);
        }
        catch (const std::exception& e)
        {
            std::cout << "Error Running MinPairWiseDistScoreTest: " << e.what() << std::endl;
        }
    }

    void bm25Test(const std::string query, const std::string indir, const std::string outdir, std::string label = "", bool refresh = false, int depth = 1)
    {
        std::cout << "-----------------------------------------" << std::endl;
        std::cout << "Bm25 Test: " << label << std::endl;
        std::cout << "-----------------------------------------" << std::endl;
        
        Benchmark bm(std::cout);
        namespace q = alex::index;
        using namespace alex::eval;
        alex::globals::initializeDefaults();
        const bool stemming = false;
        const double fpRate = 0.001;
        const int wordsPerBlock = 250;
        const int maxBlocks = 64;
        const int maxFreq = 500;
        const int verbose = 1;
        const int offsetRadius = 10;
        const int maxMinPairDist = 10;
        const std::unordered_set<std::string> ignoreWords; // = ({ "the", "and", "is", "we" });

        q::HiddenQueryBuilder::BuilderParams qParams;
        qParams.defaults();
        qParams.obfuscations = 0;
        q::HiddenQueryBuilder qu(qParams);

        q::QueryBuilder::BuilderParams pqParams;
        pqParams.defaults();
        q::QueryBuilder pq(pqParams);

        q::InvertedIndexDb::Params dbParams;
        q::SecureIndexDb::Params sdbParams;
        dbParams.defaults();
        sdbParams.defaults();

        q::InvertedIndexBuilder::BuilderParams params0;
        params0.defaults();
        params0.ignoreWords = ignoreWords;
        params0.stemming = stemming;
        params0.verbose = verbose;       
        auto idxDb = readCorpusIndexDb<alex::index::InvertedIndexBuilder>(indir, outdir + "/idx", params0, dbParams, depth, refresh);

        q::BsiBlockBuilder::BuilderParams params1;
        params1.defaults();
        params1.falsePositiveRate = fpRate;
        params1.verbose = verbose;
        params1.wordsPerBlock = wordsPerBlock;
        params1.maxBlocks = maxBlocks;
        params1.stemming = stemming;
        params1.ignoreWords = ignoreWords;
        q::SecureIndexDb bsibDb;
        bm.run("reading bsib db for bm25 test", [&bsibDb, indir, outdir, params1, sdbParams, depth, refresh]()
            { bsibDb = readCorpusSecureIndexDb<q::BsiBlockBuilder>(indir, outdir + "/bsib", params1, sdbParams, depth, refresh); });

        q::PsiBlockBuilder::BuilderParams params2;
        params2.defaults();
        params2.falsePositiveRate(fpRate);
        params2.verbose = verbose;
        params2.wordsPerBlock = wordsPerBlock;
        params2.maxBlocks = maxBlocks;
        params2.stemming = stemming;
        params2.ignoreWords = ignoreWords;
        q::SecureIndexDb psibDb;
        bm.run("reading psib db for bm25 test", [&psibDb, indir, outdir, params2, sdbParams, depth, refresh]()
            { psibDb = readCorpusSecureIndexDb<q::PsiBlockBuilder>(indir, outdir + "/psib", params2, sdbParams, depth, refresh); });

        q::PsiFreqBuilder::BuilderParams params3;
        params3.defaults();
        params3.falsePositiveRate(fpRate);
        params3.verbose = verbose;
        params3.stemming = stemming;
        params3.maxFreq = maxFreq;
        params3.ignoreWords = ignoreWords;
        q::SecureIndexDb psifDb;
        bm.run("reading psif db for bm25 test", [&psifDb, indir, outdir, params3, sdbParams, depth, refresh]()
            { psifDb = readCorpusSecureIndexDb<q::PsiFreqBuilder>(indir, outdir + "/psif", params3, sdbParams, depth, refresh); });

        q::PsiPostBuilder::BuilderParams params4;
        params4.falsePositiveRate(fpRate);
        params4.verbose = verbose;
        params4.stemming = stemming;
        params4.offsetRadius = offsetRadius;
        params4.ignoreWords = ignoreWords;
        q::SecureIndexDb psipDb;
        bm.run("reading psip db for bm25 test", [&psipDb, indir, outdir, params4, sdbParams, depth, refresh]()
            { psipDb = readCorpusSecureIndexDb<q::PsiPostBuilder>(indir, outdir + "/psip", params4, sdbParams, depth, refresh); });

        q::PsiFreqBsiBlockBuilder::BuilderParams params5;
        params5.falsePositiveRate = fpRate;
        params5.verbose = verbose;
        params5.stemming = stemming;
        params5.maxBlocks = maxBlocks;
        params5.maxFreq = maxFreq;
        params5.wordsPerBlock = wordsPerBlock;
        params5.ignoreWords = ignoreWords;
        q::SecureIndexDb bsifDb;
        bm.run("reading psif db for bm25 test", [&bsifDb, indir, outdir, params5, sdbParams, depth, refresh]()
            { bsifDb = readCorpusSecureIndexDb<alex::index::PsiFreqBsiBlockBuilder>(indir, outdir + "/bsif", params5, sdbParams, depth, refresh); });

        q::PsiMinBuilder::BuilderParams params6;
        params6.freqParams.falsePositiveRate(fpRate);
        params6.maxMinPairMaxDist = maxMinPairDist; // note: params for max min pair dist is misnamed
        params6.verbose = verbose;
        params6.freqParams.verbose = verbose;
        params6.freqParams.stemming = stemming;
        params6.freqParams.maxFreq = maxFreq;
        params6.freqParams.ignoreWords = ignoreWords;
        q::SecureIndexDb psimDb;
        bm.run("reading psim db for bm25 test", [&psimDb, indir, outdir, params6, sdbParams, depth, refresh]()
            { psimDb = readCorpusSecureIndexDb<alex::index::PsiMinBuilder>(indir, outdir + "/bsim", params6, sdbParams, depth, refresh); });

        auto truth = idxDb.bm25(pq.make(query));

        typedef std::unordered_map<q::SecureIndex, double> Results;

        Results x1;
        bm.run("query lag time for bm25 on bsib", [&x1, &bsibDb, &qu, &query]() { x1 = bsibDb.bm25(qu.make(query)); });
        absoluteError(truth, x1);
        auto prec1 = precisionAtK(truth, x1);
        std::cout << "average precision: " << averagePrecisionAtK(prec1) << std::endl;
        
        Results x2;
        bm.run("query lag time for bm25 on psib", [&x2, &psibDb, &qu, &query]() { x2 = psibDb.bm25(qu.make(query)); });
        absoluteError(truth, x2);
        auto prec2 = precisionAtK(truth, x2);
        std::cout << "average precision: " << averagePrecisionAtK(prec2) << std::endl;

        Results x3;
        bm.run("query lag time for bm25 on psif", [&x3, &psifDb, &qu, &query]() { x3 = psifDb.bm25(qu.make(query)); });
        absoluteError(truth, x3);
        auto prec3 = precisionAtK(truth, x3);
        std::cout << "average precision: " << averagePrecisionAtK(prec3) << std::endl;
        
        Results x4;
        bm.run("query lag time for bm25 on psip", [&x4, &psipDb, &qu, &query]() { x4 = psipDb.bm25(qu.make(query)); });
        absoluteError(truth, x4);
        auto prec4 = precisionAtK(truth, x4);
        std::cout << "average precision: " << averagePrecisionAtK(prec4) << std::endl;
        
        Results x5;
        bm.run("query lag time for bm25 on bsif", [&x5, &bsifDb, &qu, &query]() { x5 = bsifDb.bm25(qu.make(query)); });
        absoluteError(truth, x5);
        auto prec5 = precisionAtK(truth, x5);
        std::cout << "average precision: " << averagePrecisionAtK(prec5) << std::endl;
        
        Results x6;
        bm.run("query lag time for bm25 on psim", [&x6, &psimDb, &qu, &query]() { x6 = psimDb.bm25(qu.make(query)); });
        absoluteError(truth, x6);
        auto prec6 = precisionAtK(truth, x6);
        std::cout << "average precision: " << averagePrecisionAtK(prec6) << std::endl;
    }

    void TermTest(const std::string& filename, const std::string term, std::string label)
    {
        const int wordsPerBlock = 50;
        const double fpRate = 0.001;
        const int verbose = 0;
        const int maxBlocks = 100;
        const int maxFreq = 1000;
        const bool stemming = false;
        const int radius = 0;
        const int maxMinPair = 10;

        try
        {
            std::cout << "-----------------------------------------" << std::endl;
            std::cout << "TermTest: " << label << std::endl;
            std::cout << "-----------------------------------------" << std::endl;

            std::cout << "query term -> " << term << std::endl;

            namespace q = alex::index;
            q::QueryBuilder::BuilderParams paramsQb;
            paramsQb.ignoreWords = alex::globals::default_stop_words;
            paramsQb.stemming = alex::globals::default_stemming;       
            q::QueryBuilder qb(paramsQb);
            auto pterm = qb.make(term)[0];

            q::HiddenQueryBuilder::BuilderParams paramsHqb;
            paramsHqb.obfuscations = 0;
            paramsHqb.qb = paramsQb;
            q::HiddenQueryBuilder hqb(paramsHqb);
            auto hterm = hqb.make(term)[0];

            q::InvertedIndexBuilder::BuilderParams paramsIdxb;
            paramsIdxb.stemming = stemming;
            paramsIdxb.verbose = verbose;

            q::InvertedIndexBuilder iib(paramsIdxb);
            iib.parse(filename);
            iib.makeTerms();
            auto ii = iib.build();
            TermTestHelper(ii, pterm, "the reference");

            alex::index::PsiFreqBsiBlockBuilder::BuilderParams paramsPsifb;
            paramsPsifb.defaults();
            paramsPsifb.verbose = verbose;
            paramsPsifb.falsePositiveRate = fpRate;
            paramsPsifb.wordsPerBlock = wordsPerBlock;
            paramsPsifb.maxFreq = maxFreq;
            paramsPsifb.stemming = stemming;
            paramsPsifb.maxBlocks = maxBlocks;
            q::PsiFreqBsiBlockBuilder psifb(paramsPsifb);
            psifb.parse(filename);
            psifb.makeTerms();
            auto psifbi = psifb.build();
            TermTestHelper(psifbi, hterm);

            q::BsiBlockBuilder::BuilderParams paramsBsib;
            paramsBsib.defaults();
            paramsBsib.verbose = verbose;
            paramsBsib.stemming = stemming;
            paramsBsib.falsePositiveRate = fpRate;
            paramsBsib.wordsPerBlock = wordsPerBlock;
            paramsBsib.maxBlocks = maxBlocks;
            q::BsiBlockBuilder bsib(paramsBsib);
            bsib.parse(filename);
            bsib.makeTerms();
            auto bsibi = bsib.build();
            TermTestHelper(bsibi, hterm);

            q::PsiBlockBuilder::BuilderParams paramsPsib;
            paramsPsib.defaults();
            paramsPsib.stemming = stemming;
            paramsPsib.falsePositiveRate(fpRate);
            paramsPsib.wordsPerBlock = wordsPerBlock;
            paramsPsib.maxBlocks = maxBlocks;
            paramsPsib.verbose = verbose;
            q::PsiBlockBuilder psib(paramsPsib);
            psib.parse(filename);
            psib.makeTerms();
            auto psibi = psib.build();
            TermTestHelper(psibi, hterm);

            q::PsiFreqBuilder::BuilderParams paramsPsif;
            paramsPsif.defaults();
            paramsPsif.stemming = stemming;
            paramsPsif.falsePositiveRate(fpRate);
            paramsPsif.verbose = verbose;
            paramsPsif.maxFreq = maxFreq;
            q::PsiFreqBuilder psif(paramsPsif);
            psif.parse(filename);
            psif.makeTerms();
            auto psifi = psif.build();
            TermTestHelper(psifi, hterm);

            q::PsiPostBuilder::BuilderParams paramsPsip;
            paramsPsip.defaults();
            paramsPsip.verbose = verbose;
            paramsPsip.stemming = stemming;
            paramsPsip.offsetRadius = radius;
            paramsPsip.falsePositiveRate(fpRate);
            q::PsiPostBuilder psip(paramsPsip);
            psip.parse(filename);
            psip.makeTerms();
            auto psipi = psip.build();
            TermTestHelper(psipi, hterm);

            q::PsiMinBuilder::BuilderParams paramsPsim;
            paramsPsim.defaults();
            paramsPsim.verbose = verbose;
            paramsPsim.maxMinPairMaxDist = maxMinPair;
            paramsPsim.freqParams.stemming = stemming;
            paramsPsim.minPairFalsePositiveRate(fpRate);
            q::PsiMinBuilder psim(paramsPsim);
            psim.parse(filename);
            psim.makeTerms();
            auto psipm = psim.build();
            TermTestHelper(psipm, hterm);
        }
        catch (std::exception e)
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }

    void DbLoadTest(const std::string& dir, const std::string& query, std::string label)
    {
        std::cout << "-----------------------------------------" << std::endl;
        std::cout << "DbLoadTest: " << label << std::endl;
        std::cout << "-----------------------------------------" << std::endl;

        namespace q = alex::index;

        try
        {
            q::SecureIndexDb sdb;
            sdb.read(dir);

            q::QueryBuilder::BuilderParams paramsQb;
            paramsQb.ignoreWords = alex::globals::default_stop_words;
            paramsQb.stemming = alex::globals::default_stemming;       
            q::QueryBuilder qb(paramsQb);
            q::HiddenQueryBuilder::BuilderParams paramsHqb;
            paramsHqb.obfuscations = 0;
            paramsHqb.qb = paramsQb;
            q::HiddenQueryBuilder hqb(paramsHqb);
            auto hq = hqb.make(query);

            auto bm25Scores = sdb.bm25(hq);
            for (auto r : bm25Scores)
            {
                std::cout << r.first->getReference() << " -> " << r.second << std::endl;
            }

            auto pairWiseScores = sdb.minPairwiseDistanceScore(hq);
            for (auto r : pairWiseScores)
            {
                std::cout << r.first->getReference() << " -> " << r.second << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    void BatchBuildTest(const std::string& dir, std::string label, int depth = 1)
    {
        std::cout << "-----------------------------------------" << std::endl;
        std::cout << "BatchBuildTest: " << label << std::endl;
        std::cout << "-----------------------------------------" << std::endl;

        int maxBlocks = 100;
        int maxFreq = 500;
        bool stemming = false;
        int wordsPerBlock = 250;
        double fpRate = 0.001;
        int offsetRadius = 20;
        int maxMinPairDist = 10;
        int verbose = 0;
        Benchmark bm(std::cout);
        boost::filesystem::remove_all("BatchBuildTest");

        namespace q = alex::index;
        q::SecureIndexDb db;

        q::BsiBlockBuilder::BuilderParams paramsBsib;
        paramsBsib.defaults();
        paramsBsib.verbose = verbose;
        paramsBsib.stemming = stemming;
        paramsBsib.falsePositiveRate = fpRate;
        paramsBsib.wordsPerBlock = wordsPerBlock;
        paramsBsib.maxBlocks = maxBlocks;
        q::IndexBatchBuilder<q::BsiBlockBuilder> builder1(paramsBsib);
        
        bm.run("construction time for bsib: ", [&builder1, dir, depth]() { builder1.build("BatchBuildTest\\bsib", dir, depth); });
        bm.run("loading from disk time into db for bsib: ", [&db, dir, depth]() { db.read("BatchBuildTest\\bsib"); });

        q::PsiBlockBuilder::BuilderParams paramsPsib;
        paramsPsib.defaults();
        paramsPsib.verbose = verbose;
        paramsPsib.stemming = stemming;
        paramsPsib.falsePositiveRate(fpRate);
        paramsPsib.wordsPerBlock = wordsPerBlock;
        paramsPsib.maxBlocks = maxBlocks;
        q::IndexBatchBuilder<q::PsiBlockBuilder> builder2(paramsPsib);
        bm.run("construction time for psib: ", [&builder2, dir, depth]() { builder2.build("BatchBuildTest\\psib", dir, depth); });
        bm.run("loading from disk time into db for psib: ", [&db, dir, depth]() { db.read("BatchBuildTest\\psib"); });

        q::PsiFreqBuilder::BuilderParams paramsPsif;
        paramsPsif.defaults();
        paramsPsif.verbose = verbose;
        paramsPsif.stemming = stemming;
        paramsPsif.falsePositiveRate(fpRate);
        paramsPsif.maxFreq = maxFreq;
        q::IndexBatchBuilder<q::PsiFreqBuilder> builder3(paramsPsif);
        bm.run("construction time for psif: ", [&builder3, dir, depth]() { builder3.build("BatchBuildTest\\psif", dir, depth); });
        bm.run("loading from disk time into db for psif: ", [&db, dir, depth]() { db.read("BatchBuildTest\\psif"); });

        q::PsiMinBuilder::BuilderParams paramsPsim;
        paramsPsim.defaults();
        paramsPsim.verbose = verbose;
        paramsPsim.freqParams.defaults(); // freqParams takes care of other properties, like stemming -- this should be changed, flat params instead of nested
        paramsPsim.freqParams.maxFreq = maxFreq;
        paramsPsim.freqParams.falsePositiveRate(fpRate);
        paramsPsim.freqParams.stemming = stemming;
        paramsPsim.maxMinPairMaxDist = maxMinPairDist;
        paramsPsim.freqParams.stemming = stemming;
        paramsPsim.minPairFalsePositiveRate(fpRate);
        q::IndexBatchBuilder<q::PsiMinBuilder> builder4(paramsPsim);
        bm.run("construction time for psim: ", [&builder4, dir, depth]() { builder4.build("BatchBuildTest\\psim", dir, depth); });       
        bm.run("loading from disk time into db for psim: ", [&db, dir, depth]() { db.read("BatchBuildTest\\psim"); });

        q::PsiFreqBsiBlockBuilder::BuilderParams paramsBsif;
        paramsBsif.defaults();
        paramsBsif.wordsPerBlock = wordsPerBlock;
        paramsBsif.verbose = verbose;
        paramsBsif.stemming = stemming;
        paramsBsif.maxBlocks = maxBlocks;
        paramsBsif.maxFreq = maxFreq;
        paramsBsif.falsePositiveRate = fpRate;
        q::IndexBatchBuilder<q::PsiFreqBsiBlockBuilder> builder5(paramsBsif);
        bm.run("construction time for bsif: ", [&builder5, dir, depth]() { builder5.build("BatchBuildTest\\bsif", dir, depth); });
        bm.run("loading from disk time into db for bsif: ", [&db, dir, depth]() { db.read("BatchBuildTest\\bsif"); });

        q::PsiPostBuilder::BuilderParams paramsPsip;
        paramsPsip.defaults();
        paramsPsip.offsetRadius = offsetRadius;
        paramsPsip.stemming = stemming;
        paramsPsip.falsePositiveRate(fpRate);
        q::IndexBatchBuilder<q::PsiPostBuilder> builder6(paramsPsip);
        bm.run("construction time for psip: ", [&builder6, dir, depth]() { builder6.build("BatchBuildTest\\psip", dir, depth); });
        bm.run("loading from disk time into db for psip: ", [&db, dir, depth]() { db.read("BatchBuildTest\\psip"); });
    }









    void BatchBuildPsibTest(const std::string& dir, std::string label, int depth = 1)
    {
        std::cout << "-----------------------------------------" << std::endl;
        std::cout << "BatchBuildPsibTest: " << label << std::endl;
        std::cout << "-----------------------------------------" << std::endl;

        int maxBlocks = 75;
        bool stemming = true;
        int wordsPerBlock = 125;
        double fpRate = 0.001;
        int verbose = 1;
        Benchmark bm(std::cout);
        boost::filesystem::remove_all("BatchBuildPsibTest");

        namespace q = alex::index;
        q::PsiBlockBuilder::BuilderParams params;
        params.defaults();
        params.verbose = verbose;
        params.stemming = stemming;
        params.falsePositiveRate(fpRate);
        params.wordsPerBlock = wordsPerBlock;
        params.maxBlocks = maxBlocks;
        q::IndexBatchBuilder<q::PsiBlockBuilder> builder(params);
        
        bm.run("construction time for bsib: ", [&builder, dir, depth]() { builder.build("BatchBuildTest\\psib", dir, depth); });

        q::SecureIndexDb db;
        bm.run("loading from disk time into db for bsib: ", [&db, dir, depth]() { db.read("BatchBuildTest\\psib"); });
    }
}}