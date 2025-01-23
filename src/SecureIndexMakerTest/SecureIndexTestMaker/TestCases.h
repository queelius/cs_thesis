#include "stdafx.h"
#include "Benchmark.h"
#include "EvaluationTools.h"
#include "FileSystem.h"

namespace alex { namespace testing
{
    struct ConstructionResults
    {
        std::string label;
        alex::testing::Timer::milliseconds lag;
        size_t indexSize;
        std::string path;
        size_t numIndexes;
        size_t corpusSize;
        alex::testing::Timer::milliseconds loadLag;

        void print(ostream& outs = std::cout)
        {
            outs << label << endl;
            //outs << "\tcorpus_size\t" << corpusSize << endl;
            outs << "\t index_size\t" << indexSize << endl;
            outs << "\t build_time\t" << lag.count() << endl;
            outs << "\t    indexed\t" << numIndexes << endl;
            outs << "\t       path\t" << path << endl;
            outs << "\t  load_time\t" << loadLag.count() << endl;
        };
    };

    std::map<std::string, ConstructionResults>  SecureIndexBatchBuild(
        const std::string& indir,
        const std::string& outdir,
        const std::string& label,
        const std::string& pattern,
        const std::vector<std::string>& secrets,
        alex::index::PsiBlockBuilder::BuilderParams psibParams,
        alex::index::PsiFreqBuilder::BuilderParams psifParams,
        alex::index::PsiPostBuilder::BuilderParams psipParams,
        alex::index::PsiMinBuilder::BuilderParams psimParams,
        alex::index::BsiBlockBuilder::BuilderParams bsibParams,
        alex::index::InvertedIndexBuilder::BuilderParams iidxParams,
        const std::unordered_set<std::string>& stopWords,
        size_t depth = 1,
        bool verbose = false,
        bool stemming = false,
        bool concurrency = true)
    {
        namespace fs = boost::filesystem;
        std::map<std::string, ConstructionResults> log;
        namespace q = alex::index;
        alex::testing::Timer timer;

        iidxParams.ignoreWords = stopWords;
        psibParams.ignoreWords = stopWords;
        psifParams.ignoreWords = stopWords;
        psipParams.ignoreWords = stopWords;
        psimParams.ignoreWords = stopWords;
        bsibParams.ignoreWords = stopWords;

        psibParams.secrets = secrets;
        psifParams.secrets = secrets;
        psipParams.secrets = secrets;
        psimParams.secrets = secrets;
        bsibParams.secrets = secrets;

        iidxParams.stemming = stemming;
        psibParams.stemming = stemming;
        psifParams.stemming = stemming;
        psipParams.stemming = stemming;
        psimParams.stemming = stemming;
        bsibParams.stemming = stemming;

        iidxParams.verbose = verbose;
        psibParams.verbose = verbose;
        psifParams.verbose = verbose;
        psipParams.verbose = verbose;
        psimParams.verbose = verbose;
        psimParams.verbose = verbose;
        bsibParams.verbose = verbose;

        size_t corpusSize = alex::io::filesystem::getDirectorySize(indir, ".*", depth);

        q::IndexBatchBuilder<q::InvertedIndexBuilder> iidxBuilder(iidxParams);
        

        log["iidx"].path = outdir + "\\iidx";
        fs::remove_all(log["iidx"].path);
        alex::io::filesystem::makeDirectory(log["iidx"].path);
        timer.reset();
        log["iidx"].numIndexes = iidxBuilder.build(log["iidx"].path, indir, depth, pattern, concurrency, verbose);
        log["iidx"].lag = timer.elapsed();
        log["iidx"].label = "iidx";
        log["iidx"].indexSize = alex::io::filesystem::getDirectorySize(log["iidx"].path, ".*", depth);
        log["iidx"].corpusSize = corpusSize;

        auto types = {"psim", "psif", "psip", "psib", "bsib"};
        try
        {
            /*
             * PsiBlock
             */
            log["psib"].path = outdir + "\\psib";
            fs::remove_all(log["psib"].path);
            q::IndexBatchBuilder<q::PsiBlockBuilder> psibBuilder(psibParams);
            alex::io::filesystem::makeDirectory(log["psib"].path);
            timer.reset();
            log["psib"].numIndexes = psibBuilder.build(log["psib"].path, indir, depth, pattern, concurrency, verbose);
            log["psib"].lag = timer.elapsed();
            log["psib"].label = "psib";
            log["psib"].indexSize = alex::io::filesystem::getDirectorySize(log["psib"].path, ".*", depth);
            log["psib"].corpusSize = corpusSize;

            /*
             * PsiFreq
             */
            log["psif"].path = outdir + "\\psif";
            fs::remove_all(log["psif"].path);
            q::IndexBatchBuilder<q::PsiFreqBuilder> psifBuilder(psifParams);
            alex::io::filesystem::makeDirectory(log["psif"].path);
            timer.reset();
            log["psif"].numIndexes = psifBuilder.build(log["psif"].path, indir, depth, pattern, concurrency, verbose);
            log["psif"].lag = timer.elapsed();
            log["psif"].label = "psif";
            log["psif"].indexSize = alex::io::filesystem::getDirectorySize(log["psif"].path, ".*", depth);
            log["psif"].corpusSize = corpusSize;

            ///*
            // * PsiPost
            // */
            log["psip"].path = outdir + "\\psip";
            fs::remove_all(log["psip"].path);
            q::IndexBatchBuilder<q::PsiPostBuilder> psipBuilder(psipParams);
            alex::io::filesystem::makeDirectory(log["psip"].path);
            timer.reset();
            log["psip"].numIndexes = psipBuilder.build(log["psip"].path, indir, depth, pattern, concurrency, verbose);
            log["psip"].lag = timer.elapsed();
            log["psip"].label = "psip";
            log["psip"].indexSize = alex::io::filesystem::getDirectorySize(log["psip"].path, ".*", depth);
            log["psip"].corpusSize = corpusSize;

            ///*
            // * BsiBlock
            // */
            log["bsib"].path = outdir + "\\bsib";
            fs::remove_all(log["bsib"].path);
            q::IndexBatchBuilder<q::BsiBlockBuilder> bsibBuilder(bsibParams);
            alex::io::filesystem::makeDirectory(log["bsib"].path);
            timer.reset();
            log["bsib"].numIndexes = psibBuilder.build(log["bsib"].path, indir, depth, pattern, concurrency, verbose);
            log["bsib"].lag = timer.elapsed();
            log["bsib"].label = "bsib";
            log["bsib"].indexSize = alex::io::filesystem::getDirectorySize(log["bsib"].path, ".*", depth);
            log["bsib"].corpusSize = corpusSize;

            /*
             *PsiMin
             */
            log["psim"].path = outdir + "\\psim";
            fs::remove_all(log["psim"].path);
            q::IndexBatchBuilder<q::PsiMinBuilder> psimBuilder(psimParams);
            alex::io::filesystem::makeDirectory(log["psim"].path);
            timer.reset();
            log["psim"].numIndexes = psimBuilder.build(log["psim"].path, indir, depth, pattern, concurrency, verbose);
            log["psim"].lag = timer.elapsed();
            log["psim"].label = "psim";
            log["psim"].indexSize = alex::io::filesystem::getDirectorySize(log["psim"].path, ".*", depth);
            log["psim"].corpusSize = corpusSize;

            alex::index::SecureIndexDb db;
            for (auto x : types)
            {
                timer.reset();
                db.read(log[x].path, pattern, depth);
                auto elapsed = timer.elapsed();
                log[x].loadLag = elapsed;
                db.clear();
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
            std::cout << "Error: Unknown cause." << std::endl;
        }

        return log;
    }

    template <class Builder>
    ConstructionResults SecureIndexTestMaker(
        const std::string& indir,
        const std::string& outdir,
        const std::string& pattern,
        typename Builder::BuilderParams params,
        const std::string& type,
        size_t depth = 1,
        bool verbose = false,
        bool concurrency = true,
        bool overrideExisting = false)
    {
        namespace fs = boost::filesystem;
        ConstructionResults log;
        namespace q = alex::index;
        alex::testing::Timer timer;

        try
        {
            q::IndexBatchBuilder<Builder> idxBuilder(params);
            log.path = outdir + "\\" + type;

            if (boost::filesystem::exists(log.path))
            {
                if (overrideExisting)
                {
                    std::cout << "\t* Removing: " << log.path << std::endl;
                    fs::remove_all(log.path);
                }
                else
                    throw std::exception(("Directory Already Exists: " + log.path).c_str());
            }
            alex::io::filesystem::makeDirectory(log.path);

            timer.reset();
            size_t numIndexes = idxBuilder.build(log.path, indir, depth, pattern, concurrency, verbose);
            auto elapsed = timer.elapsed();
            log.numIndexes = numIndexes;
            log.lag = elapsed;
            log.label = type;
            log.indexSize = alex::io::filesystem::getDirectorySize(log.path, ".*", depth);

            if (type == "canonical")
            {
                alex::index::InvertedIndexDb db;
                timer.reset();
                log.numIndexes = db.read(log.path, pattern, depth);
                elapsed = timer.elapsed();
                log.loadLag = elapsed;
            }
            else
            {
                alex::index::SecureIndexDb db;
                timer.reset();
                log.numIndexes = db.read(log.path, pattern, depth);
                elapsed = timer.elapsed();
                log.loadLag = elapsed;
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

        return log;
    }
}}