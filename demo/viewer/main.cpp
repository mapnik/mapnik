/* This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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
#include "roadmerger.h"
#include "ThreadPool.h"

int main(int argc, char** argv)
{
    using mapnik::datasource_cache;
    using mapnik::freetype_engine;
    try
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
        if(argc<7)
        {
         std::cerr << "Program is missing parameters!"  << "'\n";
         return 1;
        }
        else
        {
         std::cout<<"arcv[0]:"<<argv[0]<<std::endl;
         std::cout<<" arcv[1]:"<<argv[1]<<" argv[2]:"<<argv[2]<<" arcv[3]:"<<argv[3]<<" argv[4]:"<<argv[4]<<" argv[5]:"<<argv[5]<<std::endl;
        }
        QString appPath = argv[5];
        QString iniFilePath = appPath + "/viewer.ini";
        QCoreApplication::setApplicationName("Viewer");
        QSettings settings(iniFilePath, QSettings::IniFormat);

        // register input plug-ins
        QString plugins_dir = settings.value("mapnik/plugins_dir", QVariant("/usr/local/lib/mapnik/input/")).toString();
        datasource_cache::instance().register_datasources(plugins_dir.toStdString());
        // register fonts
        int count = settings.beginReadArray("mapnik/fonts");
        for (int index = 0; index < count; ++index)
        {
            settings.setArrayIndex(index);
            QString font_dir = settings.value("dir").toString();
            freetype_engine::register_fonts(font_dir.toStdString());
        }
        settings.endArray();

        QApplication app(argc, argv);
        MainWindow window;
        QString featureid2osmidPath = argv[3];
        if(!window.loadFeatureid2osmid(featureid2osmidPath))
        {
          std::cerr << "Loading Featureid2osmid json failed!"  << "'\n";
          return 1;
        }
        window.setMidLineJsonPath(argv[4]);
        window.setCompleteRoadsFile(argv[6]);
        window.show();
        std::cout<<"arcv[1]:"<<argv[1]<<"argv[2]:"<<argv[2]<<std::endl;
        window.mapWidget()->roadMerger->merge(argv[1],argv[2]);
        return app.exec();
    } catch (std::exception const& ex)
    {
        std::cerr << "Could not start viewer: '" << ex.what() << "'\n";
        return 1;
    }
}
