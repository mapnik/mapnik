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


#include <QApplication>
#include <QStringList>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include "mainwindow.hpp"

int main( int argc, char **argv )
{
   using mapnik::datasource_cache;
   using mapnik::freetype_engine;
  
   datasource_cache::instance()->register_datasources("/usr/local/lib/mapnik/input");
   
   freetype_engine::instance()->register_font("/usr/local/lib/mapnik/fonts/DejaVuSans.ttf");
   freetype_engine::instance()->register_font("/usr/local/lib/mapnik/fonts/DejaVuSans-Bold.ttf");
   freetype_engine::instance()->register_font("/usr/local/lib/mapnik/fonts/DejaVuSansMono.ttf");
    
        
   QApplication app( argc, argv ); 
   MainWindow window;
   window.show();
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
         if (ok)
         {
            try 
            {
               mapnik::projection prj("+proj=merc +datum=WGS84");
               prj.forward(x0,y0);
               prj.forward(x1,y1);
               window.set_default_extent(x0,y0,x1,y1);
            }
            catch (...) {}
         }
      }
   }
   
   if (argc > 1) window.open(argv[1]);
   return app.exec(); 
}
