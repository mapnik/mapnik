#include "bench_framework.hpp"
#include <mapnik/util/conversions.hpp>

class test : public benchmark::test_case
{
    std::string value_;

  public:
    test(mapnik::parameters const& params)
        : test_case(params),
          value_("1.23456789")
    {}
    bool validate() const
    {
        double result = 0;
        if (!mapnik::util::string2double(value_.data(), value_.data() + value_.size(), result))
            return false;
        if (result != 1.23456789)
            return false;
        result = 0;
        if (!mapnik::util::string2double(value_, result))
            return false;
        if (result != 1.23456789)
            return false;
        return true;
    }
    bool operator()() const
    {
        for (std::size_t i = 0; i < iterations_; ++i)
        {
            double result = 0;
            mapnik::util::string2double(value_, result);
            mapnik::util::string2double(value_.data(), value_.data() + value_.size(), result);
        }
        return true;
    }
};

BENCHMARK(test, "string->double")
