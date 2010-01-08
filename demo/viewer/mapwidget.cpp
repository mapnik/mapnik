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

#include <QtGui>

#include <boost/bind.hpp>

#include <mapnik/agg_renderer.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/memory_datasource.hpp>
#include "mapwidget.hpp"
#include "info_dialog.hpp"

using mapnik::Image32;
using mapnik::Map;
using mapnik::Layer;
using mapnik::Envelope;
using mapnik::coord2d;
using mapnik::feature_ptr;
using mapnik::geometry_ptr;
using mapnik::CoordTransform;
using mapnik::projection;

double scales [] = {279541132.014,
                    139770566.007,
                    69885283.0036,
                    34942641.5018,
                    17471320.7509,
                    8735660.37545,
                    4367830.18772,
                    2183915.09386,
                    1091957.54693,
                    545978.773466,
                    272989.386733,
                    136494.693366,
                    68247.3466832,
                    34123.6733416,
                    17061.8366708,
                    8530.9183354,
                    4265.4591677,
                    2132.72958385,
                    1066.36479192,
                    533.182395962};

MapWidget::MapWidget(QWidget *parent) 
   : QWidget(parent),
     map_(),
     selected_(1),
     extent_(),
     cur_tool_(ZoomToBox),
     start_x_(0),
     start_y_(0),
     end_x_(0),
     end_y_(0),
     drag_(false),
     first_(true),
     pen_(QColor(0,0,255,96)),
     selectedLayer_(-1)
{
   pen_.setWidth(3);
   pen_.setCapStyle(Qt::RoundCap);
   pen_.setJoinStyle(Qt::RoundJoin);
}

void MapWidget::setTool(eTool tool)
{
   cur_tool_=tool;
}

void MapWidget::paintEvent(QPaintEvent*)
{  
   QPainter painter(this);
   
   if (drag_)
   {
      if (cur_tool_ == ZoomToBox)
      {
         unsigned width = end_x_-start_x_;
         unsigned height = end_y_-start_y_;
         painter.drawPixmap(QPoint(0, 0),pix_);
         painter.setPen(pen_);
         painter.drawRect(start_x_,start_y_,width,height);
      }
      else if (cur_tool_ == Pan)
      {
         int dx = end_x_-start_x_;
         int dy = end_y_-start_y_;
         painter.setBrush(QColor(200,200,255,128));
         painter.drawRect(0,0,width(),height());
         painter.drawPixmap(QPoint(dx,dy),pix_);
      }
   }
   else
   {
      painter.drawPixmap(QPoint(0, 0),pix_);
   }
   painter.end();
}
 
void MapWidget::resizeEvent(QResizeEvent * ev)
{
   if (map_)
   {
      map_->resize(ev->size().width(),ev->size().height());
      updateMap();
   }
}
   
