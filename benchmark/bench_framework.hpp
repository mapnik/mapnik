#ifndef __MAPNIK_BENCH_FRAMEWORK_HPP__
#define __MAPNIK_BENCH_FRAMEWORK_HPP__

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/params.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/safe_cast.hpp>
#include "../test/cleanup.hpp"

// stl
#include <chrono>
#include <cstdio> // snprintf
#include <iostream>
#include <set>
#include <sstream>
#include <thread>
#include <vector>

namespace benchmark {

class test_case
{
protected:
    mapnik::parameters params_;
    std::size_t threads_;
    std::size_t iterations_;
public:
    test_case(mapnik::parameters const& params)
       : params_(params),
         threads_(mapnik::safe_cast<std::size_t>(*params.get<mapnik::value_integer>("threads",0))),
         iterations_(mapnik::safe_cast<std::size_t>(*params.get<mapnik::value_integer>("iterations",0)))
         {}
    std::size_t threads() const
    {
        return threads_;
    }
    std::size_t iterations() const
    {
        return iterations_;
    }
    mapnik::parameters const& params() const
    {
        return params_;
    }
    virtual bool validate() const = 0;
    virtual bool operator()() const = 0;
    virtual ~test_case() {}
};

// gathers --long-option values in 'params';
// returns the index of the first non-option argument,
// or negated index of an ill-formed option argument
inline int parse_args(int argc, char** argv, mapnik::parameters & params)
{
    for (int i = 1; i < argc; ++i) {
        const char* opt = argv[i];
        if (opt[0] != '-') {
            // non-option argument, return its index
            return i;
        }
        if (opt[1] != '-') {
            // we only accept --long-options, but instead of throwing,
            // just issue a warning and let the caller decide what to do
            std::clog << argv[0] << ": invalid option '" << opt << "'\n";
            return -i; // negative means ill-formed option #i
        }
        if (opt[2] == '\0') {
            // option-list terminator '--'
            return i + 1;
        }

        // take option name without the leading '--'
        std::string key(opt + 2);
        size_t eq = key.find('=');
        if (eq != std::string::npos) {
            // one-argument form '--foo=bar'
            params[key.substr(0, eq)] = key.substr(eq + 1);
        }
        else if (i + 1 < argc) {
            // two-argument form '--foo' 'bar'
            params[key] = std::string(argv[++i]);
        }
        else {
            // missing second argument
            std::clog << argv[0] << ": missing option '" << opt << "' value\n";
            return -i; // negative means ill-formed option #i
        }
    }
    return argc; // there were no non-option arguments
}

inline void handle_common_args(mapnik::parameters const& params)
{
    if (auto severity = params.get<std::string>("log-severity")) {
        if (*severity == "debug")
            mapnik::logger::set_severity(mapnik::logger::debug);
        else if (*severity == "warn")
            mapnik::logger::set_severity(mapnik::logger::warn);
        else if (*severity == "error")
            mapnik::logger::set_severity(mapnik::logger::error);
        else if (*severity == "none")
            mapnik::logger::set_severity(mapnik::logger::none);
        else
            std::clog << "ignoring option --log-severity='" << *severity
                      << "' (allowed values are: debug, warn, error, none)\n";
    }
}

inline int handle_args(int argc, char** argv, mapnik::parameters & params)
{
    int res = parse_args(argc, argv, params);
    handle_common_args(params);
    return res;
}

#define BENCHMARK(test_class,name)                      \
    int main(int argc, char** argv)                     \
    {                                                   \
        try                                             \
        {                                               \
            mapnik::parameters params;                  \
            benchmark::handle_args(argc,argv,params);   \
            test_class test_runner(params);             \
            auto result = run(test_runner,name);        \
            testing::run_cleanup();                     \
            return result;                              \
        }                                               \
        catch (std::exception const& ex)                \
        {                                               \
            std::clog << ex.what() << "\n";             \
            testing::run_cleanup();                     \
            return -1;                                  \
        }                                               \
    }                                                   \

template <typename T>
int run(T const& test_runner, std::string const& name)
{
    try
    {
        if (!test_runner.validate())
        {
            std::clog << "test did not validate: " << name << "\n";
            return 1;
        }
        // run test once before timing
        // if it returns false then we'll abort timing
        if (!test_runner())
        {
            return 2;
        }

        std::chrono::high_resolution_clock::time_point start;
        std::chrono::high_resolution_clock::duration elapsed;
        auto opt_min_duration = test_runner.params().template get<double>("min-duration", 0.0);
        std::chrono::duration<double> min_seconds(*opt_min_duration);
        auto min_duration = std::chrono::duration_cast<decltype(elapsed)>(min_seconds);
        std::size_t loops = 0;

        if (test_runner.threads() > 0)
        {
            using thread_group = std::vector<std::unique_ptr<std::thread> >;
            using value_type = thread_group::value_type;
            thread_group tg;
            for (std::size_t i=0;i<test_runner.threads();++i)
            {
                tg.emplace_back(new std::thread(test_runner));
            }
            start = std::chrono::high_resolution_clock::now();
            std::for_each(tg.begin(), tg.end(), [](value_type & t) {if (t->joinable()) t->join();});
            elapsed = std::chrono::high_resolution_clock::now() - start;
            loops = 1;
        }
        else
        {
            start = std::chrono::high_resolution_clock::now();
            do {
                test_runner();
                elapsed = std::chrono::high_resolution_clock::now() - start;
                ++loops;
            } while (elapsed < min_duration);
        }

        double iters = loops * test_runner.iterations();
        double dur_total = std::chrono::duration<double, std::milli>(elapsed).count();
        double dur_avg = dur_total / iters;
        char iters_unit = ' ';
        char msg[200];

        if (iters >= 1e7) iters *= 1e-6, iters_unit = 'M';
        else if (iters >= 1e4) iters *= 1e-3, iters_unit = 'k';

        std::snprintf(msg, sizeof(msg),
                "%-43s %3zu threads %4.0f%c iters %6.0f milliseconds",
                name.c_str(),
                test_runner.threads(),
                iters, iters_unit,
                dur_total);
        std::clog << msg;

        // log average time per iteration, currently only for non-threaded runs
        if (test_runner.threads() == 0)
        {
            char unit = 'm';
            if (dur_avg < 1e-5) dur_avg *= 1e+9, unit = 'p';
            else if (dur_avg < 1e-2) dur_avg *= 1e+6, unit = 'n';
            else if (dur_avg < 1e+1) dur_avg *= 1e+3, unit = 'u';
            std::snprintf(msg, sizeof(msg), " %4.0f%cs/iter", dur_avg, unit);
            std::clog << msg;
        }

        std::clog << "\n";
        return 0;
    }
    catch (std::exception const& ex)
    {
        std::clog << "test runner did not complete: " << ex.what() << "\n";
        return 4;
    }
}

struct sequencer
{
    sequencer(int argc, char** argv)
      : exit_code_(0)
    {
        benchmark::handle_args(argc, argv, params_);
    }

    int done() const
    {
        return exit_code_;
    }

    template <typename Test, typename... Args>
    sequencer & run(std::string const& name, Args && ...args)
    {
        // Test instance lifetime is confined to this function
        Test test_runner(params_, std::forward<Args>(args)...);
        // any failing test run will make exit code non-zero
        exit_code_ |= benchmark::run(test_runner, name);
        return *this; // allow chaining calls
    }

protected:
    mapnik::parameters params_;
    int exit_code_;
};

}

#endif // __MAPNIK_BENCH_FRAMEWORK_HPP__
