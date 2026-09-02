
#include "catch.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <cstring>
#include <random>
#include <sstream>
#include <string>
#include "agg_color_rgba.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rendering_buffer.h"
#include "agg_renderer_base.h"

using color = agg::rgba8;
using order = agg::order_rgba;

std::string to_string(color const& c)
{
    std::ostringstream s;
    s << "rgba(" << (unsigned)c.r << "," << (unsigned)c.g << "," << (unsigned)c.b << "," << (unsigned)c.a << ")";
    return s.str();
}

template<typename blender>
color blend(color const& source, color const& dest, unsigned cover = 255)
{
    unsigned stride = 4;
    unsigned size = 1;

    color source_pre = source;
    source_pre.premultiply();
    color dest_pre = dest;
    dest_pre.premultiply();

    unsigned char* buffer = new unsigned char[size * size * stride];
    std::memset(buffer, 0, size * size * stride);
    buffer[0] = dest_pre.r;
    buffer[1] = dest_pre.g;
    buffer[2] = dest_pre.b;
    buffer[3] = dest_pre.a;
    // http://www.antigrain.com/doc/basic_renderers/basic_renderers.agdoc.html
    agg::rendering_buffer rbuf(buffer, size, size, size * stride);
    color::value_type* psource = (color::value_type*)rbuf.row_ptr(0, 0, 1);
    blender::blend_pix(psource, source_pre.r, source_pre.g, source_pre.b, source_pre.a, cover);
    color color_result(psource[0], psource[1], psource[2], psource[3]);
    color_result.demultiply();
    delete[] buffer;
    return color_result;
}

// agg::pixfmt_alpha_blend_rgba
color normal_blend(color const& source, color const& dest, unsigned cover = 255)
{
    using renderer_type = agg::renderer_base<agg::pixfmt_rgba32_pre>;
    unsigned stride = 4;
    unsigned size = 1;
    color source_pre = source;
    source_pre.premultiply();
    color dest_pre = dest;
    dest_pre.premultiply();
    // source buffer
    unsigned char* source_buffer = new unsigned char[size * size * stride];
    std::memset(source_buffer, 0, size * size * stride);
    source_buffer[0] = source_pre.r;
    source_buffer[1] = source_pre.g;
    source_buffer[2] = source_pre.b;
    source_buffer[3] = source_pre.a;
    agg::rendering_buffer source_rbuffer(source_buffer, size, size, size * 4);
    agg::pixfmt_rgba32_pre pixf_source(source_rbuffer);

    // destination buffer
    unsigned char* dest_buffer = new unsigned char[size * size * stride];
    std::memset(dest_buffer, 0, size * size * stride);
    dest_buffer[0] = dest_pre.r;
    dest_buffer[1] = dest_pre.g;
    dest_buffer[2] = dest_pre.b;
    dest_buffer[3] = dest_pre.a;
    agg::rendering_buffer dest_rbuffer(dest_buffer, size, size, size * 4);
    agg::pixfmt_rgba32_pre pixf_dest(dest_rbuffer);

    // renderer: blends source into destination
    renderer_type ren(pixf_dest);
    ren.blend_from(pixf_source, 0, 0, 0, cover);
    color color_result(dest_buffer[0], dest_buffer[1], dest_buffer[2], dest_buffer[3]);
    color_result.demultiply();
    delete[] source_buffer;
    delete[] dest_buffer;
    return color_result;
}

namespace agg {

// the original agg template code for src_over
// before we changed A as per https://github.com/mapnik/mapnik/issues/1452
template<class ColorT, class Order>
struct comp_op_rgba_src_over2
{
    using color_type = ColorT;
    using order_type = Order;
    using value_type = typename color_type::value_type;
    using calc_type = typename color_type::calc_type;
    enum base_scale_e { base_shift = color_type::base_shift, base_mask = color_type::base_mask };

    //   Dca' = Sca + Dca.(1 - Sa)
    //   Da'  = Sa + Da - Sa.Da
    static void blend_pix(value_type* p, unsigned sr, unsigned sg, unsigned sb, unsigned sa, unsigned cover)
    {
        if (cover < 255)
        {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }
        calc_type s1a = base_mask - sa;
        p[Order::R] = (value_type)(sr + ((p[Order::R] * s1a + base_mask) >> base_shift));
        p[Order::G] = (value_type)(sg + ((p[Order::G] * s1a + base_mask) >> base_shift));
        p[Order::B] = (value_type)(sb + ((p[Order::B] * s1a + base_mask) >> base_shift));
        p[Order::A] = (value_type)(sa + p[Order::A] - ((sa * p[Order::A] + base_mask) >> base_shift));
    }
};

} // namespace agg

