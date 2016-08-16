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


#include <QtGui>

#include <boost/bind.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/scale_denominator.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/transform_path_adapter.hpp>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/feature_kv_iterator.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/util/timer.hpp>

#ifdef HAVE_CAIRO
// cairo
#include <mapnik/cairo/cairo_image_util.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>
#endif

#include "mapwidget.hpp"
#include "info_dialog.hpp"

using mapnik::image_rgba8;
using mapnik::Map;
using mapnik::layer;
using mapnik::box2d;
using mapnik::coord2d;
using mapnik::feature_ptr;
using mapnik::view_transform;
using mapnik::projection;
using mapnik::scale_denominator;
using mapnik::feature_kv_iterator;

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
     selectedLayer_(-1),
     scaling_factor_(1.0),
     cur_renderer_(AGG)
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
         painter.setBrush(QColor(200,200,255,128));
         painter.drawRect(start_x_,start_y_,width,height);
      }
      else if (cur_tool_ == Pan)
      {
         int dx = end_x_-start_x_;
         int dy = end_y_-start_y_;
         painter.setBrush(QColor(200,200,200,128));
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
            double scale_denom = scale_denominator(map_->scale(),map_proj.is_geographic());
            view_transform t(map_->width(),map_->height(),map_->get_current_extent());

            for (unsigned index = 0; index <  map_->layer_count();++index)
            {
               if (int(index) != selectedLayer_) continue;

               layer & layer = map_->layers()[index];
               if (!layer.visible(scale_denom)) continue;
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
                       feature_kv_iterator itr(*feat,true);
                       feature_kv_iterator end(*feat);

                       for ( ;itr!=end; ++itr)
                       {
                           info.push_back(QPair<QString,QString>(QString(std::get<0>(*itr).c_str()),
                                                                 std::get<1>(*itr).to_string().c_str()));
                       }

#if 0 //
                       using path_type = mapnik::transform_path_adapter<mapnik::view_transform,mapnik::vertex_adapter>;

                       for  (unsigned i=0; i < feat->num_geometries();++i)
                       {
                           mapnik::geometry_type const& geom = feat->get_geometry(i);
                           mapnik::vertex_adapter va(geom);
                           path_type path(t,va,prj_trans);
                           if (va.size() > 0)
                           {
                               QPainterPath qpath;
                               double x,y;
                               va.vertex(&x,&y);
                               qpath.moveTo(x,y);
                               for (unsigned j = 1; j < geom.size(); ++j)
                               {
                                   va.vertex(&x,&y);
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
#endif
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
                                           bind(&layer::name,_1) == "*annotations*")
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
            view_transform t(map_->width(),map_->height(),map_->get_current_extent());
            box2d<double> box = t.backward(box2d<double>(start_x_,start_y_,end_x_,end_y_));
            map_->zoom_to_box(box);
            updateMap();
         }
      }
      else if (cur_tool_==Pan)
      {
         drag_=false;
         if (map_)
         {
            int cx = int(0.5 * map_->width());
            int cy = int(0.5 * map_->height());
            int dx = end_x_ - start_x_;
            int dy = end_y_ - start_y_;
            map_->pan(cx - dx ,cy - dy);
            updateMap();
         }
      }
   }
}

void MapWidget::wheelEvent(QWheelEvent* e)
{
   if (!map_)
   {
      return;
   }

   QPoint corner(map_->width(), map_->height());
   QPoint zoomCoords;
   double zoom;
   if (e->delta() > 0)
   {
      zoom = 0.5;
      QPoint center = corner / 2;
      QPoint delta = e->pos() - center;
      zoomCoords = zoom * delta + center;
   }
   else
   {
      zoom = 2.0;
      zoomCoords = corner - e->pos();
   }

   map_->pan_and_zoom(zoomCoords.x(), zoomCoords.y(), zoom);
   updateMap();
}

void MapWidget::keyPressEvent(QKeyEvent *e)
{
   std::cout << "key pressed:"<< e->key()<<"\n";
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
   default:
       QWidget::keyPressEvent(e);
   }


}

void MapWidget::zoomToBox(mapnik::box2d<double> const& bbox)
{
   if (map_)
   {
      map_->zoom_to_box(bbox);
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
      double cx = 0.5*map_->width();
      double cy = 0.5*map_->height();
      map_->pan(int(cx),int(cy - cy*0.25));
      updateMap();
   }
}

void MapWidget::panDown()
{
   if (map_)
   {
      double cx = 0.5*map_->width();
      double cy = 0.5*map_->height();
      map_->pan(int(cx),int(cy + cy*0.25));
      updateMap();
   }
}

void MapWidget::panLeft()
{
   if (map_)
   {
      double cx = 0.5*map_->width();
      double cy = 0.5*map_->height();
      map_->pan(int(cx - cx * 0.25),int(cy));
      updateMap();
   }
}

