#include "catch.hpp"
#include <mapnik/text/icu_shaper.hpp>
#include <mapnik/text/harfbuzz_shaper.hpp>
#include <mapnik/text/font_library.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/util/from_u8string.hpp>

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <vector>

namespace {

using mapnik::util::from_u8string;

void test_shaping(mapnik::font_set const& fontset,
                  mapnik::face_manager& fm,
                  std::vector<std::pair<unsigned, unsigned>> const& expected,
                  char const* str,
                  mapnik::font_feature_settings const& ff_settings = mapnik::font_feature_settings(),
                  bool debug = false)
{
    mapnik::transcoder tr("utf8");
    std::map<unsigned, double> width_map;
    mapnik::text_itemizer itemizer;
    auto props = std::make_unique<mapnik::detail::evaluated_format_properties>();
    props->fontset = fontset;
    props->text_size = 32;
    props->ff_settings = ff_settings;

    double scale_factor = 1;
    auto ustr = tr.transcode(str);
    auto length = ustr.length();
    itemizer.add_text(ustr, props);

    mapnik::text_line line(0, length);
    mapnik::harfbuzz_shaper::shape_text(line, itemizer, width_map, fm, scale_factor, "");

    std::size_t index = 0;
    for (auto const& g : line)
    {
        if (debug)
        {
            if (index++ > 0)
                std::cerr << ",";
            std::cerr << "{" << g.glyph_index << ", "
                      << g.char_index
                      //<< ", " << g.face->family_name() << ":" << g.face->style_name()
                      << "}";
        }
        else
        {
            unsigned glyph_index, char_index;
            CHECK(index < expected.size());
            std::tie(glyph_index, char_index) = expected[index++];
            REQUIRE(glyph_index == g.glyph_index);
            REQUIRE(char_index == g.char_index);
        }
    }
}

std::string resolve_face_name(std::string const& face_name_prefix)
{
    auto const& registered_face_names = mapnik::freetype_engine::face_names();
    auto exact_itr = std::find(registered_face_names.begin(), registered_face_names.end(), face_name_prefix);
    if (exact_itr != registered_face_names.end())
        return *exact_itr;

    auto prefix_itr = std::find_if(
      registered_face_names.begin(), registered_face_names.end(), [&](std::string const& registered_face_name) {
          return registered_face_name.starts_with(face_name_prefix + " ");
      });
    REQUIRE(prefix_itr != registered_face_names.end());
    return *prefix_itr;
}

mapnik::font_set make_fontset(std::string const& name, std::initializer_list<std::string> face_names)
{
    mapnik::font_set fontset(name);
    for (auto const& face_name : face_names)
    {
        fontset.add_face_name(resolve_face_name(face_name));
    }
    return fontset;
}

struct face_span
{
    unsigned start;
    unsigned end;
    std::string face_name;
};

struct shaped_glyph_expectation
{
    unsigned glyph_index;
    unsigned char_index;
    double advance;
    std::string face_name;
};

void test_shaping_faces(mapnik::font_set const& fontset,
                        mapnik::face_manager& fm,
                        std::vector<unsigned> const& expected_char_indices,
                        char const* str,
                        std::string const& expected_face_name)
{
    mapnik::transcoder tr("utf8");
    std::map<unsigned, double> width_map;
    mapnik::text_itemizer itemizer;
    auto props = std::make_unique<mapnik::detail::evaluated_format_properties>();
    props->fontset = fontset;
    props->text_size = 32;

    auto ustr = tr.transcode(str);
    mapnik::text_line line(0, ustr.length());
    itemizer.add_text(ustr, props);
    mapnik::harfbuzz_shaper::shape_text(line, itemizer, width_map, fm, 1.0, "");

    std::vector<unsigned> actual_char_indices;
    for (auto const& g : line)
    {
        REQUIRE(g.glyph_index != 0);
        actual_char_indices.push_back(g.char_index);
        REQUIRE(g.face);
        REQUIRE(g.face->family_name() == expected_face_name);
    }
    if (actual_char_indices != expected_char_indices)
    {
        std::ostringstream s;
        for (std::size_t i = 0; i < actual_char_indices.size(); ++i)
        {
            if (i > 0)
                s << ", ";
            s << actual_char_indices[i];
        }
        INFO("actual char indices: [" << s.str() << "]");
    }
    REQUIRE(actual_char_indices == expected_char_indices);
}

void test_shaping_face_for_range(mapnik::font_set const& fontset,
                                 mapnik::face_manager& fm,
                                 char const* str,
                                 unsigned first_char_index,
                                 unsigned last_char_index,
                                 std::string const& expected_face_name,
                                 bool require_shaped_glyph = true)
{
    mapnik::transcoder tr("utf8");
    std::map<unsigned, double> width_map;
    mapnik::text_itemizer itemizer;
    auto props = std::make_unique<mapnik::detail::evaluated_format_properties>();
    props->fontset = fontset;
    props->text_size = 32;

    auto ustr = tr.transcode(str);
    mapnik::text_line line(0, ustr.length());
    itemizer.add_text(ustr, props);
    mapnik::harfbuzz_shaper::shape_text(line, itemizer, width_map, fm, 1.0, "");

    bool found = false;
    for (auto const& g : line)
    {
        if (require_shaped_glyph)
            REQUIRE(g.glyph_index != 0);
        REQUIRE(g.face);
        if (g.char_index >= first_char_index && g.char_index < last_char_index)
        {
            found = true;
            REQUIRE(g.face->family_name() == expected_face_name);
        }
    }
    REQUIRE(found);
}

void test_shaping_face_spans(mapnik::font_set const& fontset,
                             mapnik::face_manager& fm,
                             char const* str,
                             std::vector<face_span> const& expected,
                             bool require_shaped_glyph = true)
{
    mapnik::transcoder tr("utf8");
    std::map<unsigned, double> width_map;
    mapnik::text_itemizer itemizer;
    auto props = std::make_unique<mapnik::detail::evaluated_format_properties>();
    props->fontset = fontset;
    props->text_size = 32;

    auto ustr = tr.transcode(str);
    auto length = static_cast<unsigned>(ustr.length());
    mapnik::text_line line(0, ustr.length());
    itemizer.add_text(ustr, props);
    mapnik::harfbuzz_shaper::shape_text(line, itemizer, width_map, fm, 1.0, "");

    struct cluster_face
    {
        unsigned char_index;
        std::string face_name;
    };

    std::vector<cluster_face> cluster_faces;
    for (auto const& g : line)
    {
        if (require_shaped_glyph)
            REQUIRE(g.glyph_index != 0);
        REQUIRE(g.face);

        std::string face_name = g.face->family_name();
        if (!cluster_faces.empty() && cluster_faces.back().char_index == g.char_index)
        {
            REQUIRE(cluster_faces.back().face_name == face_name);
        }
        else
        {
            cluster_faces.push_back({g.char_index, std::move(face_name)});
        }
    }

    REQUIRE(!cluster_faces.empty());
    bool rtl = cluster_faces.size() > 1 && cluster_faces.front().char_index > cluster_faces.back().char_index;
    std::vector<std::string> face_by_char(length);
    for (std::size_t i = 0; i < cluster_faces.size(); ++i)
    {
        unsigned start = cluster_faces[i].char_index;
        unsigned end = 0;
        if (!rtl)
            end = (i + 1 < cluster_faces.size()) ? cluster_faces[i + 1].char_index : length;
        else
            end = (i == 0) ? length : cluster_faces[i - 1].char_index;

        for (unsigned char_index = start; char_index < end; ++char_index)
        {
            if (face_by_char[char_index].empty())
                face_by_char[char_index] = cluster_faces[i].face_name;
            else
                REQUIRE(face_by_char[char_index] == cluster_faces[i].face_name);
        }
    }

    std::vector<face_span> actual;
    for (unsigned i = 0; i < length;)
    {
        REQUIRE(!face_by_char[i].empty());
        unsigned end = i + 1;
        while (end < length && face_by_char[end] == face_by_char[i])
            ++end;
        actual.push_back({i, end, face_by_char[i]});
        i = end;
    }

    if (actual.size() != expected.size())
    {
        std::ostringstream s;
        for (auto const& span : actual)
        {
            if (s.tellp() > 0)
                s << ", ";
            s << "[" << span.start << "," << span.end << ") " << span.face_name;
        }
        INFO("actual face spans: [" << s.str() << "]");
    }
    REQUIRE(actual.size() == expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i)
    {
        CHECK(actual[i].start == expected[i].start);
        CHECK(actual[i].end == expected[i].end);
        CHECK(actual[i].face_name == expected[i].face_name);
    }
}

void test_shaping_glyph_stream(mapnik::font_set const& fontset,
                               mapnik::face_manager& fm,
                               char const* str,
                               std::vector<shaped_glyph_expectation> const& expected,
                               double text_size = 16.0)
{
    mapnik::transcoder tr("utf8");
    std::map<unsigned, double> width_map;
    mapnik::text_itemizer itemizer;
    auto props = std::make_unique<mapnik::detail::evaluated_format_properties>();
    props->fontset = fontset;
    props->text_size = text_size;

    auto ustr = tr.transcode(str);
    mapnik::text_line line(0, ustr.length());
    itemizer.add_text(ustr, props);
    mapnik::harfbuzz_shaper::shape_text(line, itemizer, width_map, fm, 1.0, "");

    std::vector<shaped_glyph_expectation> actual;
    for (auto const& g : line)
    {
        REQUIRE(g.face);
        actual.push_back({g.glyph_index, g.char_index, g.advance(), g.face->family_name()});
    }

    if (actual.size() != expected.size())
    {
        std::ostringstream s;
        s << std::fixed << std::setprecision(3);
        for (auto const& glyph : actual)
        {
            if (s.tellp() > 0)
                s << ", ";
            s << "{" << glyph.glyph_index << ", " << glyph.char_index << ", " << glyph.advance << ", " << glyph.face_name
              << "}";
        }
        INFO("actual glyph stream: [" << s.str() << "]");
    }
    REQUIRE(actual.size() == expected.size());

    for (std::size_t i = 0; i < expected.size(); ++i)
    {
        CHECK(actual[i].glyph_index == expected[i].glyph_index);
        CHECK(actual[i].char_index == expected[i].char_index);
        CHECK(actual[i].advance == Approx(expected[i].advance).margin(0.001));
        CHECK(actual[i].face_name == expected[i].face_name);
    }
}

} // namespace

