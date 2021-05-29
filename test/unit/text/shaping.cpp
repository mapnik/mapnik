#include "catch.hpp"
#include <mapnik/text/icu_shaper.hpp>
#include <mapnik/text/harfbuzz_shaper.hpp>
#include <mapnik/text/font_library.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/util/from_u8string.hpp>

namespace {

using mapnik::util::from_u8string;

void test_shaping( mapnik::font_set const& fontset, mapnik::face_manager& fm,
                   std::vector<std::pair<unsigned, unsigned>> const& expected, char const* str, bool debug = false)
{
    mapnik::transcoder tr("utf8");
    std::map<unsigned,double> width_map;
    mapnik::text_itemizer itemizer;
    auto props = std::make_unique<mapnik::detail::evaluated_format_properties>();
    props->fontset = fontset;
    props->text_size = 32;

    double scale_factor = 1;
    auto ustr = tr.transcode(str);
    auto length = ustr.length();
    itemizer.add_text(ustr, props);

    mapnik::text_line line(0, length);
    mapnik::harfbuzz_shaper::shape_text(line, itemizer,
                                        width_map,
                                        fm,
                                        scale_factor);

    std::size_t index = 0;
    for (auto const& g : line)
    {
        if (debug)
        {
            if (index++ > 0) std::cerr << ",";
            std::cerr << "{" << g.glyph_index << ", " << g.char_index
                //<< ", " << g.face->family_name() << ":" << g.face->style_name()
                      <<  "}";
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
        std::vector<std::pair<unsigned, unsigned>> expected =
            {{0, 0}, {0, 1}, {0, 2}, {3, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7},
              {0, 8}, {0, 9}, {3, 10}, {509, 22}, {481, 21}, {438, 20}, {503, 19},
              {470, 18}, {496, 17}, {43, 16}, {3, 15}, {509, 14}, {454, 13}, {496, 12}, {43, 11}};
        test_shaping(fontset, fm, expected, from_u8string(u8"ⵃⴰⵢ ⵚⵉⵏⴰⵄⵉ الحي الصناعي").c_str());
    }


}
