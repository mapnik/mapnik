#include "catch.hpp"

#include <mapnik/util/char_array_buffer.hpp>
#include <istream>

TEST_CASE("char_array_buffer")
{
    SECTION("std::istream seekg, tellg")
    {
        std::size_t const buffer_size = 66;
        char buffer[buffer_size];
        mapnik::util::char_array_buffer array_buff(buffer, buffer_size);
        std::istream stream(&array_buff);

        CHECK(stream.seekg(0).tellg() == 0);
        CHECK(stream.seekg(buffer_size).tellg() == buffer_size);
        CHECK(stream.seekg(70).tellg() == buffer_size);
    }
}
