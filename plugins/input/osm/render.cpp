#include <mapnik/map.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/envelope.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/projection.hpp>

using namespace mapnik;

#include <iostream>
#include <cmath>
using namespace std;


int main()
{
	datasource_cache::instance()->register_datasources
		("/usr/local/lib/mapnik/input");
	freetype_engine::register_font
		("/usr/local/lib/mapnik/fonts/Vera.ttf");

	Map m (800,800);
	load_map(m,"test.xml");

	Envelope<double> bbox (-1.42,50.93,-1.38,50.97);
										
	m.zoomToBox(bbox);

	Image32 buf (m.getWidth(), m.getHeight());
	agg_renderer<Image32> r(m,buf);
	r.apply();

	save_to_file<ImageData32>(buf.data(),"blah.png","png");

	return 0;
}
