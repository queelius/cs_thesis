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

std::random_device rd;
std::default_random_engine gen(rd());

typedef std::vector<std::vector<int>> MatrixInt;

MatrixInt getMinPwDistances(const std::vector<std::vector<uint32_t>>& p);
MatrixInt makeMatrixInt(int n, int value);
double triangular(double a, double b, double c);

double blockAveragePair(int blockSize, int docSize, int trials = 1)
{
    std::vector<double> totals(trials);
    for (int trial = 0; trial < trials; ++trial)
    {
        totals[trial] = 0;
        for (int i = 0; i < docSize; ++i)
        {
            int x1 = rd() % docSize;
            int x2 = rd() % docSize;

            int blockX1 = x1 / blockSize;
            int blockX2 = x2 / blockSize;

            int x1p = blockX1 * blockSize + rd() % blockSize;
            int x2p = blockX2 * blockSize + rd() % blockSize;

            totals[trial] += abs((abs(x1p - x2p) - abs(x1 - x2)));
        }
        totals[trial] /= docSize;
    }

    int total = 0;
    for (auto t : totals)
        total += t;
    return total / trials;
}

double psipUniformAveragePair(int radiusSize, int docSize, int trials = 1)
{
    std::vector<double> totals(trials);
    for (int trial = 0; trial < trials; ++trial)
    {
        totals[trial] = 0;
        for (int i = 0; i < docSize; ++i)
        {
            int x1 = rd() % docSize;
            int x2 = rd() % docSize;

            int a1 = std::max(0, x1 - radiusSize);
            int b1 = std::min(docSize-1, x1 + radiusSize);
            int x1p = a1 + rd() % (b1 - a1);

            int a2 = std::max(0, x2 - radiusSize);
            int b2 = std::min(docSize-1, x2 + radiusSize);
            int x2p = a2 + rd() % (b2 - a2);

            totals[trial] += abs((abs(x1p - x2p) - abs(x1 - x2)));
        }
        totals[trial] /= docSize;
    }

    int total = 0;
    for (auto t : totals)
        total += t;
    return total / trials;
}

double psipTriangularAveragePair(int radiusSize, int docSize, int trials = 1)
{
    std::vector<double> totals(trials);
    for (int trial = 0; trial < trials; ++trial)
    {
        totals[trial] = 0;
        for (int i = 0; i < docSize; ++i)
        {
            int x1 = rd() % docSize;
            int x2 = rd() % docSize;

            int x1p = (int)std::round(triangular(std::max(0, x1 - radiusSize), std::min(docSize-1, x1 + radiusSize), x1));
            int x2p = (int)std::round(triangular(std::max(0, x2 - radiusSize), std::min(docSize-1, x2 + radiusSize), x2));

            totals[trial] += abs((abs(x1p - x2p) - abs(x1 - x2)));
        }
        totals[trial] /= docSize;
    }

    int total = 0;
    for (auto t : totals)
        total += t;
    return total / trials;
}

double psipUniformAverageLocationError(int radiusSize, int docSize, int trials = 1)
{
    std::vector<double> totals(trials);
    for (int trial = 0; trial < trials; ++trial)
    {
        totals[trial] = 0;

        for (int i = 0; i < docSize; ++i)
        {
            int a = std::max(0, i - radiusSize);
            int b = std::min(docSize-1, i + radiusSize);
            int xp = a + rd() % (b - a);

            totals[trial] += abs(xp - i);
        }
        totals[trial] /= docSize;
    }

    int total = 0;
    for (auto t : totals)
        total += t;
    return total / trials;
}

// block-based index (psib, bsib)
double blockUniformAverageLocationError(int blockSize, int docSize, int trials = 1)
{
    std::vector<double> totals(trials);
    for (int trial = 0; trial < trials; ++trial)
    {
        totals[trial] = 0;

        for (int i = 0; i < docSize; ++i)
        {
            int block = i / blockSize;
            int block_x = rd() % blockSize;

            totals[trial] += abs(block * blockSize + block_x - i);
        }
        totals[trial] /= docSize;
    }

    int total = 0;
    for (auto t : totals)
        total += t;
    return total / trials;
}

