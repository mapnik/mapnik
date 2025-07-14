#include "bench_framework.hpp"
#include <mapnik/util/conversions.hpp>

class test : public benchmark::test_case
{
    std::string value_;

  public:
    test(mapnik::parameters const& params)
        : test_case(params),
          value_("true")
    {}
    bool validate() const
    {
        bool result = false;
        mapnik::util::string2bool(value_.data(), value_.data() + value_.size(), result);
        if (!result)
            return result;
        mapnik::util::string2bool(value_, result);
        return (result == true);
    }
    bool operator()() const
    {
        for (std::size_t i = 0; i < iterations_; ++i)
        {
            bool result = false;
            mapnik::util::string2bool(value_, result);
            mapnik::util::string2bool(value_.data(), value_.data() + value_.size(), result);
        }
        return true;
    }
};

BENCHMARK(test, "string->bool")
