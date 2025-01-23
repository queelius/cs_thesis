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
            outs << "\t index_size\t" << indexSize << endl;
            outs << "\t build_time\t" << lag.count() << endl;
            outs << "\t    indexed\t" << numIndexes << endl;
            outs << "\t       path\t" << path << endl;
            outs << "\t  load_time\t" << loadLag.count() << endl;
        };
    };

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
        bool overrideExisting = false,
        unsigned int loadTrials = 1)
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
                db.read(log.path, pattern, depth);
                elapsed = timer.elapsed();
                log.loadLag = elapsed;
            }
            else
            {
                log.loadLag = 0;
                for (unsigned int trial = 0; trial < loadTrials; ++trial)
                {
                    alex::index::SecureIndexDb db;
                    timer.reset();
                    db.read(log.path, pattern, depth);
                    elapsed = timer.elapsed();
                    log.loadLag += elapsed;
                }
                log.loadLag /= loadTrials;
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