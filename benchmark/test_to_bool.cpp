#include "bench_framework.hpp"
#include <mapnik/util/conversions.hpp>

class test : public benchmark::test_case
{
    std::string value_;
public:
    test(mapnik::parameters const& params)
     : test_case(params),
       value_("true") {}
    bool validate() const
    {
        bool result = false;
        mapnik::util::string2bool(value_.data(),value_.data()+value_.size(),result);
        if (!result) return result;
        mapnik::util::string2bool(value_,result);
        return (result == true);
    }
    void operator()() const
    {
        for (std::size_t i=0;i<iterations_;++i) {
            bool result = false;
            mapnik::util::string2bool(value_,result);
            mapnik::util::string2bool(value_.data(),value_.data()+value_.size(),result);
        }
    }
};

class test2 : public benchmark::test_case
{
    std::string value_;
public:
    test2(mapnik::parameters const& params)
     : test_case(params),
       value_("1.23456789") {}
    bool validate() const
    {
        double result = 0;
        mapnik::util::string2double(value_.data(),value_.data()+value_.size(),result);
        if (result != 1.23456789) return false;
        result = 0;
        mapnik::util::string2double(value_,result);
        return (result == 1.23456789);
    }
    void operator()() const
    {
        for (std::size_t i=0;i<iterations_;++i) {
            double result = 0;
            mapnik::util::string2double(value_,result);
            //mapnik::util::string2double(value_.data(),value_.data()+value_.size(),result);
        }
    }
};

//BENCHMARK(test,"string->bool")
BENCHMARK(test2,"string->double")