void MapWidget::mousePressEvent(QMouseEvent* e)
{
   if (e->button()==Qt::LeftButton) 
   {
      if (cur_tool_ == ZoomToBox || cur_tool_==Pan)
      {
         start_x_ = e->x();
         start_y_ = e->y();
         drag_=true;
      }
      else if (cur_tool_==Info)
      {
         if (map_)
         {     
            QVector<QPair<QString,QString> > info;
            
            projection map_proj(map_->srs()); // map projection
            double scale_denom = scale_denominator(*map_,map_proj.is_geographic());
            CoordTransform t(map_->getWidth(),map_->getHeight(),map_->getCurrentExtent());
            
            for (unsigned index = 0; index <  map_->layerCount();++index)
            {
               if (int(index) != selectedLayer_) continue;
               
               Layer & layer = map_->layers()[index];
               if (!layer.isVisible(scale_denom)) continue;
               std::string name = layer.name();
               double x = e->x();
               double y = e->y();
               std::cout << "query at " << x << "," << y << "\n";
               projection layer_proj(layer.srs());
               mapnik::proj_transform prj_trans(map_proj,layer_proj);
               //std::auto_ptr<mapnik::memory_datasource> data(new mapnik::memory_datasource);
               mapnik::featureset_ptr fs = map_->query_map_point(index,x,y);
	         	
               if (fs)
               {
                  feature_ptr feat  = fs->next();
                  if (feat)   
                  {
                     std::map<std::string,mapnik::value> const& props = feat->props();
                     std::map<std::string,mapnik::value>::const_iterator itr=props.begin();
                     for (; itr!=props.end();++itr)
                     {
                        if (itr->second.to_string().length() > 0)
                        {
                           info.push_back(QPair<QString,QString>(QString(itr->first.c_str()),
                                                                 itr->second.to_string().c_str()));
                        }
                     }
                     typedef mapnik::coord_transform2<mapnik::CoordTransform,mapnik::geometry2d> path_type;
                     
                     for  (unsigned i=0; i<feat->num_geometries();++i)
                     {
                        mapnik::geometry2d & geom = feat->get_geometry(i);                       
                        path_type path(t,geom,prj_trans);
                        if (geom.num_points() > 0)
                        {
                           QPainterPath qpath;
                           double x,y;
                           path.vertex(&x,&y);
                           qpath.moveTo(x,y);
                           for (unsigned j=1; j < geom.num_points(); ++j)
                           {
                              path.vertex(&x,&y);
                              qpath.lineTo(x,y);
                           }
                           QPainter painter(&pix_);
                           QPen pen(QColor(255,0,0,96));
                           pen.setWidth(3);
                           pen.setCapStyle(Qt::RoundCap);
                           pen.setJoinStyle(Qt::RoundJoin);
                           painter.setPen(pen);
                           painter.drawPath(qpath);
                           update();
                        }
                     }
                  }
               }
                  
               if (info.size() > 0)
               {
                  info_dialog info_dlg(info,this);
                  info_dlg.exec();
                  break;
               }
            }
            
            // remove annotation layer
            map_->layers().erase(remove_if(map_->layers().begin(),
                                           map_->layers().end(),
                                           bind(&Layer::name,_1) == "*annotations*")
                                 , map_->layers().end());
         }
      } 
   }
   else if (e->button()==Qt::RightButton) 
   {	
      //updateMap();
   }
}
    
void MapWidget::mouseMoveEvent(QMouseEvent* e)
{    
   if (cur_tool_ == ZoomToBox || cur_tool_==Pan)
   {
      end_x_ = e->x();
      end_y_ = e->y();
      update();
   }    
}

void MapWidget::mouseReleaseEvent(QMouseEvent* e)
{
   if (e->button()==Qt::LeftButton) 
   {
      end_x_ = e->x();
      end_y_ = e->y();
      if (cur_tool_ == ZoomToBox)
      {
         drag_=false;
         if (map_)
         {
            CoordTransform t(map_->getWidth(),map_->getHeight(),map_->getCurrentExtent());	
            Envelope<double> box = t.backward(Envelope<double>(start_x_,start_y_,end_x_,end_y_));
            map_->zoomToBox(box);
            updateMap();
         }
      }
      else if (cur_tool_==Pan)
      {
         drag_=false;
         if (map_)
         {
            int cx = int(0.5 * map_->getWidth());
            int cy = int(0.5 * map_->getHeight());
            int dx = end_x_ - start_x_;
            int dy = end_y_ - start_y_;
            map_->pan(cx - dx ,cy - dy); 
            updateMap();
         }
      }
   }
}


void MapWidget::keyPressEvent(QKeyEvent *e) 
{
   std::cout << "key pressed:"<<e->key()<<"\n";
   switch (e->key()) { 
      case Qt::Key_Minus:
         zoomOut();
         break;
      case Qt::Key_Plus:
      case 61:
         zoomIn();
         break;
      case 65:
         defaultView();
         break;
      case Qt::Key_Up:
         panUp();
         break;
      case Qt::Key_Down:
         panDown();
         break;  
      case Qt::Key_Left:
         panLeft();
         break; 
      case Qt::Key_Right:
         panRight();
         break;
      case 49:
         zoomToLevel(10);
         break;
      case 50:
         zoomToLevel(11);
         break;
      case 51:
         zoomToLevel(12);
         break;   
      case 52:
         zoomToLevel(13);
         break;
      case 53:
         zoomToLevel(14);
         break;
      case 54:
         zoomToLevel(15);
         break;   
      case 55:
         zoomToLevel(16);
         break;
      case 56:
         zoomToLevel(17);
         break;
      case 57:
         zoomToLevel(18);
         break;   
   }
   QWidget::keyPressEvent(e);
}