TEST_CASE("blending")
{
    SECTION("src over")
    {
        using source_over_old_agg = agg::comp_op_rgba_src_over2<color, agg::order_rgba>;
        using source_over = agg::comp_op_rgba_src_over<color, agg::order_rgba>;

        try
        {
            color white(255, 255, 255, 255);
            color black(0, 0, 0, 255);

            REQUIRE(to_string(blend<source_over>(white, white)) == to_string(white));
            REQUIRE(to_string(blend<source_over>(white, black)) == to_string(white));
            REQUIRE(to_string(blend<source_over>(black, white)) == to_string(black));

            color near_white(254, 254, 254, 254);     // Source
            color near_trans(1, 1, 1, 1);             // Dest
            color expected_color(253, 253, 253, 255); // expected result
            REQUIRE(to_string(blend<source_over_old_agg>(near_white, near_trans)) ==
                    to_string(color(253, 253, 253, 254)));
            REQUIRE(to_string(blend<source_over>(near_white, near_trans)) == to_string(expected_color));
            REQUIRE(to_string(normal_blend(near_white, near_trans)) == to_string(expected_color));

            // using normal_blend as expected, compare a variety of other colors

            {
                color source(128, 128, 128, 255);
                color dest(128, 128, 128, 255);
                unsigned cover = 128;
                std::string expected_str = to_string(normal_blend(source, dest, cover));
                REQUIRE(to_string(blend<source_over>(source, dest, cover)) == expected_str);
                REQUIRE(to_string(blend<source_over_old_agg>(source, dest, cover)) == expected_str);
            }

            {
                color source(128, 128, 128, 255);
                color dest(128, 128, 128, 255);
                unsigned cover = 245;
                std::string expected_str = to_string(normal_blend(source, dest, cover));
                REQUIRE(to_string(blend<source_over>(source, dest, cover)) == expected_str);
                REQUIRE(to_string(blend<source_over_old_agg>(source, dest, cover)) == expected_str);
            }

            // commenting until I study these failures more (dane)
            /*
              {
              // fails, why?
              color source(127,127,127,127);
              color   dest(127,127,127,127);
              unsigned cover = 255;
              std::string expected_str = to_string(normal_blend(source,dest,cover));
              REQUIRE( to_string(blend<source_over>(source,dest,cover)) == expected_str );
              REQUIRE( to_string(blend<source_over_old_agg>(source,dest,cover)) == expected_str );
              }

              {
              // fails, why?
              color source(128,128,128,128);
              color   dest(128,128,128,128);
              unsigned cover = 128;
              std::string expected_str = to_string(normal_blend(source,dest,cover));
              REQUIRE( to_string(blend<source_over>(source,dest,cover)) == expected_str );
              REQUIRE( to_string(blend<source_over_old_agg>(source,dest,cover)) == expected_str );
              }
            */
        }
        catch (std::exception const& ex)
        {
            std::clog << ex.what() << "\n";
            REQUIRE(false);
        }
    }
}

namespace {

constexpr std::size_t span_length = 13;
constexpr std::size_t span_bytes = span_length * 4;
using span_buffer = std::array<unsigned char, span_bytes>;
using source_over_blender = agg::comp_op_adaptor_rgba_pre<color, order>;
using source_over_pixfmt = agg::pixfmt_custom_blend_rgba<source_over_blender, agg::rendering_buffer>;

void scalar_source_over(span_buffer& pixels, color const& source, unsigned cover)
{
    for (std::size_t i = 0; i < span_length; ++i)
    {
        agg::comp_op_rgba_src_over<color, order>::blend_pix(pixels.data() + i * 4,
                                                            source.r,
                                                            source.g,
                                                            source.b,
                                                            source.a,
                                                            cover);
    }
}

struct source_blender
{
    using color_type = color;
    using order_type = order;
    using value_type = color_type::value_type;

    static void blend_pix(unsigned, value_type* p, unsigned r, unsigned g, unsigned b, unsigned a, unsigned cover)
    {
        agg::comp_op_rgba_src<color_type, order_type>::blend_pix(p, r, g, b, a, cover);
    }
};

} // namespace

