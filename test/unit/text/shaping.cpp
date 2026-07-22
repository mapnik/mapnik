#include "catch.hpp"
#include <mapnik/text/icu_shaper.hpp>
#include <mapnik/text/harfbuzz_shaper.hpp>
#include <mapnik/text/font_library.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/util/from_u8string.hpp>

#include <algorithm>
#include <sstream>
#include <vector>
#include <utility>

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

mapnik::font_set make_fontset(std::string const& name, std::initializer_list<std::string> face_names)
{
    mapnik::font_set fontset(name);
    for (auto const& face_name : face_names)
    {
        fontset.add_face_name(face_name);
    }
    return fontset;
}

struct face_span
{
    unsigned start;
    unsigned end;
    std::string face_name;
};

void test_shaping_face_spans(mapnik::font_set const& fontset,
                             mapnik::face_manager& fm,
                             char const* str,
                             std::vector<face_span> const& expected,
                             std::vector<unsigned> const& expected_cluster_indices = {},
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
        unsigned cluster_index;
        std::string face_name;
    };

    std::vector<cluster_face> cluster_faces;
    std::vector<unsigned> actual_cluster_indices;
    for (auto const& g : line)
    {
        if (require_shaped_glyph)
            REQUIRE(g.glyph_index != 0);
        REQUIRE(g.face);
        actual_cluster_indices.push_back(g.char_index);

        std::string face_name = g.face->family_name();
        if (!cluster_faces.empty() && cluster_faces.back().cluster_index == g.char_index)
        {
            REQUIRE(cluster_faces.back().face_name == face_name);
        }
        else
        {
            cluster_faces.push_back({g.char_index, std::move(face_name)});
        }
    }

    REQUIRE(!cluster_faces.empty());
    std::sort(cluster_faces.begin(), cluster_faces.end(), [](cluster_face const& lhs, cluster_face const& rhs) {
        return lhs.cluster_index < rhs.cluster_index;
    });
    std::vector<std::string> face_by_char(length);
    for (std::size_t i = 0; i < cluster_faces.size(); ++i)
    {
        unsigned start = cluster_faces[i].cluster_index;
        unsigned end = (i + 1 < cluster_faces.size()) ? cluster_faces[i + 1].cluster_index : length;

        for (unsigned char_index = start; char_index < end; ++char_index)
        {
            if (face_by_char[char_index].empty())
                face_by_char[char_index] = cluster_faces[i].face_name;
            else
                REQUIRE(face_by_char[char_index] == cluster_faces[i].face_name);
        }
    }

    if (!expected_cluster_indices.empty() && actual_cluster_indices != expected_cluster_indices)
    {
        std::ostringstream s;
        for (std::size_t i = 0; i < actual_cluster_indices.size(); ++i)
        {
            if (i > 0)
                s << ", ";
            s << actual_cluster_indices[i];
        }
        INFO("actual cluster indices: [" << s.str() << "]");
    }
    if (!expected_cluster_indices.empty())
        REQUIRE(actual_cluster_indices == expected_cluster_indices);

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
        for (auto itr = coverage.covering_begin(false); itr != coverage.covering_end(false); ++itr)
            ltr_indices.push_back(*itr);
        REQUIRE(ltr_indices == std::vector<std::size_t>{10, 11, 12});

        std::vector<std::size_t> rtl_indices;
        for (auto itr = coverage.covering_begin(true); itr != coverage.covering_end(true); ++itr)
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
        for (auto itr = coverage.covering_begin(true); itr != coverage.covering_end(true); ++itr)
            rtl_indices.push_back(*itr);
        REQUIRE(rtl_indices == std::vector<std::size_t>{22, 21, 20});
    }

    SECTION("forward and reverse end iterators stay distinct")
    {
        mapnik::detail::font_coverage coverage(0, 2);
        REQUIRE(coverage.covering_end(false) != coverage.covering_end(true));
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
        std::vector<std::pair<unsigned, unsigned>> expected = {{977, 0},
                                                               {1094, 3},
                                                               {1038, 4},
                                                               {1168, 4},
                                                               {9, 7},
                                                               {3, 8},
                                                               {1414, 9},
                                                               {1458, 10},
                                                               {1459, 11},
                                                               {1460, 12},
                                                               {1415, 13}};
        test_shaping(fontset, fm, expected, from_u8string(u8"སྤུ་ཧྲེང (abc)").c_str());
    }

    {
        std::vector<std::pair<unsigned, unsigned>> expected =
          {{977, 0}, {1094, 3}, {1038, 4}, {1168, 4}, {9, 7}, {3, 8}, {1414, 9}, {0, 10}, {0, 11}, {0, 12}, {1415, 13}};
        test_shaping(fontset, fm, expected, from_u8string(u8"སྤུ་ཧྲེང (普兰镇)").c_str());
    }

    {
        std::vector<std::pair<unsigned, unsigned>> expected =
          {{1458, 0}, {1459, 1}, {1460, 2}, {3, 3}, {1414, 4}, {0, 5}, {0, 6}, {0, 7}, {1415, 8}};
        test_shaping(fontset, fm, expected, from_u8string(u8"abc (普兰镇)").c_str());
    }

    {
        std::vector<std::pair<unsigned, unsigned>> expected =
          {{1458, 0}, {1459, 1}, {1460, 2}, {3, 3}, {1414, 4}, {1458, 5}, {1459, 6}, {1460, 7}, {1415, 8}};
        test_shaping(fontset, fm, expected, "abc (abc)");
    }

    {
        //    "ⵃⴰⵢ ⵚⵉⵏⴰⵄⵉ الحي الصناعي"
        std::vector<std::pair<unsigned, unsigned>> expected = {
          {0, 0},   {0, 1},  {0, 2},    {3, 3},    {0, 4},    {0, 5},   {0, 6},    {0, 7},   {0, 8},
          {0, 9},   {3, 10}, {324, 22}, {100, 22}, {47, 21},  {9, 20},  {287, 19}, {16, 19}, {38, 18},
          {70, 17}, {8, 16}, {3, 15},   {324, 14}, {100, 14}, {24, 13}, {70, 12},  {8, 11}};
        test_shaping(fontset, fm, expected, from_u8string(u8"ⵃⴰⵢ ⵚⵉⵏⴰⵄⵉ الحي الصناعي").c_str());
    }
}

