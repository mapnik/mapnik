#include "bench_framework.hpp"
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <array>
#include <valarray>

// http://stackoverflow.com/questions/17347254/why-is-allocation-and-deallocation-of-stdvector-slower-than-dynamic-array-on-m

#define FULL_ZERO_CHECK

inline void ensure_zero(uint8_t * data, uint32_t size) {
#ifdef FULL_ZERO_CHECK
    for (std::size_t i=0;i<size;++i) {
        if (data[i] != 0) {
            throw std::runtime_error("found non zero value");
        }
    }
#else
    if (data[0] != 0) {
        throw std::runtime_error("found non zero value");
    }
#endif
}

class test1 : public benchmark::test_case
{
public:
    uint32_t size_;
    std::vector<uint8_t> array_;
    test1(mapnik::parameters const& params)
     : test_case(params),
       size_(*params.get<mapnik::value_integer>("size",256*256)),
       array_(size_,0) { }
    bool validate() const
    {
        return true;
    }
    void operator()() const
    {
         for (std::size_t i=0;i<iterations_;++i) {
             // NOTE: sizeof(uint8_t) == 1
             uint8_t *data = (uint8_t *)malloc(sizeof(uint8_t)*size_);
             memcpy(data, &array_[0], size_);
             ensure_zero(data,size_);
             free(data);
         }
    }
};

class test1b : public benchmark::test_case
{
public:
    uint32_t size_;
    std::vector<uint8_t> array_;
    test1b(mapnik::parameters const& params)
     : test_case(params),
       size_(*params.get<mapnik::value_integer>("size",256*256)),
       array_(size_,0) { }
    bool validate() const
    {
        return true;
    }
    void operator()() const
    {
         for (std::size_t i=0;i<iterations_;++i) {
             // NOTE: sizeof(uint8_t) == 1
             uint8_t *data = (uint8_t *)malloc(sizeof(uint8_t)*size_);
             memset(data, 0, sizeof(uint8_t)*size_);
             ensure_zero(data,size_);
             free(data);
         }
    }
};

class test2 : public benchmark::test_case
{
public:
    uint32_t size_;
    std::vector<uint8_t> array_;
    test2(mapnik::parameters const& params)
     : test_case(params),
       size_(*params.get<mapnik::value_integer>("size",256*256)),
       array_(size_,0) { }
    bool validate() const
    {
        return true;
    }
    void operator()() const
    {
         for (std::size_t i=0;i<iterations_;++i) {
             uint8_t * data = static_cast<uint8_t*>(::operator new(sizeof(uint8_t)*size_));
             memcpy(data, &array_[0], size_);
             ensure_zero(data,size_);
             ::operator delete(data),data=0;
         }
    }
};

class test3 : public benchmark::test_case
{
public:
    uint32_t size_;
    std::vector<uint8_t> array_;
    test3(mapnik::parameters const& params)
     : test_case(params),
       size_(*params.get<mapnik::value_integer>("size",256*256)),
       array_(size_,0) { }
    bool validate() const
    {
        return true;
    }
    void operator()() const
    {
         for (std::size_t i=0;i<iterations_;++i) {
             std::vector<uint8_t> data(size_);
             ensure_zero(&data[0],data.size());
         }
    }
};


class test3b : public benchmark::test_case
{
public:
    uint32_t size_;
    std::vector<uint8_t> array_;
    test3b(mapnik::parameters const& params)
     : test_case(params),
       size_(*params.get<mapnik::value_integer>("size",256*256)),
       array_(size_,0) { }
    bool validate() const
    {
        return true;
    }
    void operator()() const
    {
         for (std::size_t i=0;i<iterations_;++i) {
             std::vector<uint8_t> data(0);
             data.resize(size_,0);
             ensure_zero(&data[0],data.size());
         }
    }
};


