#include "bench_framework.hpp"
#include <mapnik/util/conversions.hpp>

class test : public benchmark::test_case
{
    double value_;
public:
    test(mapnik::parameters const& params)
     : test_case(params),
       value_(-0.1234) {}
    bool validate() const
    {
        std::string s;
        mapnik::util::to_string(s,value_);
        return (s == "-0.1234");
    }
    bool operator()() const
    {
        std::string out;
        for (std::size_t i=0;i<iterations_;++i) {
            out.clear();
            mapnik::util::to_string(out,value_);
        }
        return true;
    }
};

BENCHMARK(test,"to_string double->string")
