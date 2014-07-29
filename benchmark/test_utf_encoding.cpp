#include "bench_framework.hpp"
#include <mapnik/unicode.hpp>
#include <mapnik/value.hpp>
#include <boost/locale.hpp>
#ifndef __linux__
#include <codecvt>

class test : public benchmark::test_case
{
    std::string utf8_;
public:
    test(mapnik::parameters const& params)
     : test_case(params),
       utf8_(u8"שלום") {}
    bool validate() const
    {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utf32conv;
        std::u32string utf32 = utf32conv.from_bytes(utf8_);
        if (utf32.size() != 4) return false;
        if (utf32[0] != 0x5e9 &&
            utf32[1] != 0x5dc &&
            utf32[2] != 0x5d5 &&
            utf32[3] != 0x5dd) return false;
        return true;
    }
    void operator()() const
    {
        std::u32string utf32;
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utf32conv;
        for (std::size_t i=0;i<iterations_;++i) {
             utf32 = utf32conv.from_bytes(utf8_);
        }
    }
};

#endif

class test2 : public benchmark::test_case
{
    std::string utf8_;
public:
    test2(mapnik::parameters const& params)
     : test_case(params),
       utf8_(u8"שלום") {}
    bool validate() const
    {
        std::u32string utf32 = boost::locale::conv::utf_to_utf<char32_t>(utf8_);
        if (utf32.size() != 4) return false;
        if (utf32[0] != 0x5e9 &&
            utf32[1] != 0x5dc &&
            utf32[2] != 0x5d5 &&
            utf32[3] != 0x5dd) return false;
        return true;
    }
    void operator()() const
    {
         std::u32string utf32;
         for (std::size_t i=0;i<iterations_;++i) {
             utf32 = boost::locale::conv::utf_to_utf<char32_t>(utf8_);
         }
    }
};

class test3 : public benchmark::test_case
{
    std::string utf8_;
public:
    test3(mapnik::parameters const& params)
     : test_case(params),
       utf8_(u8"שלום") {}
    bool validate() const
    {
        mapnik::transcoder tr_("utf-8");
        mapnik::value_unicode_string utf32 = tr_.transcode(utf8_.data(),utf8_.size());
        //std::u32string utf32 = boost::locale::conv::utf_to_utf<char32_t>(utf8_);
        if (utf32.length() != 4) return false;
        if (utf32[0] != 0x5e9 &&
            utf32[1] != 0x5dc &&
            utf32[2] != 0x5d5 &&
            utf32[3] != 0x5dd) return false;
        return true;
    }
    void operator()() const
    {
        mapnik::transcoder tr_("utf-8");
        mapnik::value_unicode_string utf32;
        for (std::size_t i=0;i<iterations_;++i) {
            utf32 = tr_.transcode(utf8_.data(),utf8_.size());
        }
    }
};

int main(int argc, char** argv)
{
    mapnik::parameters params;
    benchmark::handle_args(argc,argv,params);
#ifndef __linux__
    test test_runner(params);
    run(test_runner,"utf encode std::codecvt");
#else
    std::clog << "skipping 'utf encode std::codecvt' test since <codecvt> is not supported on __linux__\n";
#endif
    test2 test_runner2(params);
    run(test_runner2,"utf encode boost::locale");
    test3 test_runner3(params);
    return run(test_runner3,"utf encode ICU");
}