class test3c : public benchmark::test_case
{
public:
    uint32_t size_;
    std::vector<uint8_t> array_;
    test3c(mapnik::parameters const& params)
     : test_case(params),
       size_(*params.get<mapnik::value_integer>("size",256*256)),
       array_(size_,0) { }
    bool validate() const
    {
        return true;
    }
    void operator()() const
    {
         for (std::size_t i=0;i<iterations_;++i) {
             std::vector<uint8_t> data(0);
             data.assign(size_,0);
             ensure_zero(&data[0],data.size());
         }
    }
};

class test4 : public benchmark::test_case
{
public:
    uint32_t size_;
    std::vector<uint8_t> array_;
    test4(mapnik::parameters const& params)
     : test_case(params),
       size_(*params.get<mapnik::value_integer>("size",256*256)),
       array_(size_,0) { }
    bool validate() const
    {
        return true;
    }
    void operator()() const
    {
         for (std::size_t i=0;i<iterations_;++i) {
             uint8_t *data = (uint8_t *)calloc(size_,sizeof(uint8_t));
             ensure_zero(data,size_);
             free(data);
         }
    }
};

class test5 : public benchmark::test_case
{
public:
    uint32_t size_;
    std::vector<uint8_t> array_;
    test5(mapnik::parameters const& params)
     : test_case(params),
       size_(*params.get<mapnik::value_integer>("size",256*256)),
       array_(size_,0) { }
    bool validate() const
    {
        return true;
    }
    void operator()() const
    {
         for (std::size_t i=0;i<iterations_;++i) {
             std::string data(array_.begin(),array_.end());
             ensure_zero((uint8_t *)&data[0],size_);
         }
    }
};

class test5b : public benchmark::test_case
{
public:
    uint32_t size_;
    std::vector<char> array_;
    test5b(mapnik::parameters const& params)
     : test_case(params),
       size_(*params.get<mapnik::value_integer>("size",256*256)),
       array_(size_,0) { }
    bool validate() const
    {
        return true;
    }
    void operator()() const
    {
         for (std::size_t i=0;i<iterations_;++i) {
             std::string data(&array_[0]);
             ensure_zero((uint8_t *)&data[0],size_);
         }
    }
};

// C++14 dynarray<T>
// http://isocpp.org/blog/2013/04/trip-report-iso-c-spring-2013-meeting
// http://lists.cs.uiuc.edu/pipermail/cfe-commits/Week-of-Mon-20130909/088700.html
// http://stackoverflow.com/questions/17303902/any-alternative-to-stddynarray-presently-available

class test6 : public benchmark::test_case
{
public:
    uint32_t size_;
    std::vector<uint8_t> array_;
    test6(mapnik::parameters const& params)
     : test_case(params),
       size_(*params.get<mapnik::value_integer>("size",256*256)),
       array_(size_,0) { }
    bool validate() const
    {
        return true;
    }
    void operator()() const
    {
         for (std::size_t i=0;i<iterations_;++i) {
             std::valarray<uint8_t> data(size_,0);
             ensure_zero(&data[0],size_);
         }
    }
};

int main(int argc, char** argv)
{
    mapnik::parameters params;
    benchmark::handle_args(argc,argv,params);
    {
        test4 test_runner4(params);
        run(test_runner4,"calloc");
    }
    {
        test1 test_runner(params);
        run(test_runner,"malloc/memcpy");
    }
    {
        test1b test_runner(params);
        run(test_runner,"malloc/memset");

    }
    {
        test2 test_runner(params);
        run(test_runner,"new");
    }
    {
        test3 test_runner(params);
        run(test_runner,"vector(N)");
    }
    {
        test3b test_runner(params);
        run(test_runner,"vector/resize");
    }
    {
        test3c test_runner(params);
        run(test_runner,"vector/assign");
    }
    {
        test5 test_runner(params);
        run(test_runner,"std::string range");
    }
    {
        test5 test_runner(params);
        run(test_runner,"std::string &[0]");
    }
    {
        test5 test_runner(params);
        run(test_runner,"valarray");
    }
    return 0;
}