double psipTriangularAverageLocationError(int radiusSize, int docSize, int trials = 1)
{
    std::vector<double> totals(trials);
    for (int trial = 0; trial < trials; ++trial)
    {
        totals[trial] = 0;
        for (int i = 0; i < docSize; ++i)
        {
            double a = std::max(0, i - radiusSize);
            double b = std::min(docSize-1, i + radiusSize);
            double c = i;
            int xp = (int)std::round(triangular(a, b, c));

            totals[trial] += abs(xp - i);
        }
        totals[trial] /= docSize;
    }

    int total = 0;
    for (auto t : totals)
        total += t;
    return total / trials;
}

double simulateAttackerObfuscations(int numWords, double probObfuscation, int numObfuscations, int numQueryTerms, bool obDistKnown);
double simulateAttackerMCObfuscations(int numWords, double probObfuscation, int numObfuscations, int numQueryTerms, bool obDistKnown, int numSamples);
double simulateAttackerSecrets(int numWords, int numSecrets, int numQueryTerms);
double simulateAttackerMCSecrets(int numWords, int numSecrets, int numQueryTerms, int numSamples);

std::vector<double> makeFreqTableOb(int numWords, int numOb, double probOb);

std::vector<int> maxLogLikelihoodHillClimber(int numSamples, std::vector<int> localArgmax, const std::vector<int>& data, int numWords, bool obDistKnown, double logu, const std::vector<double>& logProb, const std::vector<int> real);
//std::vector<int> maxLogLikelihoodHillClimberSecrets(int numTrials, std::vector<int> localArgmax, const std::vector<int>& data, const std::vector<double>& logProb);
std::vector<int> maxLogLikelihoodHillClimberSecrets(int numSamples, std::vector<int> localArgmax, const std::vector<int>& data, const std::vector<double>& logProb, const std::vector<int> real);
std::vector<int> maxLogLikelihood(int numSamples, std::vector<int> candidate, const std::vector<int>& data, int numWords, bool obDistKnown, double logu, const std::vector<double>& logProb);
std::vector<int> maxLogLikelihood(int numSamples, std::vector<int> candidate, const std::vector<int>& data, const std::vector<double>& logProb);
double logLikelihood(const std::vector<int>& candidate, const std::vector<int>& data, int numWords, bool obDistKnown, double logu, const std::vector<double>& logProb);
double logLikelihood(const std::vector<int>& candidate, const std::vector<int>& data, const std::vector<double>& logProb);