TEST_CASE("source-over span blending matches scalar AGG")
{
    std::minstd_rand random(0x4d61704e);
    auto random_byte = [&]() {
        return static_cast<unsigned char>(random() & 0xff);
    };

    for (unsigned trial = 0; trial < 128; ++trial)
    {
        span_buffer initial;
        span_buffer covers;
        for (auto& value : initial)
        {
            value = random_byte();
        }
        for (auto& value : covers)
        {
            value = random_byte();
        }
        if (trial == 0)
        {
            covers.fill(255);
        }

        color source(random_byte(), random_byte(), random_byte(), random_byte());
        if (trial == 0)
        {
            source.a = 255;
        }
        unsigned cover = trial == 0 ? 255 : random_byte();

        INFO("trial " << trial);

        {
            span_buffer expected = initial;
            scalar_source_over(expected, source, cover);
            span_buffer actual = initial;
            agg::rendering_buffer rbuf(actual.data(), span_length, 1, span_bytes);
            source_over_pixfmt pixfmt(rbuf, agg::comp_op_src_over);
            pixfmt.blend_hline(0, 0, span_length, source, cover);
            CHECK(actual == expected);
        }

        {
            span_buffer expected = initial;
            for (std::size_t i = 0; i < span_length; ++i)
            {
                agg::comp_op_rgba_src_over<color, order>::blend_pix(expected.data() + i * 4,
                                                                    source.r,
                                                                    source.g,
                                                                    source.b,
                                                                    source.a,
                                                                    covers[i]);
            }
            span_buffer actual = initial;
            agg::rendering_buffer rbuf(actual.data(), span_length, 1, span_bytes);
            source_over_pixfmt pixfmt(rbuf, agg::comp_op_src_over);
            pixfmt.blend_solid_hspan(0, 0, span_length, source, covers.data());
            CHECK(actual == expected);
        }

        {
            std::array<unsigned char, span_bytes * 2> storage;
            for (std::size_t i = 0; i < span_bytes; ++i)
            {
                storage[i] = random_byte();
                storage[span_bytes + i] = initial[i];
            }
            if (trial == 0)
            {
                storage.fill(0);
            }
            if (trial == 1)
            {
                for (std::size_t i = 0; i < span_length; ++i)
                {
                    storage[i * 4 + 3] = 255;
                }
                cover = 255;
            }
            span_buffer expected = initial;
            std::copy(storage.begin() + span_bytes, storage.end(), expected.begin());
            for (std::size_t i = 0; i < span_length; ++i)
            {
                agg::comp_op_rgba_src_over<color, order>::blend_pix(expected.data() + i * 4,
                                                                    storage[i * 4],
                                                                    storage[i * 4 + 1],
                                                                    storage[i * 4 + 2],
                                                                    storage[i * 4 + 3],
                                                                    cover);
            }
            agg::rendering_buffer source_rbuf(storage.data(), span_length, 1, span_bytes);
            agg::pixfmt_rgba32_pre source_pixfmt(source_rbuf);
            agg::rendering_buffer dest_rbuf(storage.data() + span_bytes, span_length, 1, span_bytes);
            source_over_pixfmt dest_pixfmt(dest_rbuf, agg::comp_op_src_over);
            dest_pixfmt.blend_from(source_pixfmt, 0, 0, 0, 0, span_length, cover);
            CHECK(std::equal(expected.begin(), expected.end(), storage.begin() + span_bytes));
        }
    }
}

TEST_CASE("custom AGG blenders are not bypassed by span optimizations")
{
    using source_pixfmt = agg::pixfmt_custom_blend_rgba<source_blender, agg::rendering_buffer>;
    span_buffer initial;
    for (std::size_t i = 0; i < span_length; ++i)
    {
        initial[i * 4] = 255;
        initial[i * 4 + 1] = 0;
        initial[i * 4 + 2] = 0;
        initial[i * 4 + 3] = 255;
    }
    color source(0, 0, 128, 128);
    span_buffer expected = initial;
    for (std::size_t i = 0; i < span_length; ++i)
    {
        agg::comp_op_rgba_src<color, order>::blend_pix(expected.data() + i * 4,
                                                       source.r,
                                                       source.g,
                                                       source.b,
                                                       source.a,
                                                       255);
    }

    span_buffer actual = initial;
    agg::rendering_buffer rbuf(actual.data(), span_length, 1, span_bytes);
    source_pixfmt pixfmt(rbuf, agg::comp_op_src_over);
    pixfmt.blend_hline(0, 0, span_length, source, 255);
    CHECK(actual == expected);
}
