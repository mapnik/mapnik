#include "bench_framework.hpp"
#include <mapnik/image_util.hpp>

class test : public benchmark::test_case
{
    mapnik::image_rgba8 im_;

  public:
    test(mapnik::parameters const& params)
        : test_case(params),
          im_(256, 256)
    {}
    bool validate() const { return true; }
    bool operator()() const
    {
        std::string out;
        for (std::size_t i = 0; i < iterations_; ++i)
        {
            out.clear();
            out = mapnik::save_to_string(im_, "png8:m=h:z=1");
        }
        return true;
    }
};

BENCHMARK(test, "encoding blank png")
