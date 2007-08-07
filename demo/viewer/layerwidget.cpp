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

StyleTab::StyleTab(QWidget*)
{
    
}

void StyleTab::contextMenuEvent(QContextMenuEvent * event )
{
   qDebug("test");
}