TEST_CASE("ligature shaping")
{
    REQUIRE(mapnik::freetype_engine::register_font("test/data/fonts/glukfonts/Foglihten-068.otf"));

    mapnik::font_set fontset("ligature-fontset");
    fontset.add_face_name("Foglihten Regular");

    mapnik::font_library fl;
    mapnik::freetype_engine::font_file_mapping_type font_file_mapping;
    mapnik::freetype_engine::font_memory_cache_type font_memory_cache;
    mapnik::face_manager fm(fl, font_file_mapping, font_memory_cache);

    SECTION("default ligatures shape office with ffi")
    {
        std::vector<std::pair<unsigned, unsigned>> expected = {{80, 0}, {861, 1}, {68, 4}, {70, 5}};
        test_shaping(fontset, fm, expected, "office");
    }

    SECTION("liga off keeps office split")
    {
        mapnik::font_feature_settings ff_settings;
        ff_settings.append(mapnik::font_feature_liga_off);
        std::vector<std::pair<unsigned, unsigned>> expected = {{80, 0}, {568, 1}, {71, 2}, {74, 3}, {68, 4}, {70, 5}};
        test_shaping(fontset, fm, expected, "office", ff_settings);
    }
}

TEST_CASE("font fallback shaping")
{
    REQUIRE(mapnik::freetype_engine::register_fonts("test/data/fonts", true));

    REQUIRE(mapnik::freetype_engine::register_font("fonts/noto/NotoSansSinhala-Subset.ttf"));
    REQUIRE(mapnik::freetype_engine::register_font("fonts/fallback/KhmerFallbackProbe.ttf"));
    REQUIRE(mapnik::freetype_engine::register_font("fonts/fallback/KhmerFallbackFull.ttf"));
    REQUIRE(mapnik::freetype_engine::register_font("fonts/fallback/MyanmarFallbackProbe.ttf"));
    REQUIRE(mapnik::freetype_engine::register_font("fonts/fallback/MyanmarFallbackFull.ttf"));
    REQUIRE(mapnik::freetype_engine::register_font("fonts/fallback/BenestroffSansTest-Regular.ttf"));
    REQUIRE(mapnik::freetype_engine::register_font("fonts/fallback/BenestroffSymbolsTest-symbols.ttf"));
    REQUIRE(mapnik::freetype_engine::register_font("fonts/fallback/BenestroffSymbolsTest-minus.ttf"));

    mapnik::font_library fl;
    mapnik::freetype_engine::font_file_mapping_type font_file_mapping;
    mapnik::freetype_engine::font_memory_cache_type font_memory_cache;
    mapnik::face_manager fm(fl, font_file_mapping, font_memory_cache);

    SECTION("tibetan falls back from font awesome")
    {
        auto fontset = make_fontset("tibetan-fallback", {"FontAwesome Regular", "Noto Sans Tibetan Regular"});
        test_shaping_face_spans(fontset,
                                fm,
                                from_u8string(u8"སྤུ་ཧྲེང").c_str(),
                                {{0, 8, "Noto Sans Tibetan"}},
                                {0, 3, 4, 4, 7});
    }

    SECTION("sinhala cluster falls back past arabic font")
    {
        auto fontset =
          make_fontset("sinhala-fallback", {"Noto Sans Regular", "XB Zar Regular", "Noto Sans Sinhala Subset Regular"});
        test_shaping_face_spans(fontset,
                                fm,
                                from_u8string(u8"Hello ශ්‍රී ලංකා").c_str(),
                                {{0, 6, "Noto Sans"},
                                 {6, 11, "Noto Sans Sinhala Subset"},
                                 {11, 12, "Noto Sans"},
                                 {12, 16, "Noto Sans Sinhala Subset"}});
    }

    SECTION("arabic rtl text falls back and keeps rtl cluster order")
    {
        auto fontset = make_fontset("arabic-rtl-fallback", {"Noto Sans Regular", "Noto Naskh Arabic Regular"});
        test_shaping_face_spans(fontset,
                                fm,
                                from_u8string(u8"مرحبا").c_str(),
                                {{0, 5, "Noto Naskh Arabic"}},
                                {4, 3, 3, 2, 1, 0});
    }

    SECTION("khmer stacked cluster falls back past partial coverage")
    {
        // Derived from Khmer stacked-cluster cases used in HarfBuzz shaping tests.
        auto fontset =
          make_fontset("khmer-stacked-fallback", {"Khmer Fallback Probe Regular", "Khmer Fallback Full Regular"});
        test_shaping_face_spans(fontset, fm, from_u8string(u8"ប្គា").c_str(), {{0, 4, "Khmer Fallback Full"}});
    }

    SECTION("khmer stacked fallback interleaves accepted ranges")
    {
        auto fontset =
          make_fontset("khmer-interleaved-fallback", {"Khmer Fallback Probe Regular", "Khmer Fallback Full Regular"});
        auto text = from_u8string(u8"បប្គប");
        test_shaping_face_spans(
          fontset,
          fm,
          text.c_str(),
          {{0, 1, "Khmer Fallback Probe"}, {1, 3, "Khmer Fallback Full"}, {3, 5, "Khmer Fallback Probe"}});
    }

    SECTION("khmer mark cluster falls back past partial coverage")
    {
        // Derived from Khmer mark-heavy cluster cases used in HarfBuzz shaping tests.
        auto fontset =
          make_fontset("khmer-zwj-fallback", {"Khmer Fallback Probe Regular", "Khmer Fallback Full Regular"});
        test_shaping_face_spans(fontset, fm, from_u8string(u8"ភ៊ឹ").c_str(), {{0, 3, "Khmer Fallback Full"}});
    }

    SECTION("myanmar kinzi cluster falls back past partial coverage")
    {
        // Derived from Myanmar kinzi-style clusters used in HarfBuzz shaping tests.
        auto fontset =
          make_fontset("myanmar-stacked-fallback", {"Myanmar Fallback Probe Regular", "Myanmar Fallback Full Regular"});
        test_shaping_face_spans(fontset, fm, from_u8string(u8"င်္ခ").c_str(), {{0, 4, "Myanmar Fallback Full"}});
    }

    SECTION("last face keeps uncovered tofu cluster")
    {
        auto fontset = make_fontset("last-face-tofu", {"FontAwesome Regular"});
        auto text = from_u8string(u8"普兰镇");
        mapnik::transcoder tr("utf8");
        auto const text_length = static_cast<unsigned>(tr.transcode(text.c_str()).length());
        test_shaping_face_spans(fontset, fm, text.c_str(), {{0, text_length, "FontAwesome"}}, {}, false);
    }

    SECTION("bus stop label keeps ligature cluster on regular face")
    {
        // Derived from the OSM Carto bus-stop label "Bénestroff − Centre".
        auto fontset = make_fontset("benestroff-bus-stop-fallback",
                                    {"Benestroff Sans Test Regular",
                                     "Benestroff Symbols Test Symbols Regular",
                                     "Benestroff Symbols Test Minus Regular"});
        auto text = from_u8string(u8"Bénestroff − Centre");
        test_shaping_face_spans(fontset,
                                fm,
                                text.c_str(),
                                {{0, 11, "Benestroff Sans Test"},
                                 {11, 12, "Benestroff Symbols Test Minus"},
                                 {12, 19, "Benestroff Sans Test"}});
    }
}
