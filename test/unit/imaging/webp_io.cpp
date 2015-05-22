
#if defined(HAVE_WEBP)

#include "catch.hpp"

#include <mapnik/image_util.hpp>
#include <mapnik/image_view_any.hpp>
#include <mapnik/webp_io.hpp>

TEST_CASE("webp io") {

SECTION("does not crash accessing view") {
    std::stringstream s;
    mapnik::image_rgba8 im(1024,1024);
    mapnik::image_view_rgba8 view(512,512,1024,1024,im);
    WebPConfig config;
    if (!WebPConfigInit(&config))
    {
        throw std::runtime_error("version mismatch");
    }
    save_as_webp(s,view,config,true);
}

}

#endif
