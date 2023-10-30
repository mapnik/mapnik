#include "catch.hpp"
#include <unicode/unistr.h>
#include <mapnik/unicode.hpp>
#include <mapnik/text/scrptrun.hpp>

TEST_CASE("nested script runs")
{
    //mapnik::value_unicode_string text(u"Nested text runs(первый(second(третий)))"); // mixed scripts
    mapnik::value_unicode_string text = UnicodeString::fromUTF8("Nested text runs(первый(second(третий)))");
    ScriptRun runs(text.getBuffer(), text.length());
    std::size_t count = 0;
    std::size_t size = 0;
    while (runs.next())
    {
        if (count & 1)
            CHECK(runs.getScriptCode() == USCRIPT_CYRILLIC);
        else
            CHECK(runs.getScriptCode() == USCRIPT_LATIN);
        size += runs.getScriptEnd() - runs.getScriptStart();
        ++count;
    }
    REQUIRE(count == 7);
    REQUIRE(size == text.length());
}

TEST_CASE("many punctuation chars")
{
    mapnik::value_unicode_string text((std::string(791, '(') + "test").data());
    ScriptRun runs(text.getBuffer(), text.length());
    while (runs.next())
    {
        CHECK(runs.getScriptCode() == 25);
        CHECK(runs.getScriptStart() == 0);
        CHECK(runs.getScriptEnd() == text.length());
    }
}

TEST_CASE("empty runs")
{
    mapnik::value_unicode_string text("()text");
    ScriptRun runs(text.getBuffer(), text.length());
    std::size_t count = 0;
    std::size_t size = 0;
    while (runs.next())
    {
        size += runs.getScriptEnd() - runs.getScriptStart();
        ++count;
    }
    REQUIRE(count == 1);
    REQUIRE(size == text.length());
}
