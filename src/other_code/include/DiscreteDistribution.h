#ifndef __DISCRETE_DISTRIBUTION_H__
#define __DISCRETE_DISTRIBUTION_H__

#include <vector>
#include <algorithm>
#include <ostream>
#include <functional>
#include <iostream>
#include <iomanip>
#include <cstdarg>
#include <list>
#include "BinaryIO.h"

namespace alex { namespace stochastic
{   
    class DiscreteDistribution
    {
    public:
        DiscreteDistribution() {};

        DiscreteDistribution(const DiscreteDistribution& copy):
            _cdf(copy._cdf)
        {
        };

        DiscreteDistribution(DiscreteDistribution&& src):
            _cdf(std::move(src._cdf))
        {
        };

        DiscreteDistribution& operator=(const DiscreteDistribution& other)
        {
            _cdf = other._cdf;
            return *this;
        };

        DiscreteDistribution& operator=(DiscreteDistribution&& src)
        {
            _cdf = std::move(src._cdf);
            return *this;
        };

        static DiscreteDistribution uniform(unsigned n)
        {
            std::vector<float> pdf(n);
            for (unsigned i = 0; i < n; ++i)
                pdf[i] = 1.0f;

            return DiscreteDistribution::fromPDF(pdf.begin(), pdf.end());
        };

        static DiscreteDistribution zipf(unsigned n)
        {
            float sum = 0;
            for (unsigned i = 1; i <= n; ++i)
                sum += (1.0f / i);

            std::vector<float> pdf(n);
            for (unsigned i = 1; i <= n; ++i)
                pdf[i-1] = 1.0f / (i*sum);

            return DiscreteDistribution::fromPDF(pdf.begin(), pdf.end());
        };

        static DiscreteDistribution binomial(unsigned n, float p)
        {
            auto choose = [](unsigned n, unsigned k) -> float
            {
                if (k == 0)
                    return 1;

                float result = (float)n;
                for (unsigned i = 2; i <= k; ++i)
                    result /= i;
                for (unsigned i = n - k + 1; i < n; ++i)
                    result *= i;

                return result;
            };

            std::vector<float> pdf(n + 1);
            for (unsigned i = 0; i <= n; ++i)
                pdf[i] = choose(n, i) * std::pow(p, (float)i) * std::pow(1.0f - p, (float)(n - i));

            return DiscreteDistribution::fromPDF(pdf.begin(), pdf.end());
        };

        static DiscreteDistribution poisson(float mu, float minAccuracy = 0.9995f)
        {
            std::list<float> cdf;
            float eu = std::exp(-mu);
            int k = 1;
            float pk = eu;
            float uk = mu;
            float f = 1;

            cdf.push_back(pk);
            while (pk < minAccuracy)
            {
                pk += eu * uk / f;
                uk *= mu;
                cdf.push_back(pk);
                ++k;
                f *= k;
            }
            cdf.push_back(1);

            return DiscreteDistribution::fromCDF(cdf.begin(), cdf.end());
        }

        static DiscreteDistribution fromPDF(size_t n, ...)
        {
            va_list args;
            va_start(args, n);
            std::list<float> pdf;
            for (size_t i = 0; i < n; ++i)
                pdf.push_back((float)va_arg(args, float));

            return DiscreteDistribution::fromPDF(pdf.begin(), pdf.end());
        }

        static DiscreteDistribution fromCDF(size_t n, ...)
        {
            va_list args;
            va_start(args, n);
            std::list<float> cdf;
            for (size_t i = 0; i < n; ++i)
                cdf.push_back((float)va_arg(args, float));

            return DiscreteDistribution::fromCDF(cdf.begin(), cdf.end());
        }

