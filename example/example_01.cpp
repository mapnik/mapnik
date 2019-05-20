#include <mapnik/map.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/datasource_cache.hpp>

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cout << "usage: ./example_01 <OSM plugin location>\n";
    return EXIT_SUCCESS;
  }
  std::string dir(argv[1]);
  std::cout << " running example with OSM plugin in ..." << dir << "\n";
  mapnik::datasource_cache::instance().register_datasources(dir);
  mapnik::Map m(256, 256);
  mapnik::load_map(m, "style_01.xml");
  m.zoom_all();
  mapnik::image_32 im(m.width(), m.height());
  mapnik::agg_renderer<mapnik::image_32> ren(m, im);
  ren.apply();
  mapnik::save_to_file(im, "rockland.png");
  return 0;
}

