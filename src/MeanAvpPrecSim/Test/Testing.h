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
}}

#endif