#ifndef __RANDOM_VARIATES_H__
#define __RANDOM_VARIATES_H__

#include <functional>
#include <map>
#include "DiscreteDistribution.h"
#include "Entropy.h"
#include <memory>

namespace alex { namespace stochastic { namespace random_variates
{
    template <class T>
	std::function<double(void)> weibull(double theta, double beta, Entropy<T>& entropy);

    template <class T>
	std::function<double(void)> exponential(double lambda, Entropy<T>& entropy);

    template <class T>
	std::function<double(void)> normal(double mu, double sigma, Entropy<T>& entropy);

    template <class T>
	std::function<double(void)> logistic(double mu, double s, Entropy<T>& entropy);

    template <class T>
	std::function<double(void)> uniform(double min, double max, Entropy<T>& entropy);

    template <class T>
	std::function<double(void)> triangular(double a, double b, double c, Entropy<T>& entropy);

	std::function<int(void)> degenerate(int x);

    template <class T>
	std::function<unsigned int(void)> binomial(unsigned n, double p, Entropy<T>& entropy);

    template <class T>
	std::function<std::vector<unsigned int>(void)> multinomial(unsigned int n, const DiscreteDistribution& d, Entropy<T>& entropy);

    template <class T>
	std::function<unsigned int(void)> nb(unsigned int r, double p, Entropy<T>& entropy);

    template <class T>
	std::function<bool(void)> bernoulli(double p, Entropy<T>& entropy);

    template <class T>
	std::function<unsigned int(void)> geometric(double p, Entropy<T>& entropy);

    template <class T>
	std::function<unsigned int(void)> poisson(double mu, double minAccuracy, Entropy<T>& entropy);

    template <class T>
	std::function<int(void)> uniform_discrete(int min, int max, Entropy<T>& entropy);

    template <class T>
	std::function<unsigned int (void)> discrete(const DiscreteDistribution& d, Entropy<T>& entropy);

    // continuous r.v.
    template <class T>
    std::function<double (void)> weibull(double theta, double beta, Entropy<T>& entropy)
    {
        const double invBeta = 1 / beta;
        return [theta, invBeta, &entropy](void) -> double
            { return theta * std::pow(-std::log(entropy.get0_1()), invBeta); };
    }

    template <class T>
    std::function<double (void)> exponential(double lambda, Entropy<T>& entropy)
    {
        const double a = -1 / lambda;
        return [a, &entropy](void) -> double { return a * std::log(entropy.get0_1()); };
    }

    template <class T>
    std::function<double (void)> normal(double mu, double sigma, Entropy<T>& entropy)
    {
        static const double two_pi = 6.28318530718;
        const double a = std::sqrt(2) * sigma;
        return [mu, a, &entropy](void) -> double
        {
            return mu + a * std::sqrt(-std::log(entropy.get0_1())) *
                std::cos(two_pi * entropy.get0_1());
        };
    }

    template <class T>
    std::function<double (void)> logistic(double mu, double s, Entropy<T>& entropy)
    {
        return [mu, s, &entropy](void) -> double { return mu - s * std::log(1 / entropy.get0_1() - 1); };
    }

    template <class T>
    std::function<double (void)> uniform(double min, double max, Entropy<T>& entropy)
    {
        return [min, max, &entropy](void) -> double { return entropy.getReal(min, max); };
    }

    template <class T>
    std::function<double (void)> triangular(double a, double b, double c, Entropy<T>& entropy)
    {
        if (a >= b || a > c || c > b)
            throw std::exception("Invalid Argument: a < b, a <= c <= b");

        const double Fc = (c - a) / (b - a);
        const double F1 = std::sqrt((b - a) * (c - a));
        const double F2 = std::sqrt((b - a) * (b - c));

        return [Fc, a, b, F1, F2, &entropy](void) -> double
        {
            const double p = entropy.get0_1();
            return p < Fc ?
                a + std::sqrt(p) * F1 : b - std::sqrt(1 - p) * F2;
        };
    }

    // discrete r.v.
    std::function<int (void)> degenerate(int x)
    {
        return [x](void) -> int { return x; };
    }

    template <class T>
    std::function<unsigned int (void)> binomial(unsigned n, double p, Entropy<T>& entropy)
    {
        if (p <= 0 || p >= 1)
            throw std::exception("Invalid Argument: 0 < p < 1");

        if (n > 100)
        {
            return [n, p, &entropy]
            {
                size_t x = 0;
                for (size_t i = 0; i < n; ++i)
                    if (entropy.get0_1() < p) ++x;
                return x;
            };
        }
        else
        {
            auto bin = DiscreteDistribution::binomial((int)n, p);
            return [bin, &entropy](void) -> int { return bin.inverseCdf(entropy.get0_1()); };
        }
    }

    template <class T>
    std::function<std::vector<unsigned int> (void)> multinomial(unsigned int n, const DiscreteDistribution& d, Entropy<T>& entropy)
    {
        return [d, n, &entropy](void) -> std::vector<size_t>
        {
            std::vector<size_t> r(d.size());
            for (size_t i = 0; i < n; ++i)
                ++r[d.inverseCdf(entropy.get0_1())];
            return r;
        };
    }

    template <class T>
    std::function<size_t (void)> nb(unsigned int r, double p, Entropy<T>& entropy)
    {
        if (p <= 0 || p >= 1)
            throw std::exception("Invalid Argument: 0 < p < 1");

        return [r, p, &entropy](void) -> size_t
        {
            size_t x = 0, rPrime = 0;
            while (rPrime < r)
            {
                if (entropy.get0_1() < p)
                    ++rPrime;
                ++x;
            }
            return x;
        };
    }

    template <class T>
    std::function<bool (void)> bernoulli(double p, Entropy<T>& entropy)
    {
        if (p <= 0 || p >= 1)
            throw std::exception("Invalid Argument: 0 < p < 1");

        return [p, &entropy](void) -> bool { return entropy.get0_1() < p; };
    }

    template <class T>
    std::function<unsigned int (void)> geometric(double p, Entropy<T>& entropy)
    {
        if (p <= 0 || p >= 1)
            throw std::exception("Invalid Argument: 0 < p < 1");

        double logq = std::log(1 - p);
        return [logq, &entropy](void) -> size_t
        {
            return 1 + (size_t)(std::log(entropy.get0_1()) / logq);
        };
    }

    template <class T>
    std::function<unsigned int (void)> poisson(double mu, double minAccuracy, Entropy<T>& entropy)
    {
        if (mu <= 0 || minAccuracy <= 0)
            throw std::exception("Invalid Argument: mu > 0, minAccuracy > 0");

        auto poi = DiscreteDistribution::poisson(mu, minAccuracy);
        return [&entropy, poi](void) -> size_t
        {
            return poi.inverseCdf(entropy.get0_1());
        };
    }

    template <class T>
    std::function<int (void)> uniform_discrete(int min, int max, Entropy<T>& entropy)
    {
        return [min, max, &entropy](void) -> int { return entropy.getInt(min, max); };
    }

    template <class T>
    std::function<size_t (void)> discrete(const DiscreteDistribution& d, Entropy<T>& entropy)
    {
        return [d, &entropy](void) -> size_t { return d.inverseCdf(entropy.get0_1()); };
    }
}}}

#endif