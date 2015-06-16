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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#include "info_dialog.hpp"


info_dialog::info_dialog(QVector<QPair<QString,QString> > const& info, QWidget *parent)
  : QDialog(parent)
{
  ui.setupUi(this);
  ui.tableWidget->setHorizontalHeaderItem(0,new QTableWidgetItem("Name"));
  ui.tableWidget->setHorizontalHeaderItem(1,new QTableWidgetItem("Value"));

  ui.tableWidget->setRowCount(info.size());
  ui.tableWidget->setColumnCount(2);
  for (int i=0;i<info.size();++i)
  {
     QTableWidgetItem *keyItem = new QTableWidgetItem(info[i].first);
     QTableWidgetItem *valueItem = new QTableWidgetItem(info[i].second);
     ui.tableWidget->setItem(i,0,keyItem);
     ui.tableWidget->setItem(i,1,valueItem);
  }
}
