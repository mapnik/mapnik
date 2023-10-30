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

class MainWindow : public QMainWindow
{
    Q_OBJECT
  public:
    MainWindow();
    virtual ~MainWindow();

  public:
    std::shared_ptr<mapnik::Map> get_map();

  protected:
    void closeEvent(QCloseEvent* event);
  public slots:
    void zoom_all();
    void zoom_to_box();
    void pan();
    void save();

    MapWidget* mapWidget(){
        return mapWidget_;
    }

  private:
    void createActions();
    void createToolBars();

    MapWidget* mapWidget_;
    // actions
    QActionGroup* toolsGroup;

    QAction* zoomAllAct;
    QAction* zoomBoxAct;
    QAction* panAct;
    QAction* aboutAct;
    // toolbars
    QToolBar* fileToolBar;
};

#endif // MAINWINDOW_HPP
