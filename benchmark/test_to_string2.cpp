#include "bench_framework.hpp"
#include <sstream>

class test : public benchmark::test_case
{
    double value_;
public:
    test(mapnik::parameters const& params)
     : test_case(params),
       value_(-0.1234) {}
    bool validate() const
    {
        std::ostringstream s;
        s << value_;
        return (s.str() == "-0.1234");
    }
    bool operator()() const
    {
        std::string out;
        for (std::size_t i=0;i<iterations_;++i) {
            std::ostringstream s;
            s << value_;
            out = s.str();
        }
        return true;
    }
};

BENCHMARK(test,"ostringstream double->string")
