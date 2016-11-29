#include "bench_framework.hpp"
#include <mapnik/unicode.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/expression_string.hpp>

class test : public benchmark::test_case
{
    std::string expr_;
public:
    test(mapnik::parameters const& params)
     : test_case(params),
       expr_("((([mapnik::geometry_type]=2) and ([oneway]=1)) and ([class]='path'))") {}
    bool validate() const
    {
        mapnik::expression_ptr expr = mapnik::parse_expression(expr_);
        std::string result = mapnik::to_expression_string(*expr);
        bool ret = (result == expr_);
        if (!ret)
        {
            std::clog << result  << " != " << expr_ << "\n";
        }
        return ret;
    }
    bool operator()() const
    {
         for (std::size_t i=0;i<iterations_;++i) {
             mapnik::expression_ptr expr = mapnik::parse_expression(expr_);
         }
         return true;
    }
};


int main(int argc, char** argv)
{
    mapnik::parameters params;
    benchmark::handle_args(argc,argv,params);
    test test_runner(params);
    return run(test_runner,"expr parsing");
}
