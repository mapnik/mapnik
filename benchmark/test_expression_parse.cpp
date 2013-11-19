#include "bench_framework.hpp"
#include <mapnik/unicode.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/expression_grammar.hpp>

class test : public benchmark::test_case
{
    std::string expr_;
public:
    test(mapnik::parameters const& params)
     : test_case(params),
       expr_("([foo]=1)") {}
    bool validate() const
    {
        mapnik::expression_ptr expr = mapnik::parse_expression(expr_,"utf-8");
        return mapnik::to_expression_string(*expr) == expr_;
    }
    void operator()() const
    {
         for (std::size_t i=0;i<iterations_;++i) {
             mapnik::expression_ptr expr = mapnik::parse_expression(expr_,"utf-8");
         }
    }
};

class test2 : public benchmark::test_case
{
    std::string expr_;
public:
    test2(mapnik::parameters const& params)
     : test_case(params),
       expr_("([foo]=1)") {}
    bool validate() const
    {
         mapnik::transcoder tr("utf-8");
         mapnik::expression_grammar<std::string::const_iterator> expr_grammar(tr);
         mapnik::expression_ptr expr = mapnik::parse_expression(expr_,expr_grammar);
         return mapnik::to_expression_string(*expr) == expr_;
    }
    void operator()() const
    {
         mapnik::transcoder tr("utf-8");
         mapnik::expression_grammar<std::string::const_iterator> expr_grammar(tr);
         for (std::size_t i=0;i<iterations_;++i) {
             mapnik::expression_ptr expr = mapnik::parse_expression(expr_,expr_grammar);
         }
    }
};

int main(int argc, char** argv)
{
    mapnik::parameters params;
    benchmark::handle_args(argc,argv,params);
    test test_runner(params);
    run(test_runner,"expr: grammer per parse");
    test2 test_runner2(params);
    return run(test_runner2,"expr: reuse grammar");
}
