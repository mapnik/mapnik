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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QList>
#include <QActionGroup>
#include <QStatusBar>
#include <QAbstractItemModel>
#include <QDoubleSpinBox>

#include "mapwidget.hpp"

// using namespace mapnik;

class LayerTab;
class StyleTab;
class QSlider;
class QComboBox;
class QDoubleSpinBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT
  public:
    MainWindow();
    virtual ~MainWindow();
    void set_default_extent(double x0, double y0, double x1, double y1);
    void set_scaling_factor(double scaling_factor);

  public:
    std::shared_ptr<mapnik::Map> get_map();

  protected:
    void closeEvent(QCloseEvent* event);
  public slots:
    void zoom_all();
    void zoom_to_box();
    void pan();
    void info();
    void export_as();
    void open(QString const& path = QString());
    void reload();
    void save();
    void print();
    void about();
    void pan_left();
    void pan_right();
    void pan_up();
    void pan_down();

  private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createContextMenu();
    void load_map_file(QString const& filename);

    QString currentPath;
    QString filename_;
    QAbstractItemModel* model;
    LayerTab* layerTab_;
    StyleTab* styleTab_;
    MapWidget* mapWidget_;
    // actions
    QList<QAction*> exportAsActs;
    QActionGroup* toolsGroup;

    QAction* zoomAllAct;
    QAction* zoomBoxAct;
    QAction* panAct;
    QAction* infoAct;
    QAction* openAct;
    QAction* saveAct;
    QAction* printAct;
    QAction* exitAct;
    QAction* aboutAct;
    QAction* panLeftAct;
    QAction* panRightAct;
    QAction* panUpAct;
    QAction* panDownAct;
    QAction* reloadAct;
    QAction* layerInfo;
    // toolbars
    QToolBar* fileToolBar;
    QToolBar* editToolBar;
    // menus
    QMenu* exportMenu;
    QMenu* fileMenu;
    QMenu* helpMenu;
    // status bar
    QStatusBar* status;
    QSlider* slider_;
    QComboBox* renderer_selector_;
    QDoubleSpinBox* scale_factor_;
    mapnik::box2d<double> default_extent_;
};

#endif // MAINWINDOW_HPP
