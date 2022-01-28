#include "bench_framework.hpp"
#include <mapnik/font_engine_freetype.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>

class test : public benchmark::test_case
{
  public:
    test(mapnik::parameters const& params)
        : test_case(params)
    {}
    bool validate() const { return mapnik::freetype_engine::register_fonts("./fonts", true); }
    bool operator()() const
    {
        unsigned long count = 0;
        for (unsigned i = 0; i < iterations_; ++i)
        {
            mapnik::freetype_engine::register_fonts("./fonts", true);
            count++;
        }
        return true;
    }
};

BENCHMARK(test, "font registration")
