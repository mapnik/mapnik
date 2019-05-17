#include <mapnik/map.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/image_util.hpp>

int main()
{
  mapnik::Map m(256, 256);
  mapnik::load_map(m, "rockland.osm");
  m.zoom_all();
  mapnik::image_32 im(m.width(), m.height());
  mapnik::agg_renderer<mapnik::image_32> ren(m, im);
  ren.apply();
  mapnik::save_to_file(im, "rockland.png");
  return 0;
}

