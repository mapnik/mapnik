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


#ifndef LAYERWIDGET_HPP
#define LAYERWIDGET_HPP

#include <QTabWidget>
#include <QListView>
#include <QTreeView>

class LayerTab : public QListView
{
   Q_OBJECT
   public:
      LayerTab(QWidget* parent=0);
      void paintEvent(QPaintEvent *e);
   signals:
      void update_mapwidget();
      void layerSelected(int) const;
   public slots:
      void layerInfo();
      void layerInfo2(QModelIndex const&);
   protected slots:
      void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
      void selectionChanged(const QItemSelection & selected, const QItemSelection &);
};

class StyleTab : public QTreeView
{
    Q_OBJECT
public:
    StyleTab(QWidget* parent=0);
protected:
      void contextMenuEvent(QContextMenuEvent * event );
};

#endif