void MapWidget::zoomToBox(mapnik::Envelope<double> const& bbox)
{
   if (map_)
   {
      map_->zoomToBox(bbox);
      updateMap();
   }
}

void MapWidget::defaultView()  
{
   if (map_)
   {
      map_->resize(width(),height());
      map_->zoom_all();
      updateMap();
   }
}

void MapWidget::zoomIn() 
{
   if (map_)
   {
      map_->zoom(0.5);
      updateMap();
   }
}

void MapWidget::zoomOut() 
{
   if (map_)
   {
      map_->zoom(2.0);
      updateMap();
   }
}

void MapWidget::panUp()
{
   if (map_)
   {
      double cx = 0.5*map_->getWidth();
      double cy = 0.5*map_->getHeight();
      map_->pan(int(cx),int(cy - cy*0.25));
      updateMap();
   }
}

void MapWidget::panDown()
{
   if (map_)
   {
      double cx = 0.5*map_->getWidth();
      double cy = 0.5*map_->getHeight();
      map_->pan(int(cx),int(cy + cy*0.25));
      updateMap();
   }
}

void MapWidget::panLeft()
{
   if (map_)
   {
      double cx = 0.5*map_->getWidth();
      double cy = 0.5*map_->getHeight();
      map_->pan(int(cx - cx * 0.25),int(cy));
      updateMap();
   }
}

void MapWidget::panRight()
{ 
   if (map_)
   {
      double cx = 0.5*map_->getWidth();
      double cy = 0.5*map_->getHeight();
      map_->pan(int(cx + cx * 0.25),int(cy)); 
      updateMap();
   }
}


void MapWidget::zoomToLevel(int level)
{
   if ( map_ && level >= 0 && level < 19 )
   {
      double scale_denom  = scales[level];
      std::cerr << "scale denominator = " << scale_denom << "\n";
      mapnik::Envelope<double> ext = map_->getCurrentExtent();
      double width = static_cast<double>(map_->getWidth());
      double height= static_cast<double>(map_->getHeight()); 
      mapnik::coord2d pt = ext.center();

      double res = scale_denom * 0.00028;
      
      mapnik::Envelope<double> box(pt.x - 0.5 * width * res,
                                   pt.y - 0.5 * height*res,
                                   pt.x + 0.5 * width * res,
                                   pt.y + 0.5 * height*res);
      map_->zoomToBox(box);
      updateMap();
   }
}

void MapWidget::export_to_file(unsigned ,unsigned ,std::string const&,std::string const&) 
{
   //Image32 image(width,height);
   //agg_renderer renderer(map,image);
   //renderer.apply();
   //image.saveToFile(filename,type);
}
       

void MapWidget::updateMap() 
{   
   if (map_)
   {
      unsigned width=map_->getWidth();
      unsigned height=map_->getHeight();
      
      Image32 buf(width,height);

      try 
      {
	  mapnik::agg_renderer<Image32> ren(*map_,buf);
	  ren.apply();
	  
	  QImage image((uchar*)buf.raw_data(),width,height,QImage::Format_ARGB32);
	  pix_=QPixmap::fromImage(image.rgbSwapped());
	  update();
	  // emit signal to interested widgets
	  emit mapViewChanged();
	  std::cout << map_->getCurrentExtent() << "\n";
      }
      catch (mapnik::config_error & ex)
      {
	  std::cerr << ex.what() << std::endl;
      }
      catch (...)
      {
	  std::cerr << "Unknown exception caught!\n";
      }
   }
}

boost::shared_ptr<Map> MapWidget::getMap()
{
   return map_;
}

void MapWidget::setMap(boost::shared_ptr<Map> map)
{
   map_ = map;
}


void MapWidget::layerSelected(int index)
{
   selectedLayer_ = index;
}