TEST_CASE("font coverage iteration")
{
    SECTION("ltr and rtl emit each covered cluster once")
    {
        mapnik::detail::font_coverage coverage(0, 8);
        coverage.cover(0, 2, 10);
        coverage.cover(2, 5, 11);
        coverage.cover(5, 8, 12);

        std::vector<std::size_t> ltr_indices;
        for (auto itr = coverage.covering_begin(false); itr != coverage.covering_end(); ++itr)
            ltr_indices.push_back(*itr);
        REQUIRE(ltr_indices == std::vector<std::size_t>{10, 11, 12});

        std::vector<std::size_t> rtl_indices;
        for (auto itr = coverage.covering_begin(true); itr != coverage.covering_end(); ++itr)
            rtl_indices.push_back(*itr);
        REQUIRE(rtl_indices == std::vector<std::size_t>{12, 11, 10});
    }

    SECTION("rtl skips uncovered holes and repeated indices")
    {
        mapnik::detail::font_coverage coverage(0, 10);
        coverage.cover(0, 3, 20);
        coverage.cover(5, 7, 21);
        coverage.cover(7, 10, 22);

        std::vector<std::size_t> rtl_indices;
        for (auto itr = coverage.covering_begin(true); itr != coverage.covering_end(); ++itr)
            rtl_indices.push_back(*itr);
        REQUIRE(rtl_indices == std::vector<std::size_t>{22, 21, 20});
    }

    SECTION("uncovered ranges merge in logical order")
    {
        mapnik::detail::font_coverage coverage(0, 10);
        auto initial = coverage.pop_current_uncovered_front();
        REQUIRE(initial.start == 0);
        REQUIRE(initial.end == 10);

        coverage.push_uncovered(2, 4);
        coverage.push_uncovered(4, 6);
        coverage.push_uncovered(6, 8);
        coverage.advance_uncovered_ranges();

        REQUIRE(coverage.has_current_uncovered());
        auto merged = coverage.pop_current_uncovered_front();
        REQUIRE(merged.start == 2);
        REQUIRE(merged.end == 8);
        REQUIRE(!coverage.has_current_uncovered());
    }
}

