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

#include <QtGui>

#include <boost/bind.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/proj_transform.hpp>
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
#include "roadmerger.h"
#include "waitingspinnerwidget.h"

using mapnik::box2d;
using mapnik::coord2d;
using mapnik::feature_kv_iterator;
using mapnik::feature_ptr;
using mapnik::image_rgba8;
using mapnik::layer;
using mapnik::Map;
using mapnik::projection;
using mapnik::scale_denominator;
using mapnik::view_transform;

double scales[] = {279541132.014, 139770566.007, 69885283.0036, 34942641.5018, 17471320.7509,
                   8735660.37545, 4367830.18772, 2183915.09386, 1091957.54693, 545978.773466,
                   272989.386733, 136494.693366, 68247.3466832, 34123.6733416, 17061.8366708,
                   8530.9183354,  4265.4591677,  2132.72958385, 1066.36479192, 533.182395962};

MapWidget::MapWidget(QWidget* parent)
    : QWidget(parent)
    , map_()
    , selected_(1)
    , extent_()
    , cur_tool_(ZoomIn)
    , start_x_(0)
    , start_y_(0)
    , end_x_(0)
    , end_y_(0)
    , drag_(false)
    , first_(true)
    , pen_(QColor(0, 0, 255, 96))
    , selectedLayer_(-1)
    , scaling_factor_(1.0)
    , cur_renderer_(AGG)
{
    pen_.setWidth(3);
    pen_.setCapStyle(Qt::RoundCap);
    pen_.setJoinStyle(Qt::RoundJoin);
    roadMerger = std::make_shared<RoadMerger>(this);
//    roadMerger
    spinner =  new WaitingSpinnerWidget(this,"正在进行数据融合中，请稍后...");
    connect(roadMerger.get(), &RoadMerger::signalMergeStart,this,&MapWidget::onMergeStart);
    connect(roadMerger.get(), &RoadMerger::signalMergeEnd,this,&MapWidget::onMergeEnd);
}

MapWidget::~MapWidget()
{
    spinner->stop();
}

void MapWidget::onMergeStart()
{
    spinner->start();
}

void MapWidget::onMergeEnd()
{
    roadMerger->showRoadLayers();
    spinner->stop();
}

void MapWidget::setTool(eTool tool)
{
    cur_tool_ = tool;
}

void MapWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    if (drag_)
    {
        if (cur_tool_ == ZoomIn || cur_tool_ == ZoomOut)
        {
            unsigned width = end_x_ - start_x_;
            unsigned height = end_y_ - start_y_;
            painter.drawPixmap(QPoint(0, 0), pix_);
            painter.setPen(pen_);
            painter.setBrush(QColor(200, 200, 255, 128));
            painter.drawRect(start_x_, start_y_, width, height);
        }
        else if (cur_tool_ == Pan)
        {
            int dx = end_x_ - start_x_;
            int dy = end_y_ - start_y_;
            painter.setBrush(QColor(200, 200, 200, 128));
            painter.drawRect(0, 0, width(), height());
            painter.drawPixmap(QPoint(dx, dy), pix_);
        }
    }
    else
    {
        painter.drawPixmap(QPoint(0, 0), pix_);
    }
    painter.end();
}

void MapWidget::resizeEvent(QResizeEvent* ev)
{
    if (map_)
    {
        map_->resize(ev->size().width(), ev->size().height());
        updateMap();
    }
}

void MapWidget::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        if (cur_tool_ == ZoomIn || cur_tool_ == Pan || cur_tool_ == ZoomOut)
        {
            start_x_ = e->x();
            start_y_ = e->y();
            drag_ = true;
        }
    }
    else if (e->button() == Qt::RightButton)
    {
        double x = e->x();
        double y = e->y();
        roadMerger->toggleMergedRoad(x,y);
        roadMerger->toggleNeedCompleteRoad(x,y);
    }
}

void MapWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (cur_tool_ == ZoomIn || cur_tool_ == Pan || cur_tool_ == ZoomOut)
    {
        end_x_ = e->x();
        end_y_ = e->y();
        update();
    }
}

void MapWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        end_x_ = e->x();
        end_y_ = e->y();
        if (cur_tool_ == ZoomIn)
        {
            drag_ = false;
            if (map_)
            {
                view_transform t(map_->width(), map_->height(), map_->get_current_extent());
                box2d<double> box = t.backward(box2d<double>(start_x_, start_y_, end_x_, end_y_));
                map_->zoom_to_box(box);
                updateMap();
            }
        }
        else if (cur_tool_ == ZoomOut)
        {
            drag_ = false;
            if (map_)
            {
                view_transform t(map_->width(), map_->height(), map_->get_current_extent());
                mapnik::box2d<double> zoomOutBox = t.backward(box2d<double>(start_x_, start_y_, end_x_, end_y_));
                mapnik::box2d<double> curExtentBox =  map_->get_current_extent();
                double width = curExtentBox.width();
                double height = curExtentBox.height();
                mapnik::coord2d pt = zoomOutBox.center();
                double resW = 2.0;
                double resH = 2.0;
                if (fabs(width)>0.000001 && fabs(zoomOutBox.width())>0.000001)
                {
                   resW = width/zoomOutBox.width();
                }

                if (fabs(height)>0.000001 && fabs(zoomOutBox.height())>0.000001)
                {
                   resH = height/zoomOutBox.height();
                }

                double scale = resW>resH ? resW:resH;
                if (scale<1.0)
                {
                   scale = 2.0;
                }
                mapnik::box2d<double> box(pt.x - 0.5 * width * scale,
                                        pt.y - 0.5 * height * scale,
                                        pt.x + 0.5 * width * scale,
                                        pt.y + 0.5 * height * scale);
                map_->zoom_to_box(box);
                updateMap();
            }
        }
        else if (cur_tool_ == Pan)
        {
            drag_ = false;
            if (map_)
            {
                int cx = int(0.5 * map_->width());
                int cy = int(0.5 * map_->height());
                int dx = end_x_ - start_x_;
                int dy = end_y_ - start_y_;
                map_->pan(cx - dx, cy - dy);
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QPointF corner(map_->width(), map_->height());
    QPointF zoomCoords;
    double zoom;
    if( e->angleDelta().y() == 0 )
        return;
    if (e->angleDelta().y() > 0)
    {
        zoom = 0.5;
        QPointF center = corner / 2;
        QPointF delta = e->position() - center;
        zoomCoords = zoom * delta + center;
    }
    else
    {
        zoom = 2.0;
        zoomCoords = corner - e->position();
    }
#else
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
#endif

    map_->pan_and_zoom(zoomCoords.x(), zoomCoords.y(), zoom);
    updateMap();
}

void MapWidget::keyPressEvent(QKeyEvent* e)
{
    std::cout << "key pressed:" << e->key() << "\n";
    switch (e->key())
    {
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

void MapWidget::zoomAll()
{
    if (map_)
    {
        map_->zoom_all();
        updateMap();
    }
}

void MapWidget::defaultView()
{
    if (map_)
    {
        map_->resize(width(), height());
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
        double cx = 0.5 * map_->width();
        double cy = 0.5 * map_->height();
        map_->pan(int(cx), int(cy - cy * 0.25));
        updateMap();
    }
}

void MapWidget::panDown()
{
    if (map_)
    {
        double cx = 0.5 * map_->width();
        double cy = 0.5 * map_->height();
        map_->pan(int(cx), int(cy + cy * 0.25));
        updateMap();
    }
}

void MapWidget::panLeft()
{
    if (map_)
    {
        double cx = 0.5 * map_->width();
        double cy = 0.5 * map_->height();
        map_->pan(int(cx - cx * 0.25), int(cy));
        updateMap();
    }
}

void MapWidget::panRight()
{
    if (map_)
    {
        double cx = 0.5 * map_->width();
        double cy = 0.5 * map_->height();
        map_->pan(int(cx + cx * 0.25), int(cy));
        updateMap();
    }
}

void MapWidget::zoomToLevel(int level)
{
    if (map_ && level >= 0 && level < 19)
    {
        double scale_denom = scales[level];
        std::cerr << "scale denominator = " << scale_denom << "\n";
        mapnik::box2d<double> ext = map_->get_current_extent();
        double width = static_cast<double>(map_->width());
        double height = static_cast<double>(map_->height());
        mapnik::coord2d pt = ext.center();

        double res = scale_denom * 0.00028;

        mapnik::box2d<double> box(pt.x - 0.5 * width * res,
                                  pt.y - 0.5 * height * res,
                                  pt.x + 0.5 * width * res,
                                  pt.y + 0.5 * height * res);
        map_->zoom_to_box(box);
        updateMap();
    }
}

void MapWidget::export_to_file(unsigned, unsigned, std::string const&, std::string const&)
{
    // image_rgba8 image(width,height);
    // agg_renderer renderer(map,image);
    // renderer.apply();
    // image.saveToFile(filename,type);
    std::cout << "Export to file .." << std::endl;
}

void MapWidget::set_scaling_factor(double scaling_factor)
{
    scaling_factor_ = scaling_factor;
}

void render_agg(mapnik::Map const& map, double scaling_factor, QPixmap& pix)
{
    unsigned width = map.width();
    unsigned height = map.height();

    image_rgba8 buf(width, height);
    mapnik::agg_renderer<image_rgba8> ren(map, buf, scaling_factor);

    try
    {
        mapnik::auto_cpu_timer t(std::clog, "rendering took: ");
        ren.apply();
        QImage image((uchar*)buf.data(), width, height, QImage::Format_ARGB32);
        pix = QPixmap::fromImage(image.rgbSwapped());
    }
    // catch (mapnik::config_error & ex)
    //{
    //     std::cerr << ex.what() << std::endl;
    // }
    catch (std::exception const& ex)
    {
        std::cerr << "exception: " << ex.what() << std::endl;
    } catch (...)
    {
        std::cerr << "Unknown exception caught!\n";
    }
}

void render_grid(mapnik::Map const& map, double scaling_factor, QPixmap& pix)
{
    std::cerr << "Not supported" << std::endl;
}

void render_cairo(mapnik::Map const& map, double scaling_factor, QPixmap& pix)
{
// FIXME
#ifdef HAVE_CAIRO
    mapnik::cairo_surface_ptr image_surface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, map.width(), map.height()),
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
    QImage image((uchar*)data.bytes(), data.width(), data.height(), QImage::Format_ARGB32);
    pix = QPixmap::fromImage(image.rgbSwapped());
#endif
}

void MapWidget::updateRenderer(QString const& txt)
{
    if (txt == "AGG")
        cur_renderer_ = AGG;
    else if (txt == "Cairo")
        cur_renderer_ = Cairo;
    else if (txt == "Grid")
        cur_renderer_ = Grid;
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
        if (cur_renderer_ == AGG)
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
            // projection prj(map_->srs(), true); // map projection
            // box2d<double> ext = map_->get_current_extent();
            // double x0 = ext.minx();
            // double y0 = ext.miny();
            // double x1 = ext.maxx();
            // double y1 = ext.maxy();
            // double z = 0;
            // std::string dest_srs = {"epsg:4326"};
            // mapnik::proj_transform proj_tr(map_->srs(), dest_srs);

            // proj_tr.forward(x0, y0, z);
            // proj_tr.forward(x1, y1, z);
            // std::cout << "MAP SIZE:" << map_->width() << "," << map_->height() << std::endl;
            // std::cout << "BBOX (WGS84): " << x0 << "," << y0 << "," << x1 << "," << y1 << "\n";
            update();
            // emit signal to interested widgets
            emit mapViewChanged();
        }
        catch (std::exception const& ex)
        {
            std::cerr << "exception: " << ex.what() << std::endl;
        } catch (...)
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
