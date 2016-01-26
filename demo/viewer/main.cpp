/* This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


// qt
#include <QApplication>
#include <QStringList>
#include <QSettings>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include "mainwindow.hpp"

int main( int argc, char **argv )
{
    using mapnik::datasource_cache;
    using mapnik::freetype_engine;

    try
    {
        QCoreApplication::setOrganizationName("Mapnik");
        QCoreApplication::setOrganizationDomain("mapnik.org");
        QCoreApplication::setApplicationName("Viewer");
        QSettings settings("viewer.ini",QSettings::IniFormat);

        // register input plug-ins
        QString plugins_dir = settings.value("mapnik/plugins_dir",
                                             QVariant("/usr/local/lib/mapnik/input/")).toString();
        datasource_cache::instance().register_datasources(plugins_dir.toStdString());
        // register fonts
        int count = settings.beginReadArray("mapnik/fonts");
        for (int index=0; index < count; ++index)
        {
            settings.setArrayIndex(index);
            QString font_dir = settings.value("dir").toString();
            freetype_engine::register_fonts(font_dir.toStdString());
        }
        settings.endArray();

        QApplication app( argc, argv );
        MainWindow window;
        window.show();
        if (argc > 1) window.open(argv[1]);
        if (argc >= 3)
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
        else
        {
            std::shared_ptr<mapnik::Map> map = window.get_map();
            if (map) map->zoom_all();
        }
        if (argc == 4)
        {
            bool ok;
            double scaling_factor = QString(argv[3]).toDouble(&ok);
            if (ok) window.set_scaling_factor(scaling_factor);
        }
        return app.exec();
    }
    catch (std::exception const& ex)
    {
        std::cerr << "Could not start viewer: '" << ex.what() << "'\n";
        return 1;
    }
}
