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

// stl
#include <iostream>

// qt
#include <QtGui>
#include <QToolBar>
// mapnik

#ifndef Q_MOC_RUN // QT moc chokes on BOOST_JOIN
//#include <mapnik/config_error.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/util/timer.hpp>
#include <mapnik/layer.hpp>
#endif

// qt
#include "mainwindow.hpp"
#include "roadmerger.h"

// boost
#include <boost/algorithm/string.hpp>

using mapnik::layer;

MainWindow::MainWindow()
{
    mapWidget_ = new MapWidget(this);

    mapWidget_->setFocusPolicy(Qt::StrongFocus);
    mapWidget_->setFocus();

    // setCentralWidget(mapWidget_);
    setCentralWidget(mapWidget_);
    createActions();
    createToolBars();
    setWindowTitle(tr("飞渡-路网融合助手"));
    resize(800, 600);
}

MainWindow::~MainWindow()
{
    delete mapWidget_;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    event->accept();
}

void MainWindow::zoom_to_box()
{
    mapWidget_->setTool(MapWidget::ZoomToBox);
}

void MainWindow::pan()
{
    mapWidget_->setTool(MapWidget::Pan);
}

void MainWindow::save()
{
    std::vector<long> result;
    mapWidget_->roadMerger->getMergeResult(result);

    // TODO： 将result转换成mwmId的json数组并保存文件
}

void MainWindow::createActions()
{
    zoomAllAct = new QAction(QIcon(":/images/home.png"), tr("全部"), this);
    connect(zoomAllAct, SIGNAL(triggered()), this, SLOT(zoom_all()));

    zoomBoxAct = new QAction(QIcon(":/images/zoombox.png"), tr("框选缩放"), this);
    zoomBoxAct->setCheckable(true);
    connect(zoomBoxAct, SIGNAL(triggered()), this, SLOT(zoom_to_box()));

    panAct = new QAction(QIcon(":/images/pan.png"), tr("平移"), this);
    panAct->setCheckable(true);
    connect(panAct, SIGNAL(triggered()), this, SLOT(pan()));

    toolsGroup = new QActionGroup(this);
    toolsGroup->addAction(zoomBoxAct);
    toolsGroup->addAction(panAct);
    panAct->setChecked(true);


    aboutAct = new QAction(QIcon(":/images/about.png"), tr("&保存"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(save()));
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("Actions"));
    fileToolBar->addAction(zoomAllAct);
    fileToolBar->addAction(zoomBoxAct);
    fileToolBar->addAction(panAct);
    fileToolBar->addAction(aboutAct);
}

void MainWindow::zoom_all()
{
    std::shared_ptr<mapnik::Map> map_ptr = mapWidget_->getMap();
    if (map_ptr)
    {
        map_ptr->zoom_all();
        mapnik::box2d<double> const& ext = map_ptr->get_current_extent();
        mapWidget_->zoomToBox(ext);
    }
}

std::shared_ptr<mapnik::Map> MainWindow::get_map()
{
    return mapWidget_->getMap();
}
