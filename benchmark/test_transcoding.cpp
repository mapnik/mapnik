#include "bench_framework.hpp"
#include <mapnik/unicode.hpp>
#include <mapnik/value.hpp>

class test : public benchmark::test_case
{
    std::string expr_;
public:
    test(mapnik::parameters const& params)
     : test_case(params),
       expr_("this is a fairly long string that might contain unicode") {}
    bool validate() const
    {
        mapnik::transcoder tr_("utf-8");
        mapnik::value_unicode_string ustr = tr_.transcode(expr_.data(),expr_.size());
        std::string utf8;
        mapnik::to_utf8(ustr,utf8);
        return utf8 == expr_;
    }
    void operator()() const
    {
         mapnik::transcoder tr_("utf-8");
         mapnik::value_unicode_string ustr;
         for (std::size_t i=0;i<iterations_;++i) {
             ustr = tr_.transcode(expr_.data(),expr_.size());
         }
    }
};

int main(int argc, char** argv)
{
    mapnik::parameters params;
    benchmark::handle_args(argc,argv,params);
    test test_runner(params);
    return run(test_runner,"transcode");
}
