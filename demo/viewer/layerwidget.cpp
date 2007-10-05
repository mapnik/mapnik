/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2007 Artem Pavlenko
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

#include "layerwidget.hpp"
#include <qabstractitemdelegate.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qvector.h>
#include <qstyle.h>
#include <qevent.h>
#include <qscrollbar.h>
#include <qrubberband.h>
#include <qdebug.h>
#include <iostream>
#include "layerlistmodel.hpp"
#include "layer_info_dialog.hpp"

using namespace std;

LayerTab::LayerTab(QWidget* parent)
    : QListView(parent) {}


void LayerTab::paintEvent(QPaintEvent *e)
{
    QListView::paintEvent(e); 
}

void LayerTab::dataChanged(const QModelIndex &topLeft,
                           const QModelIndex &bottomRight)
{
   QListView::dataChanged(topLeft, bottomRight);
   qDebug("FIXME : update map view!");
   emit update_mapwidget();
}

void LayerTab::selectionChanged(const QItemSelection & selected, const QItemSelection &)
{
   QModelIndexList list = selected.indexes();
   if (list.size() != 0) 
   {
      std::cout << "SELECTED LAYER ->" << list[0].row() << "\n";
      emit layerSelected(list[0].row());
   }
}

void LayerTab::layerInfo()
{
   qDebug("Layer info");
   QModelIndexList indexes = selectedIndexes();
   if (indexes.size() > 0)
   {
      qDebug("id = %d",indexes[0].row());
      
   }
}

void LayerTab::layerInfo2(QModelIndex const& index)
{
   qDebug("LayerInfo id = %d",index.row());
   QVector<QPair<QString,QString> > params;
   unsigned i = index.row();
   LayerListModel * model = static_cast<LayerListModel*>(this->model());
   boost::optional<mapnik::Layer&> layer = model->map_layer(i);
   
   if (layer)
   {
      mapnik::datasource_ptr ds = (*layer).datasource();
      if (ds)
      {
         mapnik::parameters ps = ds->params();

         //mapnik::parameters::extract_iterator_type itr = ps.extract_begin();
         //mapnik::parameters::extract_iterator_type end = ps.extract_end();
         
         //for (;itr != end;++itr)
         //{
            //if (itr->second)
         //   {
         ///     params.push_back(QPair<QString,QString>(itr->first.c_str(),itr->first.c_str()));
         //   }
         //}

         
         mapnik::parameters::const_iterator pos;
         
         for (pos = ps.begin();pos != ps.end();++pos)
         {
            boost::optional<std::string> result;
            boost::apply_visitor(mapnik::value_extractor_visitor<std::string>(result),pos->second);
            if (result)
            {
               params.push_back(QPair<QString,QString>(pos->first.c_str(),(*result).c_str()));
            }
         }   
      }
      layer_info_dialog dlg(params,this);
      dlg.getUI().layerNameEdit->setText(QString((*layer).name().c_str()));

      
      dlg.exec();
   }
}

StyleTab::StyleTab(QWidget*)
{
    
}

void StyleTab::contextMenuEvent(QContextMenuEvent * event )
{
   qDebug("test");
}
