#include "catch.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>

#include <mapnik/image.hpp>
#include <mapnik/image_compositing.hpp>

#include "agg_color_rgba.h"
#include "agg_pixfmt_rgba.h"

namespace {

using color = agg::rgba8;
using order = agg::order_rgba;
using rounded = agg::comp_op_rgba_src_over_rounded<color, order>;
using pixel = std::array<std::uint8_t, 4>; // premultiplied R, G, B, A

template<typename Blender>
pixel blend(pixel dst, pixel const& src, unsigned cover)
{
    Blender::blend_pix(dst.data(), src[0], src[1], src[2], src[3], cover);
    return dst;
}

pixel read_pixel(mapnik::image_rgba8 const& im, std::size_t x, std::size_t y)
{
    pixel p;
    mapnik::image_rgba8::pixel_type const v = im(x, y);
    std::memcpy(p.data(), &v, 4);
    return p;
}

void fill_image(mapnik::image_rgba8& im, pixel const& p)
{
    mapnik::image_rgba8::pixel_type v;
    std::memcpy(&v, p.data(), 4);
    for (std::size_t y = 0; y < im.height(); ++y)
        for (std::size_t x = 0; x < im.width(); ++x)
            im(x, y) = v;
}

// the fill colour of the original bug report: dark green, fully opaque
constexpr pixel fill_color = {9, 49, 45, 255};

} // namespace

// mapnik::composite() converts mapnik::composite_mode_e to agg::comp_op_e with a static_cast,
// so both enums have to stay in sync.
static_assert(static_cast<int>(mapnik::divide) == static_cast<int>(agg::comp_op_divide));
static_assert(static_cast<int>(mapnik::src_over_rounded) == static_cast<int>(agg::comp_op_src_over_rounded));

TEST_CASE("src-over-rounded")
{
    SECTION("is registered under its name")
    {
        auto mode = mapnik::comp_op_from_string("src-over-rounded");
        REQUIRE(mode);
        CHECK(*mode == mapnik::src_over_rounded);
        auto name = mapnik::comp_op_to_string(mapnik::src_over_rounded);
        REQUIRE(name);
        CHECK(*name == "src-over-rounded");
        // the existing operation is untouched
        REQUIRE(mapnik::comp_op_from_string("src-over"));
        CHECK(*mapnik::comp_op_from_string("src-over") == mapnik::src_over);
    }

    SECTION("div_mask is exactly round(x / 255)")
    {
        for (unsigned x = 0; x <= 255 * 255; ++x)
        {
            // x / 255 is never an exact half, so there is no tie to resolve
            REQUIRE(rounded::div_mask(x) == (2 * x + 255) / 510);
        }
    }

    SECTION("full coverage with an opaque source gives the source exactly")
    {
        for (unsigned d = 0; d <= 255; ++d)
        {
            pixel dst = {std::uint8_t(d), std::uint8_t(255 - d), std::uint8_t(d / 2), std::uint8_t(d)};
            REQUIRE(blend<rounded>(dst, fill_color, 255) == fill_color);
        }
    }

    SECTION("zero coverage leaves the destination untouched")
    {
        for (unsigned d = 0; d <= 255; ++d)
        {
            pixel dst = {std::uint8_t(d), std::uint8_t(255 - d), std::uint8_t(d / 2), std::uint8_t(d)};
            REQUIRE(blend<rounded>(dst, fill_color, 0) == dst);
        }
    }

    SECTION("a single blend is within one level of the exact result")
    {
        for (unsigned cover = 1; cover < 255; ++cover)
        {
            for (unsigned d : {0u, 1u, 100u, 254u, 255u})
            {
                pixel dst = {std::uint8_t(d), std::uint8_t(d), std::uint8_t(d), 255};
                pixel out = blend<rounded>(dst, fill_color, cover);
                for (int c = 0; c < 3; ++c)
                {
                    double const exact = d + (fill_color[c] - double(d)) * cover / 255.0;
                    CAPTURE(cover, d, c);
                    CHECK(std::abs(out[c] - exact) <= 1.0);
                }
            }
        }
    }

    SECTION("repeated partial blends never pass the source colour")
    {
        // Many polygons covering the same pixel partially is what makes the old rounding of
        // "src-over" drift away from the source colour (towards grey).
        for (unsigned background : {0u, 255u})
        {
            for (unsigned cover = 1; cover < 255; ++cover)
            {
                pixel p = {std::uint8_t(background), std::uint8_t(background), std::uint8_t(background), 255};
                pixel previous = p;
                for (int i = 0; i < 2000; ++i)
                {
                    p = blend<rounded>(p, fill_color, cover);
                    for (int c = 0; c < 3; ++c)
                    {
                        CAPTURE(background, cover, i, c);
                        // never beyond the source colour ...
                        REQUIRE(p[c] >= std::min<unsigned>(background, fill_color[c]));
                        REQUIRE(p[c] <= std::max<unsigned>(background, fill_color[c]));
                        // ... and always moving towards it
                        if (background == 0)
                            REQUIRE(p[c] >= previous[c]);
                        else
                            REQUIRE(p[c] <= previous[c]);
                    }
                    REQUIRE(p[3] == 255);
                    previous = p;
                }
            }
        }
    }

    SECTION("transparent destination keeps a valid premultiplied pixel")
    {
        for (unsigned cover = 1; cover < 255; ++cover)
        {
            pixel p = {0, 0, 0, 0};
            for (int i = 0; i < 2000; ++i)
            {
                p = blend<rounded>(p, fill_color, cover);
                CAPTURE(cover, i);
                REQUIRE(p[0] <= p[3]);
                REQUIRE(p[1] <= p[3]);
                REQUIRE(p[2] <= p[3]);
                REQUIRE(p[0] <= fill_color[0]);
                REQUIRE(p[1] <= fill_color[1]);
                REQUIRE(p[2] <= fill_color[2]);
            }
        }
    }

    SECTION("mapnik::composite() with the new mode")
    {
        mapnik::image_rgba8 dst(2, 2, true, true);
        fill_image(dst, {0, 0, 0, 255});
        mapnik::image_rgba8 src(2, 2, true, true);
        fill_image(src, fill_color);

        // opacity 0.05 turns into a small coverage (12/255) for every blend: the pixel gets closer to the
        // source colour but must never pass it
        for (int i = 0; i < 2000; ++i)
        {
            mapnik::composite(dst, src, mapnik::src_over_rounded, 0.05f);
            pixel p = read_pixel(dst, 1, 1);
            REQUIRE(p[0] <= fill_color[0]);
            REQUIRE(p[1] <= fill_color[1]);
            REQUIRE(p[2] <= fill_color[2]);
            REQUIRE(p[3] == 255);
        }
        // full opacity replaces the destination exactly
        fill_image(dst, {200, 100, 50, 255});
        mapnik::composite(dst, src, mapnik::src_over_rounded, 1.0f);
        CHECK(read_pixel(dst, 0, 0) == fill_color);
    }
}
