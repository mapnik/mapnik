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


int main(int argc,char *argv[])
{
	if(argc < 6)
	{
		std::cerr<<"Usage: render XMLfile w s e n [OSMfile]" << std::endl;
		exit(0);
	}

	datasource_cache::instance()->register_datasources
		("/usr/local/lib/mapnik/input");
	freetype_engine::register_font
		("/usr/local/lib/mapnik/fonts/DejaVuSans.ttf");

	Map m (800,800);
	load_map(m,argv[1]);
	
	if(argc>6)
	{
		parameters p;
		p["type"] = "osm";
		p["file"] = argv[6];
		for(int count=0; count<m.layerCount(); count++)
		{
			parameters q = m.getLayer(count).datasource()->params();
			m.getLayer(count).set_datasource(datasource_cache::instance()->
				create(p));
		}
	}

	Envelope<double> bbox (atof(argv[2]),atof(argv[3]),
							atof(argv[4]),atof(argv[5]));
										
	m.zoomToBox(bbox);

	Image32 buf (m.getWidth(), m.getHeight());
	agg_renderer<Image32> r(m,buf);
	r.apply();

	save_to_file<ImageData32>(buf.data(),"blah.png","png");

	return 0;
}