void MapWidget::panRight()
{
   if (map_)
   {
      double cx = 0.5*map_->width();
      double cy = 0.5*map_->height();
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
      mapnik::box2d<double> ext = map_->get_current_extent();
      double width = static_cast<double>(map_->width());
      double height= static_cast<double>(map_->height());
      mapnik::coord2d pt = ext.center();

      double res = scale_denom * 0.00028;

      mapnik::box2d<double> box(pt.x - 0.5 * width * res,
                                   pt.y - 0.5 * height*res,
                                   pt.x + 0.5 * width * res,
                                   pt.y + 0.5 * height*res);
      map_->zoom_to_box(box);
      updateMap();
   }
}

void MapWidget::export_to_file(unsigned ,unsigned ,std::string const&,std::string const&)
{
   //image_rgba8 image(width,height);
   //agg_renderer renderer(map,image);
   //renderer.apply();
   //image.saveToFile(filename,type);
    std::cout << "Export to file .." << std::endl;
}

void MapWidget::set_scaling_factor(double scaling_factor)
{
    scaling_factor_ = scaling_factor;
}

void render_agg(mapnik::Map const& map, double scaling_factor, QPixmap & pix)
{
    unsigned width=map.width();
    unsigned height=map.height();

    image_rgba8 buf(width,height);
    mapnik::agg_renderer<image_rgba8> ren(map,buf,scaling_factor);

    try
    {
        mapnik::auto_cpu_timer t(std::clog, "rendering took: ");
        ren.apply();
        QImage image((uchar*)buf.data(),width,height,QImage::Format_ARGB32);
        pix = QPixmap::fromImage(image.rgbSwapped());
    }
    //catch (mapnik::config_error & ex)
    //{
    //    std::cerr << ex.what() << std::endl;
    //}
    catch (std::exception const& ex)
    {
        std::cerr << "exception: " << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown exception caught!\n";
    }
}


void render_grid(mapnik::Map const& map, double scaling_factor, QPixmap & pix)
{
    std::cerr << "Not supported" << std::endl;
}


void render_cairo(mapnik::Map const& map, double scaling_factor, QPixmap & pix)
{
// FIXME
#ifdef HAVE_CAIRO
    mapnik::cairo_surface_ptr image_surface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32,map.width(),map.height()),
                                            mapnik::cairo_surface_closer());
    mapnik::cairo_ptr cairo = mapnik::create_context(image_surface);
    if (cairo)
    {
        mapnik::auto_cpu_timer t(std::clog, "rendering took: ");
        mapnik::cairo_renderer<mapnik::cairo_ptr> renderer(map, cairo, scaling_factor);
        renderer.apply();
    }
    mapnik::image_rgba8 data(map.width(), map.height());
    mapnik::cairo_image_to_rgba8(data, image_surface);
    QImage image((uchar*)data.bytes(),data.width(),data.height(),QImage::Format_ARGB32);
    pix = QPixmap::fromImage(image.rgbSwapped());
#endif
}

void MapWidget::updateRenderer(QString const& txt)
{
    if (txt == "AGG") cur_renderer_ = AGG;
    else if (txt == "Cairo") cur_renderer_ = Cairo;
    else if (txt == "Grid") cur_renderer_ = Grid;
    std::cerr << "Update renderer called" << std::endl;
    updateMap();
}

void MapWidget::updateScaleFactor(double scale_factor)
{
    set_scaling_factor(scale_factor);
    updateMap();
}

void MapWidget::updateMap()
{
   if (map_)
   {
       if (cur_renderer_== AGG)
       {
           render_agg(*map_, scaling_factor_, pix_);
       }
       else if (cur_renderer_ == Cairo)
       {
           render_cairo(*map_, scaling_factor_, pix_);
       }
       else if (cur_renderer_ == Grid)
       {
           render_grid(*map_, scaling_factor_, pix_);
       }
       else
       {
           std::cerr << "Unknown renderer..." << std::endl;
       }

       try
       {
          projection prj(map_->srs()); // map projection
          box2d<double> ext = map_->get_current_extent();
          double x0 = ext.minx();
          double y0 = ext.miny();
          double x1 = ext.maxx();
          double y1 = ext.maxy();
          prj.inverse(x0,y0);
          prj.inverse(x1,y1);
          std::cout << "BBOX (WGS84): " << x0 << "," << y0 << "," << x1 << "," << y1 << "\n";
          update();
          // emit signal to interested widgets
          emit mapViewChanged();
      }
      catch (...)
      {
          std::cerr << "Unknown exception caught!\n";
      }
   }
}

std::shared_ptr<Map> MapWidget::getMap()
{
   return map_;
}

void MapWidget::setMap(std::shared_ptr<Map> map)
{
   map_ = map;
}


void MapWidget::layerSelected(int index)
{
   selectedLayer_ = index;
}
