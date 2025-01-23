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
        std::string indir;
        std::string outdir;
        std::string label;
        std::string pattern = ".*";
        std::string stopWordsFile;
        size_t depth = std::numeric_limits<int>::max();
        bool verbose = alex::globals::default_verbose;
        bool stemming = alex::globals::default_stemming;
        double falsePositiveRate = alex::globals::default_fp_rate;
        double junkFraction = 1 - alex::globals::default_load_factor;
        double freqError = 0;
        std::vector<std::string> secrets = alex::globals::default_secrets;
        std::unordered_set<std::string> stopWords = alex::globals::default_stop_words;
        size_t locationUncertainty = alex::globals::default_words_per_block;
        std::string configFile;
        bool concurrency = true;

        alex::index::PsiBlockBuilder::BuilderParams psibParams;
        alex::index::PsiFreqBuilder::BuilderParams psifParams;
        alex::index::PsiPostBuilder::BuilderParams psipParams;
        alex::index::PsiMinBuilder::BuilderParams psimParams;
        alex::index::BsiBlockBuilder::BuilderParams bsibParams;
        alex::index::InvertedIndexBuilder::BuilderParams iidxParams;

        int psibAlgo, psifAlgo, psimAlgo, psipAlgo;
        psibAlgo = psifAlgo = psimAlgo = psipAlgo = alex::globals::default_ph;
        std::vector<std::string> types = {"canonical", "psib", "psif", "psip", "psim", "bsib"};

        po::options_description desc("options");
        desc.add_options()
            ("help,h", "help message")
            ("config,c", po::value<string>(&configFile), "load a config file")
            ("secrets,s", po::value<vector<string>>(&secrets)->multitoken(), "secrets to construct hidden terms with for query obfuscation (histogram analysis)")
            ("recursion,r", po::value<size_t>(&depth)->default_value(std::numeric_limits<size_t>::max()), "depth of indir recursion")
            ("verbose,v", po::value<bool>(&verbose)->default_value(verbose), "verbose mode")
            ("stemming", po::value<bool>(&stemming)->default_value(stemming), "porter2stemmer")
            ("pattern,p",  po::value<string>(&pattern)->default_value(pattern), "file pattern to index")
            ("label,l",  po::value<string>(&label), "label of the construction experiment")
            ("outdir,o",  po::value<string>(&outdir)->default_value("."), "base output dir for secure indexes, e.g., \"outdir\\psib\"")
            ("indir,i",  po::value<string>(&indir)->required(), "directories/files to index")
            ("concurrency", po::value<bool>(&concurrency)->default_value(concurrency), "whether to build indexes concurrently")
            ("stop_words", po::value<string>(&stopWordsFile), "stop words file")
            ("types,t", po::value<std::vector<std::string>>(&types)->multitoken(), "types to make: canonical psib psif psip psim bsib")
            ("false_positive_rate,f", po::value<double>(&falsePositiveRate)->default_value(falsePositiveRate), "the false positive rate")
            ("location_uncertainty,u", po::value<size_t>(&locationUncertainty)->default_value(locationUncertainty), "range of uncertainty for token location -- e.g., in PsiBlock this is block size, in PsiFreq this is entire document")
            ("junk_fraction,j", po::value<double>(&junkFraction)->default_value(junkFraction), "what fraction of the secure index consists of indexes for junk/fake terms for index obfuscation")
            ("freq_error", po::value<double>(&freqError)->default_value(freqError), "how much error (psif, psip) allowed for frequency data")

            // psib
            ("psib.block_size", po::value<uint32_t>(&psibParams.wordsPerBlock), "the preferred words/block")
            ("psib.max_blocks", po::value<uint32_t>(&psibParams.maxBlocks), "the maximum number of blocks/doc")
            ("psib.fp_bits", po::value<uint32_t>(&psibParams.falsePositiveBits), "the false positive rate--1/2^bits")
            ("psib.algo", po::value<int>(&psibAlgo)->default_value(psibAlgo)->default_value((int)alex::globals::default_ph), "perfect hash algorithm")
            ("psib.load_factor", po::value<double>(&psibParams.loadFactor), "what fraction of indexes is not junk")
            ("psip.freq_error", po::value<double>(&psipParams.freqError), "up to percent error for term frequency")

            // psif
            ("psif.max_freq", po::value<uint32_t>(&psifParams.maxFreq)->default_value(std::numeric_limits<uint32_t>::max()), "maximum frequency")
            ("psif.fp_bits", po::value<uint32_t>(&psifParams.falsePositiveBits), "the false positive rate--1/2^bits")
            ("psif.algo", po::value<int>(&psifAlgo)->default_value(psifAlgo)->default_value((int)alex::globals::default_ph), "perfect hash algorithm")
            ("psif.load_factor", po::value<double>(&psifParams.loadFactor), "what fraction of indexes is not junk")
            ("psif.freq_error", po::value<double>(&psifParams.freqError), "up to percent error for term frequency")

            // psip
            ("psip.offset_radius", po::value<uint32_t>(&psipParams.offsetRadius), "offset radius")
            ("psip.fp_bits", po::value<uint32_t>(&psipParams.falsePositiveBits), "the false positive rate--1/2^bits")
            ("psip.algo", po::value<int>(&psipAlgo)->default_value(psipAlgo)->default_value((int)alex::globals::default_ph), "perfect hash algorithm")
            ("psip.load_factor", po::value<double>(&psipParams.loadFactor), "what fraction of indexes is not junk")

            // psim
            ("psim.max_min_pair_dist", po::value<uint32_t>(&psimParams.maxMinPairMaxDist)->default_value(alex::globals::max_min_pair_dist), "maximum minimum pair distance to track")
            ("psim.fp_bits", po::value<uint32_t>(&psimParams.falsePositiveBits), "the false positive rate--1/2^bits")
            ("psim.load_factor", po::value<double>(&psimParams.loadFactor), "specifies amount of noise (fake junk terms) to add to psifreq index to obfuscate")
            ("psim.max_freq", po::value<uint32_t>(&psimParams.maxFreq)->default_value(std::numeric_limits<uint32_t>::max()), "maximum frequency")
            ("psim.algo", po::value<int>(&psimAlgo)->default_value(psimAlgo)->default_value((int)alex::globals::default_ph), "perfect hash algorithm for psim")

            // bsib
            ("bsib.block_size", po::value<uint32_t>(&bsibParams.wordsPerBlock), "the preferred words/block")
            ("bsib.max_blocks", po::value<uint32_t>(&bsibParams.maxBlocks), "the maximum number of blocks/doc")
            ("bsib.fp_rate", po::value<double>(&bsibParams.falsePositiveRate), "the false positive rate")
            ("bsib.noise_factor", po::value<double>(&bsibParams.noiseFactor), "fraction of junk terms")
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

        size_t corpusSize = alex::io::filesystem::getDirectorySize(indir, ".*", depth);
        std::ofstream log("maker.log");
        log << "label " << label << std::endl;
        log << "corpus_size " << corpusSize << std::endl;
        log << "false_positive_rate " << falsePositiveRate << std::endl;
        log << "location_uncertainty " << locationUncertainty << std::endl;
        log << "junk_fraction " << junkFraction << std::endl;
        log << "freq_error " << freqError << std::endl;
        log << "stemming " << stemming << std::endl;
        log << "stop_words " << stopWords.size() << std::endl;
        log << "secrets " << secrets.size() << std::endl;
        for (auto secret : secrets)
        {
            log << "\t" << secret << endl;
        }

        for (auto type : types)
        {
            try
            {
                alex::testing::ConstructionResults results;
                type = alex::str::trim(alex::str::lower(type));
                if (type == "psib")
                {
                    psibParams.ignoreWords = stopWords;
                    psibParams.secrets = secrets;
                    psibParams.stemming = stemming;
                    psibParams.verbose = verbose;
                    psibParams.algo = (cmph_cpp::CMPH_ALGO)psibAlgo;

                    if (vm.count("psib.block_size") == 0)
                        psibParams.wordsPerBlock = locationUncertainty;
                    if (vm.count("psib.fp_bits") == 0)
                        psibParams.falsePositiveRate(falsePositiveRate);
                    if (vm.count("psib.load_factor") == 0)
                        psibParams.loadFactor = 1.0 - junkFraction;

                    auto file = std::ofstream("maker_" + type + ".log");

                    file << "false_positive_rate " << (1.0 / (double)(1 << psibParams.falsePositiveBits)) << std::endl;
                    file << "load_factor " << psibParams.loadFactor << std::endl;
                    file << "max_blocks " << psibParams.maxBlocks << std::endl;
                    file << "block_size " << psibParams.wordsPerBlock << std::endl;
                    file << "algo " << (int)psibParams.algo << std::endl;
                    results = alex::testing::SecureIndexTestMaker<alex::index::PsiBlockBuilder>
                        (indir, outdir, pattern, psibParams, type, depth, verbose, concurrency, true);
                    results.print(file);
                }
                if (type == "psif")
                {
                    psifParams.ignoreWords = stopWords;
                    psifParams.secrets = secrets;
                    psifParams.stemming = stemming;
                    psifParams.verbose = verbose;
                    psifParams.algo = (cmph_cpp::CMPH_ALGO)psifAlgo;

                    if (vm.count("psif.fp_bits") == 0)
                        psifParams.falsePositiveRate(falsePositiveRate);
                    if (vm.count("psif.load_factor") == 0)
                        psifParams.loadFactor = 1.0 - junkFraction;
                    if (vm.count("psif.freq_error") == 0)
                        psifParams.freqError = freqError;

                    // min location uncertainty is entire doc

                    auto file = std::ofstream("maker_" + type + ".log");
                    file << "false_positive_rate " << (1.0 / (double)(1 << psifParams.falsePositiveBits)) << std::endl;
                    file << "load_factor " << psifParams.loadFactor << std::endl;
                    file << "max_freq " << psifParams.maxFreq << std::endl;
                    file << "algo " << (int)psifParams.algo << std::endl;

                    results = alex::testing::SecureIndexTestMaker<alex::index::PsiFreqBuilder>
                        (indir, outdir, pattern, psifParams, type, depth, verbose, concurrency, true);
                    results.print(file);
                }
                if (type == "psip")
                {
                    psipParams.ignoreWords = stopWords;
                    psipParams.secrets = secrets;
                    psipParams.stemming = stemming;
                    psipParams.verbose = verbose;
                    psipParams.algo = (cmph_cpp::CMPH_ALGO)psipAlgo;

                    if (vm.count("psip.freq_error") == 0)
                        psipParams.freqError = freqError;
                    if (vm.count("psip.offset_radius") == 0)
                        psipParams.offsetRadius = (uint32_t)std::ceil(locationUncertainty / 2.0);
                    if (vm.count("psip.fp_bits") == 0)
                        psipParams.falsePositiveRate(falsePositiveRate);
                    if (vm.count("psip.load_factor") == 0)
                        psipParams.loadFactor = 1.0 - junkFraction;

                    auto file = std::ofstream("maker_" + type + ".log");
                    file << "false_positive_rate " << (1.0 / (double)(1 << psipParams.falsePositiveBits)) << std::endl;
                    file << "load_factor " << psipParams.loadFactor << std::endl;
                    file << "offset_radius " << psipParams.offsetRadius << std::endl;
                    file << "algo " << (int)psipParams.algo << std::endl;

                    results = alex::testing::SecureIndexTestMaker<alex::index::PsiPostBuilder>
                        (indir, outdir, pattern, psipParams, type, depth, verbose, concurrency, true);
                    results.print(file);
                }
                if (type == "psim")
                {
                    psimParams.ignoreWords = stopWords;
                    psimParams.secrets = secrets;
                    psimParams.stemming = stemming;
                    psimParams.verbose = verbose;
                    psimParams.algo = (cmph_cpp::CMPH_ALGO)psimAlgo;

                    if (vm.count("psim.fp_bits") == 0)
                        psimParams.falsePositiveRate(falsePositiveRate);
                    if (vm.count("psim.load_factor") == 0)
                        psimParams.loadFactor = 1.0 - junkFraction;

                    auto file = std::ofstream("maker_" + type + ".log");
                    file << "false_positive_rate " << (1.0 / (double)(1 << psimParams.falsePositiveBits)) << std::endl;
                    file << "load_factor " << psimParams.loadFactor << std::endl;
                    file << "max_freq " << psimParams.maxFreq << std::endl;
                    file << "algo " << (int)psimParams.algo << std::endl;
                    file << "max_min_pair_dst " << psimParams.maxMinPairMaxDist << std::endl;

                    results = alex::testing::SecureIndexTestMaker<alex::index::PsiMinBuilder>
                        (indir, outdir, pattern, psimParams, type, depth, verbose, false, true);
                    results.print(file);
                }
                if (type == "bsib")
                {
                    bsibParams.ignoreWords = stopWords;
                    bsibParams.secrets = secrets;
                    bsibParams.stemming = stemming;
                    bsibParams.verbose = verbose;

                    if (vm.count("psib.block_size") == 0)
                        bsibParams.wordsPerBlock = locationUncertainty;
                    if (vm.count("bsib.fp_rate") == 0)
                        bsibParams.falsePositiveRate = falsePositiveRate;
                    if (vm.count("bsib.noise_factor") == 0)
                        bsibParams.noiseFactor = junkFraction;

                    auto file = std::ofstream("maker_" + type + ".log");
                    file << "false_positive_rate " << bsibParams.falsePositiveRate << std::endl;
                    file << "block_size " << bsibParams.wordsPerBlock << std::endl;
                    file << "max_blocks " << bsibParams.maxBlocks << std::endl;
                    
                    results = alex::testing::SecureIndexTestMaker<alex::index::BsiBlockBuilder>
                        (indir, outdir, pattern, bsibParams, type, depth, verbose, concurrency, true);
                    results.print(file);
                }
                if (type == "canonical")
                {
                    iidxParams.ignoreWords = stopWords;
                    iidxParams.stemming = stemming;
                    iidxParams.verbose = verbose;
                    auto file = std::ofstream("maker_" + type + ".log");

                    results = alex::testing::SecureIndexTestMaker<alex::index::InvertedIndexBuilder>
                        (indir, outdir, pattern, iidxParams, type, depth, verbose, concurrency, true);
                    results.print(file);
                }
                
                results.print(log);
            }
            catch (const std::exception& e)
            {
                std::cout << "Error: " << e.what() << std::endl;
            }
        }
    }
    catch (const exception& e)
    {
        cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const char* e)
    {
        cerr << "Error: " << e << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        cerr << "Error: Unknown cause" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
