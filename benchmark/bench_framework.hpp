#ifndef __MAPNIK_BENCH_FRAMEWORK_HPP__
#define __MAPNIK_BENCH_FRAMEWORK_HPP__

// mapnik
#include <mapnik/params.hpp>
#include <mapnik/value_types.hpp>

// stl
#include <chrono>
#include <iomanip>
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
         threads_(*params.get<mapnik::value_integer>("threads",0)),
         iterations_(*params.get<mapnik::value_integer>("iterations",0))
         {}
    std::size_t threads() const
    {
        return threads_;
    }
    std::size_t iterations() const
    {
        return iterations_;
    }
    virtual bool validate() const = 0;
    virtual void operator()() const = 0;
    virtual ~test_case() {}
};

void handle_args(int argc, char** argv, mapnik::parameters & params)
{
    if (argc > 0) {
        for (int i=1;i<argc;++i) {
            std::string opt(argv[i]);
            // parse --foo bar
            if (!opt.empty() && (opt.find("--") != 0)) {
                std::string key = std::string(argv[i-1]);
                if (!key.empty() && (key.find("--") == 0)) {
                    key = key.substr(key.find_first_not_of("-"));
                    params[key] = opt;
                }
            }
        }
    }
}

#define BENCHMARK(test_class,name)                      \
    int main(int argc, char** argv)                     \
    {                                                   \
        try                                             \
        {                                               \
            mapnik::parameters params;                  \
            benchmark::handle_args(argc,argv,params);   \
            test_class test_runner(params);             \
            return run(test_runner,name);               \
        }                                               \
        catch (std::exception const& ex)                \
        {                                               \
            std::clog << ex.what() << "\n";             \
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
            return -1;
        }
        std::chrono::high_resolution_clock::time_point start;
        std::chrono::high_resolution_clock::duration elapsed;
        std::stringstream s;
        s << name << ":"
            << std::setw(45 - (int)s.tellp()) << std::right
            << " t:" << test_runner.threads()
            << " i:" << test_runner.iterations();
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
        }
        else
        {
            start = std::chrono::high_resolution_clock::now();
            test_runner();
            elapsed = std::chrono::high_resolution_clock::now() - start;
        }
        s << std::setw(65 - (int)s.tellp()) << std::right
            << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << " milliseconds\n";
        std::clog << s.str();
        return 0;
    }
    catch (std::exception const& ex)
    {
        std::clog << "test runner did not complete: " << ex.what() << "\n";
        return -1;
    }
    return 0;
}

}

#endif // __MAPNIK_BENCH_FRAMEWORK_HPP__
