
#include "catch.hpp"
#include <mapnik/safe_cast.hpp>

#define CAST_ASSERT(numeric_type)                                  \
    using limit = std::numeric_limits<numeric_type>;                      \
    auto min_value = static_cast<std::intmax_t>(limit::min())-1;          \
    auto max_value = static_cast<std::intmax_t>(limit::max())+1;          \
    CHECK( limit::min() == mapnik::safe_cast<numeric_type>(min_value) ); \
    CHECK( limit::max() == mapnik::safe_cast<numeric_type>(max_value) ); \

#define CAST_ASSERT2(numeric_type)                                 \
    using limit = std::numeric_limits<numeric_type>;                      \
    auto min_value = static_cast<std::intmax_t>(limit::min());            \
    auto max_value = static_cast<std::intmax_t>(limit::max());            \
    CHECK( limit::min() == mapnik::safe_cast<numeric_type>(min_value) ); \
    CHECK( limit::max() == mapnik::safe_cast<numeric_type>(max_value) ); \

#define CAST_ASSERT3(numeric_type)                                 \
    using limit = std::numeric_limits<numeric_type>;                      \
    auto min_value = static_cast<std::intmax_t>(limit::min())-1;          \
    auto max_value = static_cast<std::uintmax_t>(limit::max());            \
    CHECK( limit::min() == mapnik::safe_cast<numeric_type>(min_value) ); \
    CHECK( limit::max() == mapnik::safe_cast<numeric_type>(max_value) ); \

#define CAST_ASSERT4(numeric_type)                                 \
    using limit = std::numeric_limits<numeric_type>;                      \
    auto min_value = static_cast<double>(-limit::max())-1;          \
    auto max_value = static_cast<double>(limit::max());            \
    CHECK( -limit::max() == mapnik::safe_cast<numeric_type>(min_value) ); \
    CHECK( limit::max() == mapnik::safe_cast<numeric_type>(max_value) ); \

TEST_CASE("saturated cast") {

    SECTION("int8")   { CAST_ASSERT(std::int8_t);   }
    SECTION("int16")  { CAST_ASSERT(std::int16_t);  }
    SECTION("int32")  { CAST_ASSERT(std::int32_t);  }

    SECTION("int64")  { CAST_ASSERT2(std::int64_t);  }
    SECTION("intmax") { CAST_ASSERT2(std::intmax_t); }
    SECTION("intptr") { CAST_ASSERT2(std::intptr_t); }

    SECTION("uint8")   { CAST_ASSERT(std::uint8_t);   }
    SECTION("uint16")  { CAST_ASSERT(std::uint16_t);  }
    SECTION("uint32")  { CAST_ASSERT(std::uint32_t);  }

    SECTION("uint64")  { CAST_ASSERT3(std::uint64_t);  }
    SECTION("uintmax") { CAST_ASSERT3(std::uintmax_t); }
    SECTION("uintptr") { CAST_ASSERT3(std::uintptr_t); }

    SECTION("float") { CAST_ASSERT4(float); }

    SECTION("freeform") {

        CHECK( static_cast<std::size_t>(0) == mapnik::safe_cast<std::size_t>(-1) );
        CHECK( static_cast<std::uint64_t>(0) == mapnik::safe_cast<std::uint64_t>(-1) );
        CHECK( static_cast<unsigned long long>(0) == mapnik::safe_cast<unsigned long long>(-1) );
    }

}
