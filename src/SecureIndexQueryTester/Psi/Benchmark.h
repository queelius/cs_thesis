#ifndef __BENCHMARK_H__
#define __BENCHMARK_H__

#include <chrono>
#include <vector>
#include <string>
#include <functional>
#include <iostream>

namespace alex { namespace testing
{
    class Timer
    {
        typedef std::chrono::high_resolution_clock high_resolution_clock;
    public:
        typedef std::chrono::milliseconds milliseconds;
        typedef std::chrono::seconds seconds;

        explicit Timer(bool run = false)
        {
            if (run)
                reset();
        };

        void reset()
        {
            _start = high_resolution_clock::now();
        };

        milliseconds elapsed() const
        {
            return std::chrono::duration_cast<milliseconds>(high_resolution_clock::now() - _start);
        };

        seconds elapsedSeconds() const
        {
            return std::chrono::duration_cast<seconds>(high_resolution_clock::now() - _start);
        };

        template <typename T, typename Traits>
        friend std::basic_ostream<T, Traits>& operator<<(std::basic_ostream<T, Traits>& out, const Timer& timer)
        {
            return out << timer.elapsed().count();
        };

    private:
        high_resolution_clock::time_point _start;
    };

    class Benchmark
    {
            typedef std::conditional<std::chrono::high_resolution_clock::is_steady,
                std::chrono::high_resolution_clock, std::chrono::steady_clock>::type Clock;
            typedef Clock::time_point TimePoint;
            typedef Clock::duration Duration;

        public:
            class BadTrial: public std::exception { public: BadTrial(const char* msg) : std::exception(msg) {}; };

            Benchmark(std::ostream& os) : os(os) {};

            ~Benchmark()
            {
                if(mCurrentRun.nTrials > 0)
                {
                    mRuns.push_back(std::move(mCurrentRun));
                }
                if(mRuns.size() == 0)
                {
                    os << "benchmark: no trials" << std::endl;
                }
                unsigned int i = 0;
                for(const auto& run : mRuns)
                {
                    ++i;
                    os << "run ";
                    if(run.name.empty())
                    {
                        os << i;
                    }
                    else
                    {
                        os << '"' << run.name << '"';
                    }
                    os << ": ";
                    auto ticks = std::chrono::duration_cast<std::chrono::milliseconds>
                                 (run.elapsed).count();
                    os << ticks << "ms";
                    if(run.nTrials > 1)
                    {
                        os << " (" << run.nTrials << " trials, " << (ticks / run.nTrials) << "ms per trial)";
                    }
                    os << std::endl;
                }
            }

            void startRun(std::string name = std::string())
            {
                if(mCurrentRun.nTrials > 0)
                {
                    mRuns.push_back(std::move(mCurrentRun));
                    mCurrentRun = BenchRun(name);
                }
                else
                {
                    mCurrentRun.name = name;
                }
            }

            void startTrial()
            {
                if(mCurrentRun.start != TimePoint())
                {
                    throw BadTrial("");
                }
                ++mCurrentRun.nTrials;
                mCurrentRun.start = Clock::now();
            }

            void stopTrial()
            {
                TimePoint now = Clock::now();
                if(mCurrentRun.start == TimePoint())
                {
                    throw BadTrial("");
                }
                mCurrentRun.elapsed += now - mCurrentRun.start;
                mCurrentRun.start = TimePoint();
            }

            void run(const std::string& name, std::function<void (void)> f)
            {
                startRun(name);
                trial(f);
            }

            void run(const std::string& name, unsigned int num,
                     std::function<void (void)> f)
            {
                startRun(name);
                trial(num, f);
            }

            void run(const std::string& name, unsigned int num,
                     std::function<void (unsigned int)> f)
            {
                startRun(name);
                trial(num, f);
            }

            void trial(std::function<void (void)> f)
            {
                startTrial();
                try
                {
                    f();
                }
                catch(...)
                {
                    stopTrial();
                    throw;
                }
                stopTrial();
            }

            void trial(unsigned int num, std::function<void (void)> f)
            {
                for(unsigned int i = 0; i < num; i++)
                {
                    trial(f);
                }
            }
            void trial(unsigned int num, std::function<void (unsigned int)> f)
            {
                for(unsigned int i = 0; i < num; i++)
                {
                    trial(std::bind(f, i));
                }
            }

        private:
            struct BenchRun
            {
                std::string name;
                TimePoint start;
                Duration elapsed;
                unsigned int nTrials;

                BenchRun() : BenchRun(std::string()) {}
                BenchRun(const std::string& str) : name(str), elapsed(0), nTrials(0) {}
                BenchRun(const BenchRun& r) = default;
                BenchRun(BenchRun&& r)
                {
                    std::swap(name, r.name);
                    std::swap(start, r.start);
                    std::swap(elapsed, r.elapsed);
                    std::swap(nTrials, r.nTrials);
                }
                BenchRun& operator=(const BenchRun& r) = default;
            };
            std::vector<BenchRun> mRuns;
            BenchRun mCurrentRun;
            std::ostream& os;
    };
}}

#endif