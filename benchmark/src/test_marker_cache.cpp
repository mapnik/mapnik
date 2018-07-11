#include "bench_framework.hpp"
#include <mapnik/marker_cache.hpp>

class test : public benchmark::test_case
{
    std::vector<std::string> images_;
public:
    test(mapnik::parameters const& params)
     : test_case(params),
       images_{
          "./test/data/images/dummy.jpg",
          "./test/data/images/dummy.jpeg",
          "./test/data/images/dummy.png",
          "./test/data/images/dummy.tif",
          "./test/data/images/dummy.tiff",
          //"./test/data/images/landusepattern.jpeg", // will fail since it is a png
          //"./test/data/images/xcode-CgBI.png", // will fail since its an invalid png
          "./test/data/svg/octocat.svg",
          "./test/data/svg/place-of-worship-24.svg",
          "./test/data/svg/point_sm.svg",
          "./test/data/svg/point.svg",
          "./test/data/svg/airfield-12.svg"
       } {}
    bool validate() const
    {
        return true;
    }
    bool operator()() const
    {
        unsigned count = 0;
        for (std::size_t i=0;i<iterations_;++i) {
            for (auto filename : images_)
            {
                auto marker = mapnik::marker_cache::instance().find(filename,true);
            }
            ++count;
        }
        return (count == iterations_);
    }
};

BENCHMARK(test,"marker cache")