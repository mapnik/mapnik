#ifndef MAPNIK_BENCH_FRAMEWORK_HPP
#define MAPNIK_BENCH_FRAMEWORK_HPP

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/params.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/safe_cast.hpp>
#include "../test/cleanup.hpp"

// stl
#include <chrono>
#include <cmath>  // log10, round
#include <cstdio> // snprintf
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <thread>
#include <mutex>
#include <vector>

namespace benchmark {

template<typename T>
using milliseconds = std::chrono::duration<T, std::milli>;

template<typename T>
using seconds = std::chrono::duration<T>;

class test_case
{
  protected:
    mapnik::parameters params_;
    std::size_t threads_;
    std::size_t iterations_;

  public:
    test_case(mapnik::parameters const& params)
        : params_(params)
        , threads_(mapnik::safe_cast<std::size_t>(*params.get<mapnik::value_integer>("threads", 0)))
        , iterations_(mapnik::safe_cast<std::size_t>(*params.get<mapnik::value_integer>("iterations", 0)))
    {}
    std::size_t threads() const { return threads_; }
    std::size_t iterations() const { return iterations_; }
    mapnik::parameters const& params() const { return params_; }
    virtual bool validate() const = 0;
    virtual bool operator()() const = 0;
};

// gathers --long-option values in 'params';
// returns the index of the first non-option argument,
// or negated index of an ill-formed option argument
inline int parse_args(int argc, char** argv, mapnik::parameters& params)
{
    for (int i = 1; i < argc; ++i)
    {
        const char* opt = argv[i];
        if (opt[0] != '-')
        {
            // non-option argument, return its index
            return i;
        }
        if (opt[1] != '-')
        {
            // we only accept --long-options, but instead of throwing,
            // just issue a warning and let the caller decide what to do
            std::clog << argv[0] << ": invalid option '" << opt << "'\n";
            return -i; // negative means ill-formed option #i
        }
        if (opt[2] == '\0')
        {
            // option-list terminator '--'
            return i + 1;
        }

        // take option name without the leading '--'
        std::string key(opt + 2);
        size_t eq = key.find('=');
        if (eq != std::string::npos)
        {
            // one-argument form '--foo=bar'
            params[key.substr(0, eq)] = key.substr(eq + 1);
        }
        else if (i + 1 < argc)
        {
            // two-argument form '--foo' 'bar'
            params[key] = std::string(argv[++i]);
        }
        else
        {
            // missing second argument
            std::clog << argv[0] << ": missing option '" << opt << "' value\n";
            return -i; // negative means ill-formed option #i
        }
    }
    return argc; // there were no non-option arguments
}

inline void handle_common_args(mapnik::parameters const& params)
{
    if (auto severity = params.get<std::string>("log"))
    {
        if (*severity == "debug")
            mapnik::logger::set_severity(mapnik::logger::debug);
        else if (*severity == "warn")
            mapnik::logger::set_severity(mapnik::logger::warn);
        else if (*severity == "error")
            mapnik::logger::set_severity(mapnik::logger::error);
        else if (*severity == "none")
            mapnik::logger::set_severity(mapnik::logger::none);
        else
            std::clog << "ignoring option --log='" << *severity << "' (allowed values are: debug, warn, error, none)\n";
    }
}

inline int handle_args(int argc, char** argv, mapnik::parameters& params)
{
    int res = parse_args(argc, argv, params);
    handle_common_args(params);
    return res;
}

#define BENCHMARK(test_class, name)                                                                                    \
    int main(int argc, char** argv)                                                                                    \
    {                                                                                                                  \
        try                                                                                                            \
        {                                                                                                              \
            mapnik::parameters params;                                                                                 \
            benchmark::handle_args(argc, argv, params);                                                                \
            test_class test_runner(params);                                                                            \
            auto result = run(test_runner, name);                                                                      \
            testing::run_cleanup();                                                                                    \
            return result;                                                                                             \
        }                                                                                                              \
        catch (std::exception const& ex)                                                                               \
        {                                                                                                              \
            std::clog << ex.what() << "\n";                                                                            \
            testing::run_cleanup();                                                                                    \
            return -1;                                                                                                 \
        }                                                                                                              \
    }

struct big_number_fmt
{
    int w;
    double v;
    const char* u;

    big_number_fmt(int width, double value, int base = 1000)
        : w(width)
        , v(value)
        , u("")
    {
        static const char* suffixes = "\0\0k\0M\0G\0T\0P\0E\0Z\0Y\0\0";
        u = suffixes;

        while (v > 1 && std::log10(std::round(v)) >= width && u[2])
        {
            v /= base;
            u += 2;
        }

        // adjust width for proper alignment without suffix
        w += (u == suffixes);
    }
};

template<typename T>
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
        auto num_iters = test_runner.iterations();
        auto num_threads = test_runner.threads();
        auto total_iters = 0;

        if (num_threads > 0)
        {
            std::mutex mtx_ready;
            std::unique_lock<std::mutex> lock_ready(mtx_ready);

            auto stub = [&](T const& test_copy) {
                // workers will wait on this mutex until the main thread
                // constructs all of them and starts measuring time
                std::unique_lock<std::mutex> my_lock(mtx_ready);
                my_lock.unlock();
                test_copy();
            };

            std::vector<std::thread> tg;
            tg.reserve(num_threads);
            for (auto i = num_threads; i-- > 0;)
            {
                tg.emplace_back(stub, test_runner);
            }
            start = std::chrono::high_resolution_clock::now();
            lock_ready.unlock();
            // wait for all workers to finish
            for (auto& t : tg)
            {
                if (t.joinable())
                    t.join();
            }
            elapsed = std::chrono::high_resolution_clock::now() - start;
            // this is actually per-thread count, not total, but I think
            // reporting average 'iters/thread/second' is more useful
            // than 'iters/second' multiplied by the number of threads
            total_iters += num_iters;
        }
        else
        {
            start = std::chrono::high_resolution_clock::now();
            do
            {
                test_runner();
                elapsed = std::chrono::high_resolution_clock::now() - start;
                total_iters += num_iters;
            } while (elapsed < min_duration);
        }

        char msg[200];
        double dur_total = milliseconds<double>(elapsed).count();
        auto elapsed_nonzero = std::max(elapsed, decltype(elapsed){1});
        big_number_fmt itersf(4, total_iters);
        big_number_fmt ips(5, total_iters / seconds<double>(elapsed_nonzero).count());

        std::clog << std::left << std::setw(43) << name;
        std::clog << std::resetiosflags(std::ios::adjustfield);
        if (num_threads > 0)
        {
            std::clog << ' ' << std::setw(3) << num_threads << " worker" << (num_threads > 1 ? "s" : " ");
        }
        else
        {
            std::clog << " main thread";
        }
        std::snprintf(msg,
                      sizeof(msg),
                      " %*.0f%s iters %6.0f milliseconds %*.0f%s i/t/s\n",
                      itersf.w,
                      itersf.v,
                      itersf.u,
                      dur_total,
                      ips.w,
                      ips.v,
                      ips.u);
        std::clog << msg;
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

    int done() const { return exit_code_; }

    template<typename Test, typename... Args>
    sequencer& run(std::string const& name, Args&&... args)
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

} // namespace benchmark

#endif // MAPNIK_BENCH_FRAMEWORK_HPP
