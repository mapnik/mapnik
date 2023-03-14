#include <mapnik/mapnik.hpp>
#include <mutex> // call once
#include "create_image_reader.hpp"
namespace mapnik {

static void setup_once();

void setup()
{
    static std::once_flag init_flag;
    std::call_once(init_flag, setup_once);
}
#ifdef MAPNIK_STATIC_PLUGINS
extern void init_datasource_cache_static();
#endif
static void register_image_readers();

void setup_once()
{
#ifdef MAPNIK_STATIC_PLUGINS
    init_datasource_cache_static(); // defined in datasource_cache_static.cpp
#endif
    register_image_readers();
}

void register_image_readers()
{
#ifdef HAVE_JPEG
    register_jpeg_reader();
#endif
#ifdef HAVE_PNG
    register_png_reader();
#endif
#ifdef HAVE_TIFF
    register_tiff_reader();
#endif
#ifdef HAVE_WEBP
    register_webp_reader();
#endif
}
} // namespace mapnik

// only on other systems then windows and in shared builds
#if !defined(MAPNIK_DISABLE_AUTOSETUP) && MAPNIK_STATIC_PLUGINS == 0 && !defined(_WIN32)
namespace {
class AutoSetup final
{
  public:
    AutoSetup() { mapnik::setup(); };
};
AutoSetup auto_setup{};
} // namespace
#endif