TEST_CASE("shaping")
{
    mapnik::freetype_engine::register_font("test/data/fonts/NotoSans-Regular.ttc");
    mapnik::freetype_engine::register_fonts("test/data/fonts/Noto");
    mapnik::font_set fontset("fontset");
    for (auto const& name : mapnik::freetype_engine::face_names())
    {
        fontset.add_face_name(name);
    }

    mapnik::font_library fl;
    mapnik::freetype_engine::font_file_mapping_type font_file_mapping;
    mapnik::freetype_engine::font_memory_cache_type font_memory_cache;
    mapnik::face_manager fm(fl, font_file_mapping, font_memory_cache);

    {
        std::vector<std::pair<unsigned, unsigned>> expected =
          {{977, 0}, {1094, 3}, {1038, 4}, {1168, 4}, {9, 7}, {3, 8}, {11, 9}, {68, 10}, {69, 11}, {70, 12}, {12, 13}};
        test_shaping(fontset, fm, expected, from_u8string(u8"སྤུ་ཧྲེང (abc)").c_str());
    }

    {
        std::vector<std::pair<unsigned, unsigned>> expected =
          {{977, 0}, {1094, 3}, {1038, 4}, {1168, 4}, {9, 7}, {3, 8}, {11, 9}, {0, 10}, {0, 11}, {0, 12}, {12, 13}};
        test_shaping(fontset, fm, expected, from_u8string(u8"སྤུ་ཧྲེང (普兰镇)").c_str());
    }

    {
        std::vector<std::pair<unsigned, unsigned>> expected =
          {{68, 0}, {69, 1}, {70, 2}, {3, 3}, {11, 4}, {0, 5}, {0, 6}, {0, 7}, {12, 8}};
        test_shaping(fontset, fm, expected, from_u8string(u8"abc (普兰镇)").c_str());
    }

    {
        std::vector<std::pair<unsigned, unsigned>> expected =
          {{68, 0}, {69, 1}, {70, 2}, {3, 3}, {11, 4}, {68, 5}, {69, 6}, {70, 7}, {12, 8}};
        test_shaping(fontset, fm, expected, "abc (abc)");
    }

    {
        //    "ⵃⴰⵢ ⵚⵉⵏⴰⵄⵉ الحي الصناعي"
        std::vector<std::pair<unsigned, unsigned>> expected = {
          {0, 0},    {0, 1},   {0, 2},  {3, 3},    {0, 4},    {0, 5},    {0, 6},    {0, 7},
          {0, 8},    {0, 9},   {3, 10}, {509, 22}, {481, 21}, {438, 20}, {503, 19}, {470, 18},
          {496, 17}, {43, 16}, {3, 15}, {509, 14}, {454, 13}, {496, 12}, {43, 11}};
        test_shaping(fontset, fm, expected, from_u8string(u8"ⵃⴰⵢ ⵚⵉⵏⴰⵄⵉ الحي الصناعي").c_str());
    }
}