int main(int ac, char* av[])
{
    try
    {
        int numMCSamples;
        int numTrials;
        double probObfuscation;
        int wordHistory;
        int numWords;
        int obfuscations;
        bool obfuscationDistKnown;
        int numSecrets;
        bool simulateObfuscations;
        bool useMC;

        po::options_description desc("options");
        desc.add_options()
            ("help", "help message")
            ("monte_carlo_samples", po::value<int>(&numMCSamples)->default_value(1000))
            ("trials", po::value<int>(&numTrials)->default_value(10))
            ("history_size", po::value<int>(&wordHistory))
            ("obfuscation_probability", po::value<double>(&probObfuscation)->default_value(0))
            ("vocabulary_size", po::value<int>(&numWords))
            ("num_obfuscations", po::value<int>(&obfuscations)->default_value(0))
            ("known_obfuscation_distribution", po::value<bool>(&obfuscationDistKnown)->default_value(false))
            ("simulate_obfuscations", po::value<bool>(&simulateObfuscations), "simulate obfuscation approach or (exclusive) simulate secrets approach")
            ("num_secrets", po::value<int>(&numSecrets)->default_value(1))
            ("use_monte_carlo", po::value<bool>(&useMC)->default_value(true))
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
            if (useMC)
            {
                for (int i = 0; i < numTrials; ++i)
                    totalCorrect += simulateAttackerMCObfuscations(numWords, probObfuscation, obfuscations, wordHistory, obfuscationDistKnown, numMCSamples);
            }
            else
            {
                for (int i = 0; i < numTrials; ++i)
                    totalCorrect += simulateAttackerObfuscations(numWords, probObfuscation, obfuscations, wordHistory, obfuscationDistKnown);
                numMCSamples = -1;
            }

            double avgCorrect = totalCorrect / numTrials;
            std::cout << numWords << ","                // words
                        << obfuscations << ","          // obfuscations
                        << 1 << ","                     // secrets
                        << numTrials << ","             // trials
                        << wordHistory << ","           // query term samples
                        << probObfuscation << ","       // probability of an obfuscation
                        << obfuscationDistKnown << ","  // obfuscation distribution unknown
                        << useMC << ","                 // using monte carlo sim -- not really mc though since using uniform sampling for input vars, argh
                        << numMCSamples << ","          // how many mc samples per trial?
                        << avgCorrect                   // average correct over all trials
                        << std::endl;
        }
        else // simulate secrets approach
        {
            double totalCorrect = 0;
            if (useMC)
            {
                for (int i = 0; i < numTrials; ++i)
                    totalCorrect += simulateAttackerMCSecrets(numWords, numSecrets, wordHistory, numMCSamples);
            }
            else
            {
                for (int i = 0; i < numTrials; ++i)
                    totalCorrect += simulateAttackerSecrets(numWords, numSecrets, wordHistory);
                numMCSamples = -1;
            }
            double avgCorrect = totalCorrect / numTrials;

            std::cout << numWords << ","            // words
                        << 0 << ","                 // obfuscations
                        << numSecrets << ","        // secrets
                        << numTrials << ","         // trials
                        << wordHistory << ","       // query term samples
                        << 0 << ","                 // probability of an obfuscation
                        << -1 << ","                // obfuscation distribution known
                        << useMC << ","             // using monte carlo sim -- not really mc though since using uniform sampling for input vars, argh
                        << numMCSamples << ","      // how many mc samples per trial?
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

std::vector<double> getZipfFrequencyTable(int n)
{
    std::vector<double> table;

    double p = 1;
    for (int i = 0; i < n; ++i)
    {
        table.push_back(p);
        p /= (i + 1);
    }
}

std::vector<double> makeFreqTableOb(int numWords, int numOb, double probOb)
{
    if (probOb <= 0 || numWords <= 0 || numOb <= 0)
        throw std::exception("Invalid Argument");

    std::vector<double> freqTable;
    double sum = 0;

    for (int i = 0; i < numWords; ++i)
    {
        double freq = 1 + (rd() % (100 * numWords));
        freqTable.push_back(freq);
        sum += freq;
    }

    std::vector<double> obFreqTable;
    double sumOb = 0;
    for (int i = 0; i < numOb; ++i)
    {
        double freq = 1 + (rd() % (int)(100 / probOb));
        obFreqTable.push_back(freq);
        sumOb += freq;
    }

    /*
    for (int x = 0; x < obFreqTable.size(); ++x)
    {
        std::cout << "\t" << obFreqTable[x] << std::endl;
    }
    system("pause");
    */

    for (auto& x : obFreqTable)
        x /= sumOb;

    /*
    for (int x = 0; x < obFreqTable.size(); ++x)
    {
        std::cout << "\t" << obFreqTable[x] << std::endl;
    }
    system("pause");
    */

    double freqOb = probOb * sum / (1 - probOb);

    for (int i = 0; i < numOb; ++i)
        freqTable.push_back(freqOb * obFreqTable[i]);

    /*
    for (int x = 0; x < freqTable.size(); ++x)
    {
        std::cout << "\t" << freqTable[x] << std::endl;
    }
    system("pause");
    */

    return freqTable;
}

std::vector<double> makeFreqTableOb2(int numWords, double probOb)
{
    if (probOb <= 0 || numWords <= 0)
        throw std::exception("Invalid Argument");

    std::vector<double> freqTable;
    double sum = 0;

    for (int i = 0; i < numWords; ++i)
    {
        double freq = 1 + (rd() % (100 * numWords));
        freqTable.push_back(freq);
        sum += freq;
    }

    double freqOb = probOb * sum / (1 - probOb);
    freqTable.push_back(freqOb);

    return freqTable;
}

double simulateAttackerObfuscations(int numWords, double probObfuscation, int numObfuscations, int numQueryTerms, bool obDistKnown)
{
    const auto& freqTable = makeFreqTableOb(numWords, numObfuscations, probObfuscation);
    boost::random::discrete_distribution<> dist(freqTable.begin(), freqTable.end());

    std::vector<int> hiddenToPlainMap;
    const int nn = numWords + numObfuscations;
    for (int i = 0; i < nn; ++i)
    {
        hiddenToPlainMap.push_back(i);
    }
    std::shuffle(hiddenToPlainMap.begin(), hiddenToPlainMap.end(), gen);
    auto hCandidate = hiddenToPlainMap;

    std::vector<int> plainToHidden(numWords + numObfuscations);
    for (int i = 0; i < hiddenToPlainMap.size(); ++i)
    {
        plainToHidden[hiddenToPlainMap[i]] = i;
    }
    
    std::vector<int> hTerms;
    for (int i = 0; i < numQueryTerms; ++i)
    {
        hTerms.push_back(plainToHidden[dist(gen)]);
    }

    double maxLogProb = -std::numeric_limits<double>::infinity();
    std::vector<int> argmax;
    auto logProb = dist.probabilities();
    for (auto& p : logProb)
        p = std::log(p);
    const double logu = std::log(probObfuscation / numObfuscations);

    do
    {
        double logp = 0;
        for (const auto& ht : hTerms)
        {
            int x = hCandidate[ht];
            logp += ((obDistKnown || x < numWords) ? logProb[x] : logu);

            if (logp < maxLogProb)
                break;
        }

        if (logp > maxLogProb)
        {
            maxLogProb = logp;
            argmax = hCandidate;
        }       
    } while (std::next_permutation(hCandidate.begin(), hCandidate.end()));

    int correct = 0;
    for (int i = 0; i < nn; ++i)
    {
        if (hiddenToPlainMap[i] < numWords && hiddenToPlainMap[i] == argmax[i])
            ++correct;
    }

    return (double)correct / nn;
}

double simulateAttackerSecrets(int numWords, int numSecrets, int numQueryTerms)
{
    std::vector<double> freqTable;
    for (int i = 0; i < numWords; ++i)
        freqTable.push_back(rd() % (100 * numWords));
    boost::random::discrete_distribution<> dist(freqTable.begin(), freqTable.end());

    const int nn = numWords * numSecrets;
    std::vector<int> hiddenToPlainMap;
    hiddenToPlainMap.reserve(nn);

    for (int i = 0; i < numWords; ++i)
    {
        for (int j = 0; j < numSecrets; ++j)
            hiddenToPlainMap.push_back(i);
    }
    std::shuffle(hiddenToPlainMap.begin(), hiddenToPlainMap.end(), gen);
    auto hCandidate = hiddenToPlainMap;
    std::shuffle(hCandidate.begin(), hCandidate.end(), gen);

    std::vector<std::vector<int>> plainToHidden(numWords);
    for (int i = 0; i < nn; ++i)
    {
        plainToHidden[hiddenToPlainMap[i]].push_back(i);
    }
    
    std::vector<int> hTerms;
    for (int i = 0; i < numQueryTerms; ++i)
    {
        auto hiddenTerms = plainToHidden[dist(gen)];
        hTerms.push_back(hiddenTerms[rd() % numSecrets]);
    }

    double maxLogProb = -std::numeric_limits<double>::infinity();
    std::vector<int> argmax = hCandidate;
    auto logProb = dist.probabilities();
    for (auto& p : logProb)
        p = std::log(p);

    do
    {
        double logp = 0;
        for (const auto& d : hTerms)
        {
            logp += logProb[hCandidate[d]];
            if (logp < maxLogProb)
                break;
        }

        if (logp > maxLogProb)
        {
            maxLogProb = logp;
            argmax = hCandidate;
        }       
    } while (std::next_permutation(hCandidate.begin(), hCandidate.end()));

    int correct = 0;
    for (int i = 0; i < nn; ++i)
    {
        if (argmax[i] == hiddenToPlainMap[i])
            ++correct;
    }

    return (double)correct / nn;
}

double logLikelihood(const std::vector<int>& candidate, const std::vector<int>& data,
                      int numWords, bool obDistKnown, double logu, const std::vector<double>& logProb)
{
    double logp = 0;
    for (const auto& d : data)
    {
        int x = candidate[d];
        logp += ((obDistKnown || x < numWords) ? logProb[x] : logu);
    }
    return logp;
}

double logLikelihood(const std::vector<int>& candidate, const std::vector<int>& data, const std::vector<double>& logProb)
{
    double logp = 0;
    for (const auto& d : data)
        logp += logProb[candidate[d]];
    return logp;
}

std::vector<int> maxLogLikelihoodHillClimber(int numTrials, std::vector<int> localArgmax, const std::vector<int>& data,
                      int numWords, bool obDistKnown, double logu, const std::vector<double>& logProb, const std::vector<int> real)
{
    std::uniform_int_distribution<int> u(0, localArgmax.size() - 1);
    double maxLogp = logLikelihood(localArgmax, data, numWords, obDistKnown, logu, logProb);
    double localMaxLogp = maxLogp;
    std::vector<int> argmax = localArgmax;
    
    int nn = 0;
    for (int i = 0; i < numTrials; ++i)
    {
        auto x = localArgmax;
        int swap1 = u(gen);
        int swap2 = u(gen);
        while (swap1 == swap2)
            swap2 = u(gen);

        int tmp = x[swap1];
        x[swap1] = x[swap2];
        x[swap2] = tmp;

        double logp = logLikelihood(x, data, numWords, obDistKnown, logu, logProb);

        if (logp > localMaxLogp)
        {
            localMaxLogp = logp;
            localArgmax = x;
            if (logp > maxLogp)
            {
                maxLogp = logp;
                argmax = x;
            }            
            nn = 0;
        }
        else if (++nn > 10 * localArgmax.size())
        {
            localArgmax = maxLogLikelihood(1000, std::move(localArgmax), data, numWords, obDistKnown, logu, logProb); 
            localMaxLogp = logLikelihood(localArgmax, data, numWords, obDistKnown, logu, logProb);
            nn = 0;
        }
    }
    return argmax;
}

std::vector<int> maxLogLikelihoodHillClimberSecrets(int numSamples, std::vector<int> localArgmax, const std::vector<int>& data,
                                                    const std::vector<double>& logProb, const std::vector<int> real)
{
    std::uniform_int_distribution<int> u(0, localArgmax.size() - 1);
    double maxLogp = logLikelihood(localArgmax, data, logProb);
    double localMaxLogp = maxLogp;
    std::vector<int> argmax = localArgmax;
    
    int nn = 0;
    for (int i = 0; i < numSamples; )
    {
        auto x = localArgmax;
        int swap1 = u(gen);
        int swap2 = u(gen);
        while (swap1 == swap2)
            swap2 = u(gen);

        int tmp = x[swap1];
        x[swap1] = x[swap2];
        x[swap2] = tmp;

        double logp = logLikelihood(x, data, logProb);
        if (logp > localMaxLogp)
        {
            localMaxLogp = logp;
            localArgmax = x;
            if (logp >= maxLogp)
            {
                maxLogp = logp;
                argmax = x;

                /*
                std::cout << "actual: " << std::endl;
                for (int i = 0; i < real.size(); ++i)
                    std::cout << i << " -> " << real[i] << std::endl;
                std::cout << std::endl;

                std::cout << "argmax: " << std::endl;
                for (int i = 0; i < argmax.size(); ++i)
                    std::cout << i << " -> " << argmax[i] << std::endl;
                std::cout << std::endl;
                int correct = 0;
                for (int i = 0; i < argmax.size(); ++i)
                {
                    if (argmax[i] == real[i])
                        ++correct;
                }
                */
                //std::cout << correct << " out of " << argmax.size() << std::endl;
                //std::cout << "----------------------------" << std::endl;
            }            
            nn = 0;
        }
        else if (++nn == 650)
        {
            i += 1000;
            localArgmax = maxLogLikelihood(1000, std::move(localArgmax), data, logProb);
            localMaxLogp = logLikelihood(localArgmax, data, logProb);
            nn = 0;
            continue;
        }
        ++i;
    }
    return argmax;
}

std::vector<int> maxLogLikelihood(int numSamples, std::vector<int> candidate, const std::vector<int>& data,
                      int numWords, bool obDistKnown, double logu, const std::vector<double>& logProb)
{
    std::vector<int> argmax = candidate;
    double maxLogProb = logLikelihood(argmax, data, numWords, obDistKnown, logu, logProb);

    for (int i = 0; i < numSamples; ++i)
    {
        double logp = 0;
        for (const auto& d : data)
        {
            int x = candidate[d];
            if (obDistKnown || x < numWords)
                logp += logProb[x];
            else
                logp += logu;

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

std::vector<int> maxLogLikelihood(int numSamples, std::vector<int> candidate, const std::vector<int>& data, const std::vector<double>& logProb)
{
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

double simulateAttackerMCObfuscations(int numWords, double probObfuscation, int numObfuscations, int numQueryTerms, bool obDistKnown, int numSamples)
{
    if (probObfuscation <= 0 || numWords <= 0 || numObfuscations <= 0 || numQueryTerms <= 0 || numSamples <= 0)
        throw std::exception("Invalid Argument");

    const auto& freqTable = makeFreqTableOb(numWords, numObfuscations, probObfuscation);
    boost::random::discrete_distribution<> dist(freqTable.begin(), freqTable.end());
    std::vector<int> hiddenToPlainMap;
    const int nn = numWords + numObfuscations;

    for (int i = 0; i < nn; ++i)
    {
        hiddenToPlainMap.push_back(i);
    }

    std::shuffle(hiddenToPlainMap.begin(), hiddenToPlainMap.end(), gen);
    auto candidate = hiddenToPlainMap;
    std::shuffle(candidate.begin(), candidate.end(), gen);

    std::vector<int> plainToHidden(nn);
    for (int i = 0; i < nn; ++i)
    {
        plainToHidden[hiddenToPlainMap[i]] = i;
    }
    
    std::vector<int> hTerms;
    for (int i = 0; i < numQueryTerms; ++i)
    {
        hTerms.push_back(plainToHidden[dist(gen)]);
    }

    auto logProb = dist.probabilities();
    for (auto& p : logProb)
        p = std::log(p);

    double logu = std::log(probObfuscation / numObfuscations);
    auto argmax = maxLogLikelihoodHillClimber(numSamples,
                                   maxLogLikelihood(1000, candidate, hTerms, numWords, obDistKnown, logu, logProb),
                                   hTerms, numWords, obDistKnown, logu, logProb, hiddenToPlainMap);

    int correct = 0;
    for (int i = 0; i < nn; ++i)
    {
        if (hiddenToPlainMap[i] < numWords && hiddenToPlainMap[i] == argmax[i])
            ++correct;
    }

    return (double)correct / nn;
}

double simulateAttackerMCSecrets(int numWords, int numSecrets, int numQueryTerms, int numSamples)
{
    std::vector<double> freqTable;
    for (int i = 0; i < numWords; ++i)
        freqTable.push_back(1 + rd() % (100 * numWords));
    boost::random::discrete_distribution<> dist(freqTable.begin(), freqTable.end());

    const int nn = numWords * numSecrets;

    std::vector<int> hiddenToPlainMap;
    std::vector<int> candidateMap;

    hiddenToPlainMap.reserve(nn);
    candidateMap.reserve(nn);

    for (int i = 0; i < numWords; ++i)
    {
        for (int j = 0; j < numSecrets; ++j)
        {
            hiddenToPlainMap.push_back(i);
            candidateMap.push_back(i);
        }
    }
    std::shuffle(hiddenToPlainMap.begin(), hiddenToPlainMap.end(), gen);

    std::vector<std::vector<int>> plainToHidden(numWords);
    for (int hidden = 0; hidden < nn; ++hidden)
    {
        int plain = hiddenToPlainMap[hidden];
        plainToHidden[plain].push_back(hidden);
    }

    /*
    std::cout << "plain to hidden map" << std::endl;
    for (int i = 0; i < plainToHidden.size(); ++i)
    {
        std::cout << i << " -> { ";
        for (auto hidden : plainToHidden[i])
        {
            std::cout << hidden << " ";
        }
        std::cout << "}" << std::endl;
    }

    std::cout << "hidden to plain map" << std::endl;
    for (int i = 0; i < nn; ++i)
    {
        std::cout << i << " -> " << hiddenToPlainMap[i] << std::endl;
    }

    std::vector<int> pTerms;
    */

    std::vector<int> hTerms;
    for (int i = 0; i < numQueryTerms; ++i)
    {
        int plain = dist(gen);
        //pTerms.push_back(plain);
        auto hiddenTerms = plainToHidden[plain];
        hTerms.push_back(hiddenTerms[rd() % numSecrets]);
    }

    /*
    std::cout << "hidden terms" << std::endl;
    for (int i = 0; i < hTerms.size(); ++i)
    {
        std::cout << hTerms[i] << " [" << hiddenToPlainMap[hTerms[i]] << "]" << "\t" << pTerms[i] << " -> { ";

        for (auto x : plainToHidden[pTerms[i]])
            std::cout << x << " ";
        std::cout << "}" << std::endl;
    }

    std::cout << "pdf" << std::endl;
    */

    auto logProb = dist.probabilities();
    //int i = 0;
    for (auto& p : logProb)
    {
        //std::cout << i++ << " -> " << p << " [";
        p = std::log(p);
        //std::cout << p << "]" << std::endl;
    }
    candidateMap = maxLogLikelihood(1000, candidateMap, hTerms, logProb);

    /*
    std::cout << "candidate map: " << std::endl;
    for (int i = 0; i < candidateMap.size(); ++i)
        std::cout << i << " -> " << candidateMap[i] << std::endl;
    std::cout << std::endl;
    */

    auto argmax = maxLogLikelihoodHillClimberSecrets(numSamples,
                                   candidateMap,
                                   hTerms, logProb, hiddenToPlainMap);

    int correct = 0;
    for (int i = 0; i < nn; ++i)
    {
        if (argmax[i] == hiddenToPlainMap[i])
            ++correct;
    }

    return (double)correct / nn;
}

void testPsip()
{
    int blockSizes[] = { 31, 63, 125, 250, 500, 1000 };

    std::cout << "type,doc_size,block_size,loc_err,pair_err" << std::endl;
    for (int numBlocks = 4; numBlocks <= 20; numBlocks += 4)
    {
        for (int blockSize : blockSizes)
        {
            int docSize = numBlocks * blockSize;           
            std::cout << "block_uniform," << docSize << "," << blockSize << "," << blockUniformAverageLocationError(blockSize, docSize, 100) << "," << blockAveragePair(blockSize, docSize, 100) << std::endl;
            std::cout << "centered_uniform," << docSize << "," << blockSize << "," << psipUniformAverageLocationError(blockSize/2, docSize, 100) << "," << psipUniformAveragePair(blockSize/2, docSize, 100) << std::endl;
            std::cout << "centered_triangular," << docSize << "," << blockSize << "," << psipTriangularAverageLocationError(blockSize/2, docSize, 100) << "," << psipTriangularAveragePair(blockSize/2, docSize, 100) << std::endl;
        }
    }
}

double triangular(double a, double b, double c)
{
    if (a >= b || a > c || c > b)
        throw std::exception("Invalid Argument: a < b, a <= c <= b");

    const double Fc = (c - a) / (b - a);
    const double F1 = std::sqrt((b - a) * (c - a));
    const double F2 = std::sqrt((b - a) * (b - c));

    const double p = (double)rd() / (double)rd.max();
    return p < Fc ?
        a + std::sqrt(p) * F1 : b - std::sqrt(1 - p) * F2;
}

MatrixInt makeMatrixInt(int n, int value)
{
    MatrixInt m;
    m.reserve(n);
    for (int i = 0; i < n; ++i)
    {
        m.push_back(std::vector<int>(n, value));
    }
    return m;
}

MatrixInt getMinPwDistances(const std::vector<std::vector<uint32_t>>& p)
{
    std::vector<int> places(p.size(), 0);
    std::vector<int> last(p.size(), -1);
    MatrixInt m = makeMatrixInt(p.size(), std::numeric_limits<int>::max());
    int idx, minPlace;

    do
    {
        minPlace = std::numeric_limits<int>::max();
        idx = -1;
        for (int i = 0; i < places.size(); ++i)
        {
            if (p[i].size() <= places[i])
                continue;

            auto place = p[i][places[i]];
            if (place < minPlace)
            {
                idx = i;
                minPlace = place;
            }
        }

        if (idx != -1)
        {
            last[idx] = p[idx][places[idx]];
            for (int i = 0; i < p.size(); ++i)
            {
                if (i == idx || last[i] == -1)
                    continue;

                int d = std::abs(last[i] - last[idx]);
                if (d < m[idx][i])
                    m[idx][i] = m[i][idx] = d;
            }
            ++places[idx];
        }
    } while (idx != -1);

    return m;
}
