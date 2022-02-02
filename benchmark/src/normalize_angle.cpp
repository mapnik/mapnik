#include "bench_framework.hpp"

#include <mapnik/util/math.hpp>

template<typename T>
struct bench_func : benchmark::test_case
{
    T (*const func_)(T);
    T const value_;

    bench_func(mapnik::parameters const& params, T (*func)(T), T value)
        : test_case(params)
        , func_(func)
        , value_(value)
    {}

    bool validate() const { return true; }

    bool operator()() const
    {
        for (auto i = this->iterations_; i-- > 0;)
        {
            func_(value_);
        }
        return true;
    }
};

#define BENCH_FUNC1(func, value) run<bench_func<double>>(#func "(" #value ")", func, value)

int main(int argc, char** argv)
{
    return benchmark::sequencer(argc, argv)
      .BENCH_FUNC1(mapnik::util::normalize_angle, +3)
      .BENCH_FUNC1(mapnik::util::normalize_angle, +6)
      .BENCH_FUNC1(mapnik::util::normalize_angle, +9)
      .BENCH_FUNC1(mapnik::util::normalize_angle, +12)
      .BENCH_FUNC1(mapnik::util::normalize_angle, +15)
      .BENCH_FUNC1(mapnik::util::normalize_angle, +20)
      .BENCH_FUNC1(mapnik::util::normalize_angle, +30)
      .BENCH_FUNC1(mapnik::util::normalize_angle, +40)
      .BENCH_FUNC1(mapnik::util::normalize_angle, +50)
      .BENCH_FUNC1(mapnik::util::normalize_angle, +70)
      .BENCH_FUNC1(mapnik::util::normalize_angle, +90)
      .BENCH_FUNC1(mapnik::util::normalize_angle, +110)
      .BENCH_FUNC1(mapnik::util::normalize_angle, +130)
      .BENCH_FUNC1(mapnik::util::normalize_angle, +157)
      .BENCH_FUNC1(mapnik::util::normalize_angle, +209)
      .BENCH_FUNC1(mapnik::util::normalize_angle, +314)
      .BENCH_FUNC1(mapnik::util::normalize_angle, +628)
      .BENCH_FUNC1(mapnik::util::normalize_angle, +942)
      .BENCH_FUNC1(mapnik::util::normalize_angle, -3)
      .BENCH_FUNC1(mapnik::util::normalize_angle, -6)
      .BENCH_FUNC1(mapnik::util::normalize_angle, -9)
      .BENCH_FUNC1(mapnik::util::normalize_angle, -12)
      .BENCH_FUNC1(mapnik::util::normalize_angle, -15)
      .BENCH_FUNC1(mapnik::util::normalize_angle, -20)
      .BENCH_FUNC1(mapnik::util::normalize_angle, -30)
      .BENCH_FUNC1(mapnik::util::normalize_angle, -40)
      .BENCH_FUNC1(mapnik::util::normalize_angle, -50)
      .BENCH_FUNC1(mapnik::util::normalize_angle, -70)
      .BENCH_FUNC1(mapnik::util::normalize_angle, -90)
      .BENCH_FUNC1(mapnik::util::normalize_angle, -110)
      .BENCH_FUNC1(mapnik::util::normalize_angle, -130)
      .BENCH_FUNC1(mapnik::util::normalize_angle, -157)
      .BENCH_FUNC1(mapnik::util::normalize_angle, -209)
      .BENCH_FUNC1(mapnik::util::normalize_angle, -314)
      .BENCH_FUNC1(mapnik::util::normalize_angle, -628)
      .BENCH_FUNC1(mapnik::util::normalize_angle, -942)
      .done();
}
