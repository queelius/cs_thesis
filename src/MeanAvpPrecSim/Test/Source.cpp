#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <random>
#include <algorithm>
#include <random>
#include <unordered_set>
#include <boost/random/discrete_distribution.hpp>
#include <boost/program_options.hpp>
#include "SHA256.h"
#include "Testing.h"
using namespace std;
namespace po = boost::program_options;

std::random_device rd;
std::default_random_engine gen(rd());

int main(int ac, char* av[])
{
    const int MAX = 1000000;
    alex::testing::Timer t;
    t.reset();
    for (int i = 0; i < MAX; ++i)
    {
        auto s = alex::crypt::hexdigest<16>("1234567890123456");
    }
    std::cout << (t.elapsed().count() / (double)MAX) << std::endl;
    exit(0);


    try
    {
        std::cout.precision(10);

        const int TRIALS = 10000000;
        const int TOP_K = 10;
        const int ELEMENTS = 250;

        std::vector<int> ref; ref.reserve(ELEMENTS);
        for (int i = 0; i < ELEMENTS; ++i)
            ref.push_back(i);

        std::vector<int> test = ref;
        std::vector<double> trials(TRIALS);
        double mean;
        int i, trial, sum;

        for (trial = 0; trial < TRIALS; ++trial)
        {
            std::random_shuffle(ref.begin(), ref.end());
            std::random_shuffle(test.begin(), test.end());

            mean = 0;
            // calculate mean avg precision
            std::unordered_set<int> a;
            std::unordered_set<int> b;
            for (i = 0; i < TOP_K; ++i)
            {
                a.insert(ref[i]);
                b.insert(test[i]);

                sum = 0;
                for (auto x : a)
                {
                    if (b.count(x) != 0)
                        ++sum;
                }

                mean += (double)sum / (i + 1);
                // sum has total occurrences of element i
            }

            trials[trial] = mean / TOP_K;
        }
        
        double trial_mean = 0;
        std::vector<double> r = {0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1.0};
        std::vector<double> bins(r.size(), 0);

        for (auto x : trials)
        {
            trial_mean += x;
            
            for (int i = 0; i < r.size(); ++i)
            {
                if (x < r[i])
                {
                    bins[i]++;
                    break;
                }
            }            
        }

        trial_mean /= TRIALS;
        for (auto& x : bins)
            x /= TRIALS;

        std::cout << "mean: " << trial_mean << std::endl;

        std::cout << "pdf" << std::endl;
        double tmp = 0;
        for (int i = 0; i < bins.size(); ++i)
        {
            std::cout << (i == 0 ? 0 : r[i-1]) << " - " << r[i] << " => " << bins[i] << std::endl;
            tmp += 0.5 * (r[i] + (i == 0 ? 0 : r[i-1])) * bins[i];
        }
        std::cout << "tmp == " << tmp << std::endl;


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