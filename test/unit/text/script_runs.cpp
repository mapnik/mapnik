#include "catch.hpp"
#include <unicode/unistr.h>
#include <mapnik/unicode.hpp>
#include <mapnik/text/scrptrun.hpp>
#include <utility>
#include <tuple>
#include <vector>

namespace {

using script_run = std::tuple<int32_t, int32_t, UScriptCode>;

std::vector<script_run> collect_script_runs(mapnik::value_unicode_string const& text)
{
    ScriptRun runs(text.getBuffer(), text.length());
    std::vector<script_run> actual;
    while (runs.next())
    {
        actual.emplace_back(runs.getScriptStart(), runs.getScriptEnd(), runs.getScriptCode());
    }
    return actual;
}

void check_script_runs(char16_t const* text, std::initializer_list<script_run> expected)
{
    mapnik::value_unicode_string value(text);
    REQUIRE(collect_script_runs(value) == std::vector<script_run>(expected));
}

} // namespace

TEST_CASE("nested script runs")
{
    mapnik::value_unicode_string text(u"Nested text runs(первый(second(третий)))"); // mixed scripts
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
    REQUIRE(std::cmp_equal(size, text.length()));
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
    REQUIRE(std::cmp_equal(size, text.length()));
}

TEST_CASE("script runs for mixed Japanese punctuation")
{
    check_script_runs(u"#愛す。",
                      {
                        {0, 2, USCRIPT_HAN},
                        {2, 4, USCRIPT_HIRAGANA},
                      });
}

TEST_CASE("script runs for telugu followed by latin in parentheses")
{
    check_script_runs(u"(ఆ a)",
                      {
                        {0, 3, USCRIPT_TELUGU},
                        {3, 4, USCRIPT_LATIN},
                        {4, 5, USCRIPT_TELUGU},
                      });
}

TEST_CASE("script runs for repeated empty parentheses around latin")
{
    check_script_runs(u"()()()x()", {{0, 8, USCRIPT_LATIN}});
}

TEST_CASE("script runs for latin followed by sinhala with latin in parentheses")
{
    check_script_runs(u"Hello ශ්‍රී ලංකා (abc)",
                      {
                        {0, 6, USCRIPT_LATIN},
                        {6, 18, USCRIPT_SINHALA},
                        {18, 21, USCRIPT_LATIN},
                        {21, 22, USCRIPT_SINHALA},
                      });
}
