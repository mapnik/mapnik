/* This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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


#include "layer_info_dialog.hpp"

// mapnik
#include <mapnik/layer.hpp>


layer_info_dialog::layer_info_dialog(mapnik::layer& lay, QWidget *parent)
  : QDialog(parent)
{
    ui.setupUi(this);

    ui.tableWidget->setHorizontalHeaderItem(0,new QTableWidgetItem("Name"));
    ui.tableWidget->setHorizontalHeaderItem(1,new QTableWidgetItem("Value"));

    // Layer name
    ui.layerNameEdit->setText(QString(lay.name().c_str()));

    // Named Styles : TODO!!!

    // Datasource
    mapnik::datasource_ptr ds = lay.datasource();
    if (ds)
    {
        mapnik::parameters ps = ds->params();

        ui.tableWidget->setRowCount(ps.size());
        ui.tableWidget->setColumnCount(2);

        mapnik::parameters::const_iterator pos;
        int index=0;
        for (pos = ps.begin();pos != ps.end();++pos)
        {
            boost::optional<std::string> result;
            boost::apply_visitor(mapnik::value_extractor_visitor<std::string>(result),pos->second);
            if (result)
            {
                QTableWidgetItem *keyItem = new QTableWidgetItem(QString(pos->first.c_str()));
                QTableWidgetItem *valueItem = new QTableWidgetItem(QString((*result).c_str()));
                ui.tableWidget->setItem(index,0,keyItem);
                ui.tableWidget->setItem(index,1,valueItem);
                ++index;
            }
        }
    }
}

Ui::LayerInfoDialog& layer_info_dialog::getUI()
{
   return ui;
}

