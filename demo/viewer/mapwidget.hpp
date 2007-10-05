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

#ifndef MAP_WIDGET_HPP
#define MAP_WIDGET_HPP

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QPen>
#include <QItemSelection>
#include <iostream>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <mapnik/map.hpp>

class MapWidget : public QWidget 
{
      Q_OBJECT
      
   public:   
      enum eTool 
      {
         ZoomToBox = 1,
         Pan,
         Info,
      };
      
   private: 
      boost::shared_ptr<mapnik::Map> map_;
      int selected_;
      QPixmap pix_;
      mapnik::Envelope<double> extent_;
      eTool cur_tool_;
      int start_x_;
      int start_y_;
      int end_x_;
      int end_y_;
      bool drag_;
      bool first_;
      QPen pen_;
      int selectedLayer_;
   public:
      MapWidget(QWidget *parent=0);
      void setTool(eTool tool);
      boost::shared_ptr<mapnik::Map> getMap();
      inline QPixmap const& pixmap() const { return pix_;}
      void setMap(boost::shared_ptr<mapnik::Map> map);    
      void defaultView();
      void zoomToBox(mapnik::Envelope<double> const& box);
      void zoomIn();
      void zoomOut();
      void panLeft();
      void panRight();
      void panUp();
      void panDown();
 public slots:
      void zoomToLevel(int level);
      void updateMap();
      void layerSelected(int);
   signals:
      void mapViewChanged();
   protected:    
      void paintEvent(QPaintEvent* ev);
      void resizeEvent(QResizeEvent* ev);
      void mousePressEvent(QMouseEvent* e);
      void mouseMoveEvent(QMouseEvent* e);
      void mouseReleaseEvent(QMouseEvent* e);
      void keyPressEvent(QKeyEvent *e);
      void export_to_file(unsigned width,
                          unsigned height,
                          std::string const& filename, 
                          std::string const& type);    
};

#endif // MAP_WIDGET_HPP
