#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <random>
#include <algorithm>
#include <random>
#include <boost/random/discrete_distribution.hpp>
#include <boost/program_options.hpp>
using namespace std;
namespace po = boost::program_options;

const int SAMPLES_PER_RANDOM_RESTART = 500;
const int FAILED_SAMPLES_BEFORE_RESTART = 500;

std::random_device rd;
std::default_random_engine gen(rd());

double simulateAttackerObfuscations(int vocabSize, double probObfuscation, int numObfuscations, int historySize, int numSamples);
double simulateAttackerSecrets(int vocabSize, int numSecrets, int historySize, int numSamples);
std::vector<double> makeFreqTable(int vocabSize, double probOb);

std::vector<int> maxLogLikelihoodHillClimber(int numSamples, std::vector<int> localMax, const std::vector<int>& data, const std::vector<double>& logProb);
std::vector<int> maxLogLikelihood(int numSamples, std::vector<int> candidate, const std::vector<int>& data, const std::vector<double>& logProb);
double logLikelihood(const std::vector<int>& candidate, const std::vector<int>& data, const std::vector<double>& logProb);

int main(int ac, char* av[])
{
    try
    {
        int samplesPerTrial;
        int trials;
        double probObfuscation;
        int historySize;
        int vocabSize;
        int numObfuscations;
        int numSecrets;
        bool simulateObfuscations;

        po::options_description desc("options");
        desc.add_options()
            ("help", "help message")
            ("monte_carlo_samples", po::value<int>(&samplesPerTrial)->default_value(50000))
            ("trials", po::value<int>(&trials)->default_value(1))
            ("history_size", po::value<int>(&historySize))
            ("obfuscation_probability", po::value<double>(&probObfuscation)->default_value(0))
            ("vocabulary_size", po::value<int>(&vocabSize))
            ("num_obfuscations", po::value<int>(&numObfuscations)->default_value(0))
            ("simulate_obfuscations", po::value<bool>(&simulateObfuscations), "simulate obfuscation approach or (exclusive) simulate secrets approach")
            ("num_secrets", po::value<int>(&numSecrets)->default_value(1))
            ;

        po::variables_map vm;        
        po::store(po::parse_command_line(ac, av, desc), vm);

        if (vm.count("help"))
        {
            cout << desc << "\n";
            return EXIT_SUCCESS;;
        }
        po::notify(vm);

        if (simulateObfuscations) // simulate obfuscations approach
        {
            double totalCorrect = 0;
            for (int i = 0; i < trials; ++i)
                totalCorrect += simulateAttackerObfuscations(vocabSize, probObfuscation, numObfuscations, historySize, samplesPerTrial);

            double avgCorrect = totalCorrect / trials;
            std::cout << vocabSize << ","               // words
                        << numObfuscations << ","       // obfuscations
                        << 1 << ","                     // secrets
                        << trials << ","                // trials
                        << historySize << ","           // query term samples
                        << probObfuscation << ","       // probability of an obfuscation
                        << samplesPerTrial << ","       // how many mc samples per trial?
                        << avgCorrect                   // average correct over all trials
                        << std::endl;
        }
        else // simulate secrets approach
        {
            double totalCorrect = 0;
            for (int i = 0; i < trials; ++i)
                totalCorrect += simulateAttackerSecrets(vocabSize, numSecrets, historySize, samplesPerTrial);
            double avgCorrect = totalCorrect / trials;

            std::cout << vocabSize << ","            // words
                        << 0 << ","                 // obfuscations
                        << numSecrets << ","        // secrets
                        << trials << ","         // trials
                        << historySize << ","       // query term samples
                        << 0 << ","                 // probability of an obfuscation
                        << samplesPerTrial << ","      // how many mc samples per trial?
                        << avgCorrect               // average error over trials
                        << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (const char* e)
    {
        std::cout << e << std::endl;
    }
    catch (...)
    {
        std::cout << "Unknown error" << std::endl;
    }
}

std::vector<double> makeFreqTable(int vocabSize, double probOb)
{
    if (vocabSize <= 0 || probOb < 0)
        throw std::exception("Invalid Argument");

    std::vector<double> freqTable;

    freqTable.reserve(vocabSize + 1);
    double sum = 0;
    for (int i = 1; i <= vocabSize; ++i)
        sum += 1.0 / i;

    for (int i = 1; i <= vocabSize; ++i)
        freqTable.push_back((1 - probOb) / (i * sum));

    if (probOb != 0)
        freqTable.push_back(probOb);

    return freqTable;
}

inline double logLikelihood(const std::vector<int>& candidate, const std::vector<int>& data, const std::vector<double>& logProb)
{
    double logp = 0;
    for (const auto& d : data)
        logp += logProb[candidate[d]];
    return logp;
}

std::vector<int> maxLogLikelihoodHillClimber(int numSamples, std::vector<int> localMax, const std::vector<int>& data, const std::vector<double>& logProb)
{
    if (numSamples <= 0)
        throw std::exception("Invalid Argument");

    std::uniform_int_distribution<int> u(0, localMax.size() - 1);
    double maxLogp = logLikelihood(localMax, data, logProb);
    double localMaxLogp = maxLogp;
    std::vector<int> argMax = localMax;
    int failed = 0;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // work on a copy of the mapping function hidden term -> plain term
        auto candidateMap = localMax;

        // transform the mapping function slightly
        int swap1 = u(gen);
        int swap2 = u(gen);

        // if they map to the same thing (e.g., obfuscations or like terms with the same secret),
        // then don't consider it a good move. note that this is only true if a term is uniformly
        // mapped to one of its corresponding secrets. i am making this assumption here for the
        // sake of simplicity -- i may extend it if necessary
        while (candidateMap[swap1] == candidateMap[swap2])
        {
            swap1 = u(gen);
            swap2 = u(gen);
        }
        std::swap(candidateMap[swap1], candidateMap[swap2]);

        double logp = logLikelihood(candidateMap, data, logProb);
        if (logp > localMaxLogp)
        {
            // found a better neighbor
            localMaxLogp = logp;
            localMax = candidateMap;

            if (logp > maxLogp)
            {
                // this neighbor is a global max
                maxLogp = logp;
                argMax = candidateMap;
            }

            // reset failed attempts back to 0
            failed = 0;
        }
        else if (++failed == FAILED_SAMPLES_BEFORE_RESTART)
        {
            // restart after too many failed attempts at finding a better neighbor
            localMax = maxLogLikelihood(SAMPLES_PER_RANDOM_RESTART, std::move(localMax), data, logProb);
            localMaxLogp = logLikelihood(localMax, data, logProb);

            // add SAMPLES_PER_RANDOM_RESTART to sample, since maxLogLikelihood uses that many
            sample += SAMPLES_PER_RANDOM_RESTART - 1;

            // reset failed attempts back to 0
            failed = 0;
        }
    }

    return argMax;
}

std::vector<int> maxLogLikelihood(int numSamples, std::vector<int> candidate, const std::vector<int>& data, const std::vector<double>& logProb)
{
    if (numSamples <= 0)
        throw std::exception("Invalid Argument");

    std::vector<int> argmax = candidate;
    double maxLogProb = logLikelihood(argmax, data, logProb);

    for (int i = 0; i < numSamples; ++i)
    {
        double logp = 0;
        for (const auto& d : data)
        {
            logp += logProb[candidate[d]];
            if (logp < maxLogProb)
                break;
        }

        if (logp > maxLogProb)
        {
            maxLogProb = logp;
            argmax = candidate;
        }

        std::shuffle(candidate.begin(), candidate.end(), gen);
    }

    return argmax;
}

double simulateAttackerObfuscations(int vocabSize, double probObfuscation, int numObfuscations, int historySize, int numSamples)
{
    if (probObfuscation <= 0 || vocabSize <= 0 || numObfuscations <= 0 || historySize <= 0 || numSamples <= 0)
        throw std::exception("Invalid Argument");

    const auto& freqTable = makeFreqTable(vocabSize, probObfuscation);
    boost::random::discrete_distribution<> dist(freqTable.begin(), freqTable.end());
    std::vector<int> hiddenToPlainMap;

    for (int i = 0; i < vocabSize; ++i)
        hiddenToPlainMap.push_back(i);

    // the hidden obfuscations all map to the same plain term -- class OBFUSCATION
    for (int i = 0; i < numObfuscations; ++i)
        hiddenToPlainMap.push_back(vocabSize);

    std::shuffle(hiddenToPlainMap.begin(), hiddenToPlainMap.end(), gen);
    auto candidateMap = hiddenToPlainMap;
    std::shuffle(candidateMap.begin(), candidateMap.end(), gen);

    std::vector<int> plainToHiddenMap(vocabSize);
    std::vector<int> obfuscations;
    obfuscations.reserve(numObfuscations);

    for (int i = 0; i < vocabSize + numObfuscations; ++i)
    {
        int x = hiddenToPlainMap[i];
        if (x != vocabSize)
            plainToHiddenMap[x] = i;
        else
            obfuscations.push_back(i);
    }

    /*
    std::cout << "plain to hidden" << std::endl;
    for (int i = 0; i < plainToHiddenMap.size(); ++i)
        std::cout << "\t" << i <<" -> " << plainToHiddenMap[i] << std::endl;

    std::cout << "hidden to plain" << std::endl;
    for (int i = 0; i < hiddenToPlainMap.size(); ++i)
        std::cout << "\t" << i <<" -> " << hiddenToPlainMap[i] << std::endl;

    std::cout << "p(plain)" << std::endl;
    for (int i = 0; i < dist.probabilities().size(); ++i)
        std::cout << "\t" << i <<" -> " << dist.probabilities()[i] << std::endl;

    system("pause");
    */

    std::uniform_int_distribution<int> ob(0, numObfuscations - 1);
    std::vector<int> data;
    for (int i = 0; i < historySize; ++i)
    {
        int x = dist(gen);
        if (x == vocabSize)
            data.push_back(obfuscations[ob(gen)]);
        else
            data.push_back(plainToHiddenMap[x]);
    }

    /*
    std::vector<double> counts(vocabSize + 1);
    std::cout << "data" << std::endl;
    for (int i = 0; i < data.size(); ++i)
    {
        int x = hiddenToPlainMap[data[i]];
        if (x != vocabSize)
            counts[x]++;
        else
            counts[vocabSize]++;
    }

    std::cout << "histogram" << std::endl;
    for (int i = 0; i < counts.size(); ++i)
    {
        std::cout << i << " -> " << counts[i] / (double)data.size() << std::endl;
    }
    system("pause");
    */

    auto logProb = dist.probabilities();
    for (double& p : logProb)
        p = std::log(p);

    auto argmax = maxLogLikelihoodHillClimber(numSamples,
                                   maxLogLikelihood(SAMPLES_PER_RANDOM_RESTART, std::move(candidateMap), data, logProb),
                                   data, logProb);

    int correct = 0;
    for (int i = 0; i < vocabSize + numObfuscations; ++i)
    {
        if (hiddenToPlainMap[i] != vocabSize && hiddenToPlainMap[i] == argmax[i])
            ++correct;
    }

    return (double)correct / vocabSize;
}

double simulateAttackerSecrets(int vocabSize, int numSecrets, int historySize, int numSamples)
{
    if (vocabSize <= 0 || numSecrets < 1 || numSamples <= 0)
        throw std::exception("Invalid Arguments");

    const std::vector<double> freqTable = makeFreqTable(vocabSize, 0);
    boost::random::discrete_distribution<> dist(freqTable.begin(), freqTable.end());
    const int nn = vocabSize * numSecrets;

    std::vector<int> hiddenToPlainMap;
    std::vector<int> candidateMap;
    hiddenToPlainMap.reserve(nn);
    candidateMap.reserve(nn);

    for (int i = 0; i < vocabSize; ++i)
    {
        for (int j = 0; j < numSecrets; ++j)
        {
            hiddenToPlainMap.push_back(i);
            candidateMap.push_back(i);
        }
    }
    std::shuffle(hiddenToPlainMap.begin(), hiddenToPlainMap.end(), gen);

    std::vector<std::vector<int>> plainToHidden(vocabSize);
    for (int hidden = 0; hidden < nn; ++hidden)
    {
        int plain = hiddenToPlainMap[hidden];
        plainToHidden[plain].push_back(hidden);
    }

    std::vector<int> data;
    for (int i = 0; i < historySize; ++i)
    {
        int plain = dist(gen);
        auto hiddenTerms = plainToHidden[plain];
        data.push_back(hiddenTerms[rd() % numSecrets]);
    }

    auto logProb = dist.probabilities();
    for (auto& p : logProb)
        p = std::log(p);

    auto argmax = maxLogLikelihoodHillClimber(numSamples,
                                   maxLogLikelihood(SAMPLES_PER_RANDOM_RESTART, candidateMap, data, logProb),
                                   data, logProb);

    int correct = 0;
    for (int i = 0; i < nn; ++i)
    {
        if (argmax[i] == hiddenToPlainMap[i])
            ++correct;
    }

    return (double)correct / nn;
}
