include(CheckCXXSourceRuns)

function(check_boost_regex)
    set(CMAKE_REQUIRED_LIBRARIES ICU::uc ICU::data ICU::i18n Boost::headers Boost::regex)
    check_cxx_source_runs([[
    #include <boost/regex/icu.hpp>
    #include <unicode/unistr.h>
    int main()
    {
        U_NAMESPACE_QUALIFIER UnicodeString ustr;
        try {
            boost::u32regex pattern = boost::make_u32regex(ustr);
        }
        // an exception is fine, still indicates support is
        // likely compiled into regex
        catch (...) {
            return 0;
        }
        return 0;
    }
    ]] BOOST_REGEX_HAS_ICU)
    set(BOOST_REGEX_HAS_ICU ${BOOST_REGEX_HAS_ICU} PARENT_SCOPE)
endfunction(check_boost_regex)