TEST_CASE("ligature shaping")
{
    REQUIRE(mapnik::freetype_engine::register_font("test/data/fonts/glukfonts/Foglihten-068.otf"));

    mapnik::font_set fontset("ligature-fontset");
    fontset.add_face_name(resolve_face_name("Foglihten"));

    mapnik::font_library fl;
    mapnik::freetype_engine::font_file_mapping_type font_file_mapping;
    mapnik::freetype_engine::font_memory_cache_type font_memory_cache;
    mapnik::face_manager fm(fl, font_file_mapping, font_memory_cache);

    SECTION("default ligatures shape office with ffi")
    {
        std::vector<std::pair<unsigned, unsigned>> expected = {{80, 0}, {861, 1}, {68, 4}, {70, 5}};
        test_shaping(fontset, fm, expected, "office");
    }

    SECTION("liga off keeps ffi split")
    {
        mapnik::font_feature_settings ff_settings;
        ff_settings.append(mapnik::font_feature_liga_off);
        std::vector<std::pair<unsigned, unsigned>> expected = {{568, 0}, {71, 1}, {74, 2}};
        test_shaping(fontset, fm, expected, "ffi", ff_settings);
    }
}

TEST_CASE("font fallback shaping")
{
    REQUIRE(mapnik::freetype_engine::register_fonts("test/data/fonts", true));

    REQUIRE(mapnik::freetype_engine::register_font("fonts/noto/NotoSansSinhala-Regular.ttf"));
    REQUIRE(mapnik::freetype_engine::register_font("fonts/fallback/BenestroffSansTest-Regular.ttf"));
    REQUIRE(mapnik::freetype_engine::register_font("fonts/fallback/BenestroffSymbolsTest-Regular.ttf"));
    REQUIRE(mapnik::freetype_engine::register_font("fonts/fallback/FallbackRegularTest-Regular.ttf"));
    REQUIRE(mapnik::freetype_engine::register_font("fonts/fallback/BengaliJoinerTest-Regular.ttf"));
    REQUIRE(mapnik::freetype_engine::register_font("fonts/fallback/KhmerFallbackProbe.ttf"));
    REQUIRE(mapnik::freetype_engine::register_font("fonts/fallback/KhmerFallbackFull.ttf"));
    REQUIRE(mapnik::freetype_engine::register_font("fonts/fallback/MongolianProblemTest-Regular.ttf"));
    REQUIRE(mapnik::freetype_engine::register_font("fonts/fallback/MyanmarFallbackProbe.ttf"));
    REQUIRE(mapnik::freetype_engine::register_font("fonts/fallback/MyanmarFallbackFull.ttf"));

    mapnik::font_library fl;
    mapnik::freetype_engine::font_file_mapping_type font_file_mapping;
    mapnik::freetype_engine::font_memory_cache_type font_memory_cache;
    mapnik::face_manager fm(fl, font_file_mapping, font_memory_cache);

    SECTION("tibetan falls back from font awesome")
    {
        auto fontset = make_fontset("tibetan-fallback", {"FontAwesome", "Noto Sans Tibetan"});
        std::vector<unsigned> expected_char_indices = {0, 3, 4, 4, 7};
        test_shaping_faces(fontset, fm, expected_char_indices, from_u8string(u8"སྤུ་ཧྲེང").c_str(), "Noto Sans Tibetan");
    }

    SECTION("sinhala cluster falls back past arabic font")
    {
        auto fontset = make_fontset("sinhala-fallback", {"Noto Sans", "XB Zar", "Noto Sans Sinhala"});
        test_shaping_face_for_range(
          fontset, fm, from_u8string(u8"Hello ශ්‍රී ලංකා").c_str(), 6, 10, "Noto Sans Sinhala");
    }

    SECTION("arabic rtl text falls back and keeps rtl cluster order")
    {
        auto fontset = make_fontset("arabic-rtl-fallback", {"Noto Sans", "Noto Naskh Arabic"});
        test_shaping_faces(fontset, fm, {4, 3, 2, 1, 0}, from_u8string(u8"مرحبا").c_str(), "Noto Naskh Arabic");
    }

    SECTION("bus stop label keeps ligature cluster on regular face")
    {
        // Derived from the OSM Carto bus-stop label "Bénestroff − Centre".
        auto fontset = make_fontset(
          "benestroff-bus-stop-fallback", {"Benestroff Sans Test", "Benestroff Symbols Test"});
        auto text = from_u8string(u8"Bénestroff − Centre");
        test_shaping_face_spans(fontset,
                                fm,
                                text.c_str(),
                                {{0, 11, "Benestroff Sans Test"},
                                 {11, 12, "Benestroff Symbols Test"},
                                 {12, 19, "Benestroff Sans Test"}});
    }

    SECTION("khmer stacked cluster falls back past partial coverage")
    {
        // Derived from Khmer stacked-cluster cases used in HarfBuzz shaping tests.
        auto fontset = make_fontset("khmer-stacked-fallback", {"Khmer Fallback Probe", "Khmer Fallback Full"});
        test_shaping_face_for_range(fontset, fm, from_u8string(u8"ប្គា").c_str(), 0, 4, "Khmer Fallback Full");
    }

    SECTION("khmer stacked fallback interleaves accepted ranges")
    {
        auto fontset = make_fontset("khmer-interleaved-fallback", {"Khmer Fallback Probe", "Khmer Fallback Full"});
        auto text = from_u8string(u8"បប្គប");
        test_shaping_face_spans(
          fontset, fm, text.c_str(), {{0, 1, "Khmer Fallback Probe"}, {1, 3, "Khmer Fallback Full"}, {3, 5, "Khmer Fallback Probe"}});
    }

    SECTION("khmer mark cluster falls back past partial coverage")
    {
        // Derived from Khmer mark-heavy cluster cases used in HarfBuzz shaping tests.
        auto fontset = make_fontset("khmer-zwj-fallback", {"Khmer Fallback Probe", "Khmer Fallback Full"});
        test_shaping_face_for_range(fontset, fm, from_u8string(u8"ភ៊ឹ").c_str(), 0, 3, "Khmer Fallback Full");
    }

    SECTION("myanmar kinzi cluster falls back past partial coverage")
    {
        // Derived from Myanmar kinzi-style clusters used in HarfBuzz shaping tests.
        auto fontset = make_fontset("myanmar-stacked-fallback", {"Myanmar Fallback Probe", "Myanmar Fallback Full"});
        test_shaping_face_for_range(fontset, fm, from_u8string(u8"င်္ခ").c_str(), 0, 4, "Myanmar Fallback Full");
    }

    SECTION("last face keeps uncovered tofu cluster")
    {
        auto fontset = make_fontset("last-face-tofu", {"FontAwesome"});
        auto text = from_u8string(u8"普兰镇");
        mapnik::transcoder tr("utf8");
        auto const text_length = static_cast<unsigned>(tr.transcode(text.c_str()).length());
        test_shaping_face_spans(fontset, fm, text.c_str(), {{0, text_length, "FontAwesome"}}, false);
    }
}