        template <class Iter> static DiscreteDistribution fromPDF(Iter begin, Iter end)
        {
            DiscreteDistribution d;
            for (auto i = begin; i != end; ++i)
            {
                if (*i < 0)
                    throw std::exception("Not A Distribution");
                d._cdf.push_back(*i);
            }
            for (unsigned i = 1; i < d._cdf.size(); ++i)
                d._cdf[i] += d._cdf[i - 1];
            if (d._cdf.back() == 0)
                throw std::exception("Not A Distribution");
            for (auto i = d._cdf.begin(); i != d._cdf.end(); ++i)
                *i /= d._cdf.back();
            return d;
        }
        
        template <class Iter> static DiscreteDistribution fromCDF(Iter begin, Iter end)
        {
            std::vector<float> pdf;
            float last = 0;
            for (auto i = begin; i != end; ++i)
            {
                pdf.push_back(*i - last);
                last = *i;
            }
            return DiscreteDistribution::fromPDF(pdf.begin(), pdf.end());
        };

        // list consists of nonnegative integers, and probability distribution is constructed
        // by the frequency of occurrence of each nonnegative integer.
        // 
        // example:
        //      0, 2, 0, 1, 4 -> P(X<0)=0, P(X=0)=2/5, P(X=1)=1/5, P(X=2)=1/5, P(X=3)=0, P(X=4)=1/5, P(X>4)=0
        template <class Iter> static DiscreteDistribution fromFrequency(Iter begin, Iter end)
        {
            std::map<int, int> freqs;
            int max = -1;
            for (auto i = begin; i != end; ++i)
            {
                if (*i < 0)
                    throw std::exception("Not A Distribution");
                else if (freqs.count(*i) == 0)
                {
                    freqs[*i] = 1;
                    if (*i > max)
                        max = *i;
                }
                else
                    ++freqs[*i];
            }

            if (max < 0)
                throw std::exception("Not A Distribution");

            std::vector<float> pdf(max + 1);
            for (int i = 0; i <= max; ++i)
            {
                if (freqs.count(i) == 0)
                    pdf[i] = 0;
                else
                    pdf[i] = (float)freqs[i];
            }

            return DiscreteDistribution::fromPDF(pdf.begin(), pdf.end());
        };

        static DiscreteDistribution fromFrequency(size_t n, ...)
        {
            va_list args;
            va_start(args, n);
            std::list<int> v;
            for (size_t i = 0; i < n; ++i)
                v.push_back((int)va_arg(args, int));

            return DiscreteDistribution::fromFrequency(v.begin(), v.end());
        };

        std::function<float (int)> toCdf() const
        {
            const auto copy = *this;
            return [copy](int x) -> float { return copy.cdf(x); };
        };

        std::function<float (int)> toPdf() const
        {
			std::vector<float> pdf(_cdf.size());
			pdf[0] = _cdf[0];
			for (size_t i = 1; i < _cdf.size(); ++i)
				pdf[i] = _cdf[i] - _cdf[i - 1];

            return [pdf](int x) -> float
            {
                if (x >= (int)pdf.size() || x < 0)
                    return 0;
                else
                    return pdf[x];
            };
        };

        size_t size() const
        {
            return _cdf.size();
        };

        float cdf(int x) const
        {
            if (x < 0)
                return 0;
            else if (x >= (int)_cdf.size())
                return 1;
            else
                return _cdf[x];
        };

        float pdf(int x) const
        {
            if (x >= (int)_cdf.size() || x < 0)
                return 0;
            else
                return x == 0 ? _cdf[0] : _cdf[x] - _cdf[x - 1];
        };

        int inverseCdf(float p) const
        {
            if (p < 0 || p > 1)
                throw std::exception("Invalid Argument: 0 <= p <= 1");
            else
                return (p <= _cdf[0] ? 0 : inverseFind(0, (int)_cdf.size(), p));
        };

        float mean() const
        {
            float mu = 0;
            for (int i = 0; i < (int)_cdf.size(); ++i)
                mu += i * pdf(i);
            return mu;
        };

        float variance() const
        {
            return centralMoment(2);
        };

        float stddev() const
        {
            return std::sqrt(variance());
        };

