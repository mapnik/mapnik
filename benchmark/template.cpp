#include "bench_framework.hpp"

class test : public benchmark::test_case
{
  public:
    test(mapnik::parameters const& params)
        : test_case(params)
    {}
    bool validate() const { return true; }
    void operator()() const {}
};

BENCHMARK(test, "test name")