TEST_CASE("known fallback shaping regressions", "[!mayfail]")
{
    REQUIRE(mapnik::freetype_engine::register_fonts("test/data/fonts", true));

    REQUIRE(mapnik::freetype_engine::register_font("fonts/fallback/FallbackRegularTest-Regular.ttf"));
    REQUIRE(mapnik::freetype_engine::register_font("fonts/fallback/BengaliJoinerTest-Regular.ttf"));
    REQUIRE(mapnik::freetype_engine::register_font("fonts/fallback/MongolianProblemTest-Regular.ttf"));

    mapnik::font_library fl;
    mapnik::freetype_engine::font_file_mapping_type font_file_mapping;
    mapnik::freetype_engine::font_memory_cache_type font_memory_cache;
    mapnik::face_manager fm(fl, font_file_mapping, font_memory_cache);

    SECTION("bengali joiner keeps upstream substitution")
    {
        auto fontset = make_fontset("bengali-joiner-fallback", {"Fallback Regular Test", "Bengali Joiner Test"});
        auto text = from_u8string(u8"দুশান্‌বে");
        test_shaping_glyph_stream(
          fontset,
          fm,
          text.c_str(),
          {{37, 0, 9.648, "Bengali Joiner Test"},
           {57, 0, 0.0, "Bengali Joiner Test"},
           {48, 2, 10.832, "Bengali Joiner Test"},
           {54, 2, 4.256, "Bengali Joiner Test"},
           {39, 4, 9.664, "Bengali Joiner Test"},
           {64, 4, 0.0, "Bengali Joiner Test"},
           {3, 6, 0.0, "Fallback Regular Test"},
           {60, 7, 5.536, "Bengali Joiner Test"},
           {42, 7, 9.536, "Bengali Joiner Test"}});
    }

    SECTION("mongolian cluster keeps upstream substitutions")
    {
        auto fontset = make_fontset("mongolian-problem-fallback", {"Fallback Regular Test", "Mongolian Problem Test"});
        auto text = from_u8string(u8"᠑᠐ ᠳᠦᠭᠡᠷ ᠪᠠᠭ");
        test_shaping_glyph_stream(
          fontset,
          fm,
          text.c_str(),
          {{921, 0, 9.152, "Mongolian Problem Test"},
           {915, 1, 9.152, "Mongolian Problem Test"},
           {1542, 2, 2.656, "Fallback Regular Test"},
           {36, 3, 10.768, "Mongolian Problem Test"},
           {940, 4, 9.168, "Mongolian Problem Test"},
           {506, 5, 10.656, "Mongolian Problem Test"},
           {31, 7, 8.064, "Mongolian Problem Test"},
           {3, 8, 4.16, "Fallback Regular Test"},
           {91, 9, 13.808, "Mongolian Problem Test"},
           {501, 11, 10.608, "Mongolian Problem Test"}});
    }
}
