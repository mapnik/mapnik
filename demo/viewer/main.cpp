/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2006 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id$

// qt
#include <QApplication>
#include <QStringList>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include "mainwindow.hpp"
// boost
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>

bool is_font_file (std::string const& filename)
{
    return boost::algorithm::ends_with(filename,std::string(".ttf"));
}

int main( int argc, char **argv )
{
	using mapnik::datasource_cache;
	using mapnik::freetype_engine;
	
	// modify this prefix based on your install location
	std::string mapnik_dir = "/opt/mapnik";
	
	datasource_cache::instance()->register_datasources(mapnik_dir + "/lib/mapnik/input");
	boost::filesystem::path path(mapnik_dir + "/lib/mapnik/fonts");
	boost::filesystem::directory_iterator end_itr;	
    
    
	if (boost::filesystem::exists(path) && boost::filesystem::is_directory(path))
	{
		for (boost::filesystem::directory_iterator itr(path);itr!=end_itr;++itr )
		{
			if (!boost::filesystem::is_directory(*itr) && is_font_file(itr->path().leaf())) 
			{
				std::cout << "register font " << itr->string() << "\n";
				freetype_engine::register_font(itr->string());
			}
		}
    }
		
	QApplication app( argc, argv ); 
	MainWindow window;
	window.show();
	if (argc > 1) window.open(argv[1]);
	if (argc == 3)
	{
		QStringList list = QString(argv[2]).split(",");
		if (list.size()==4)
		{
			bool ok;
			double x0 = list[0].toDouble(&ok);
			double y0 = list[1].toDouble(&ok);
			double x1 = list[2].toDouble(&ok);
			double y1 = list[3].toDouble(&ok);
			if (ok) window.set_default_extent(x0,y0,x1,y1);
		}
	}
	
	return app.exec(); 
}
