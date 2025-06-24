/* This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef MAP_WIDGET_HPP
#define MAP_WIDGET_HPP

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QPen>
#include <QItemSelection>
#include <iostream>
#include <string>
#include <memory>

#ifndef Q_MOC_RUN
#include <mapnik/map.hpp>
#endif

class MapWidget : public QWidget
{
    Q_OBJECT

  public:
    enum eTool {
        ZoomToBox = 1,
        Pan,
        Info,
    };

    enum eRenderer { AGG, Cairo, Grid };

  private:
    std::shared_ptr<mapnik::Map> map_;
    int selected_;
    QPixmap pix_;
    mapnik::box2d<double> extent_;
    eTool cur_tool_;
    int start_x_;
    int start_y_;
    int end_x_;
    int end_y_;
    bool drag_;
    bool first_;
    QPen pen_;
    int selectedLayer_;
    double scaling_factor_;
    eRenderer cur_renderer_;

  public:
    MapWidget(QWidget* parent = 0);
    void setTool(eTool tool);
    std::shared_ptr<mapnik::Map> getMap();
    inline QPixmap const& pixmap() const { return pix_; }
    void setMap(std::shared_ptr<mapnik::Map> map);
    void defaultView();
    void zoomToBox(mapnik::box2d<double> const& box);
    void zoomIn();
    void zoomOut();
    void panLeft();
    void panRight();
    void panUp();
    void panDown();
    void set_scaling_factor(double);
  public slots:
    void zoomToLevel(int level);
    void updateMap();
    void layerSelected(int);
    void updateRenderer(int);
    void updateScaleFactor(double scale_factor);
  signals:
    void mapViewChanged();

  protected:
    void paintEvent(QPaintEvent* ev);
    void resizeEvent(QResizeEvent* ev);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);
    void keyPressEvent(QKeyEvent* e);
    void export_to_file(unsigned width, unsigned height, std::string const& filename, std::string const& type);
};

#endif // MAP_WIDGET_HPP