        float skewness() const
        {
            return centralMoment(3) / pow(variance(), 1.5f);
        };

        float centralMoment(unsigned n) const
        {
            const float mu = mean();
            float result = 0;
            for (int i = 0; i < (int)_cdf.size(); ++i)
                result += std::pow((float)i - mu, (float)n) * pdf(i);
            return result;
        };

        float kurtosis() const
        {
            return centralMoment(4) / std::pow(variance(), 2);
        };

        int median() const
        {
            return quantile(0.5);
        };

        int percentile(float p) const
        {
            if (p < 0 || p > 100)
                throw std::exception("Invalid Argument: 0 <= p <= 100");

            return quantile(p / 100);
        };

        int quartile(int n) const
        {
            if (n < 1 || n > 3)
                throw std::exception("Invalid Argument: n = { 1, 2, 3 }");

            return quantile(n / 4.0f);
        };

        int quantile(float p) const
        {
            if (p < 0 || p > 1)
                throw std::exception("Invalid Argument: 0 <= p <= 1");

            if (p < _cdf[0])
                return 0;
            else
                return inverseFind(0, (int)_cdf.size(), p);
        };

        int minimum() const
        {
            return 0;
        };

        int maximum() const
        {
            return (int)_cdf.size() - 1;
        };

        int iqr() const
        {
            return quartile(3) - quartile(1);
        };

        void statistics(std::ostream& outs = std::cout) const
        {
            outs << "Mean: " << mean() << std::endl
                 << "Variance: " << variance() << std::endl
                 << "Standard Deviation: " << stddev() << std::endl
                 << "Skewness: " << skewness() << std::endl
                 << "Kurtosis: " << kurtosis() << std::endl
                 << "Median: " << median() << std::endl
                 << "1st Quartile: " << quartile(1) << std::endl
                 << "3rd Quartile: " << quartile(3) << std::endl
                 << "Minimum: " << minimum() << std::endl
                 << "Maximum: " << maximum() << std::endl
                 << "Support: { 0, ..., " << _cdf.size() - 1 << " }\n" << std::endl
                 << std::left << std::setw(10) << "x" << std::setw(20) << "P(X = x)" << std::setw(20) << "P(X <= x)" << std::endl
                 << std::setw(10) << "-" << std::setw(20) << "--------" << std::setw(20) << "---------" << std::endl;
            for (int i = 0; i < (int)_cdf.size(); ++i)
                outs << std::setw(10) << i << std::setw(20) << pdf(i) << std::setw(20) << cdf(i) << std::endl;
        };

        void readBinary(std::istream& file)
        {
            const auto& hdr = alex::io::binary::readString(file);
            if (hdr != "dd")
                throw std::exception(("Unexpected Header: " + hdr).c_str());
            uint8_t version = alex::io::binary::readU8(file);
            if (version != 1)
                throw std::exception(("Unexpected Version: " + std::to_string(version)).c_str());

            _cdf.clear();
            uint32_t n = alex::io::binary::readVarU32(file);
            _cdf.reserve(n);
            for (size_t i = 0; i < n; ++i)
                _cdf.push_back(alex::io::binary::read754F(file));
        }

        void writeBinary(std::ostream& file)
        {
            alex::io::binary::writeString(file, "dd");
            alex::io::binary::writeU8(file, 1);
            alex::io::binary::writeVarU32(file, (uint32_t)_cdf.size());
            for (size_t i = 0; i < _cdf.size(); ++i)
                alex::io::binary::write754F(file, _cdf[i]);
        }

    private:
        int inverseFind(int low, int high, float u) const
        {
            if (low > high)
                return (int)_cdf.size() - 1;

            int mid = (low + high) / 2;
            if (u <= _cdf[mid])
                return inverseFind(low, mid - 1, u);
            else if (u > _cdf[mid + 1])
                return inverseFind(mid + 1, high, u);
            else
                return mid + 1;
        };

        std::vector<float> _cdf;
    };
}}

#endif
