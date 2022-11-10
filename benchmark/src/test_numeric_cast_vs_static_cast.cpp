#include "bench_framework.hpp"
// boost
#include <boost/numeric/conversion/cast.hpp>

static double STEP_NUM = 0.0000000001;
static std::uint8_t START_NUM = 2;

class test_static : public benchmark::test_case
{
    double step_;
    std::uint8_t start_;

  public:
    test_static(mapnik::parameters const& params)
        : test_case(params)
        , step_(STEP_NUM)
        , start_(START_NUM)
    {}
    bool validate() const { return true; }
    bool operator()() const
    {
        double value_ = 0.0;
        std::uint8_t x;
        for (std::size_t i = 0; i < iterations_; ++i)
        {
            double c = static_cast<double>(start_) * value_;
            if (c >= 256.0)
                c = 255.0;
            if (c < 0.0)
                c = 0.0;
            x = static_cast<std::uint8_t>(c);
            value_ += step_;
        }
        return static_cast<double>(x) < (static_cast<double>(start_) * value_);
    }
};

using boost::numeric::negative_overflow;
using boost::numeric::positive_overflow;

class test_numeric : public benchmark::test_case
{
    double step_;
    std::uint8_t start_;

  public:
    test_numeric(mapnik::parameters const& params)
        : test_case(params)
        , step_(STEP_NUM)
        , start_(START_NUM)
    {}
    bool validate() const { return true; }
    bool operator()() const
    {
        double value_ = 0.0;
        std::uint8_t x;
        for (std::size_t i = 0; i < iterations_; ++i)
        {
            try
            {
                x = boost::numeric_cast<std::uint8_t>(start_ * value_);
            }
            catch (negative_overflow&)
            {
                x = std::numeric_limits<std::uint8_t>::min();
            }
            catch (positive_overflow&)
            {
                x = std::numeric_limits<std::uint8_t>::max();
            }
            value_ += step_;
        }
        return static_cast<double>(x) < (static_cast<double>(start_) * value_);
    }
};

int main(int argc, char** argv)
{
    return benchmark::sequencer(argc, argv).run<test_static>("static_cast").run<test_numeric>("numeric_cast").done();
}
