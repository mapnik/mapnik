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
#include <QSplitter>
#include <QTreeView>
#include <QListView>
#include <QTabWidget>
#include <QList>
#include <QItemDelegate>
#include <QSlider>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
// mapnik

#ifndef Q_MOC_RUN // QT moc chokes on BOOST_JOIN
//#include <mapnik/config_error.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/save_map.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/util/timer.hpp>
#endif

// qt
#include "mainwindow.hpp"
#include "layerlistmodel.hpp"
#include "styles_model.hpp"
#include "layerwidget.hpp"
#include "layerdelegate.hpp"
#include "about_dialog.hpp"

// boost
#include <boost/algorithm/string.hpp>

MainWindow::MainWindow()
    : filename_()
    , default_extent_(-20037508.3428, -20037508.3428, 20037508.3428, 20037508.3428)
{
    mapWidget_ = new MapWidget(this);
    QSplitter* splitter = new QSplitter(this);
    QTabWidget* tabWidget = new QTabWidget;
    layerTab_ = new LayerTab;
    layerTab_->setFocusPolicy(Qt::NoFocus);
    layerTab_->setIconSize(QSize(16, 16));

    // LayerDelegate *delegate = new LayerDelegate(this);
    // layerTab_->setItemDelegate(delegate);
    // layerTab_->setItemDelegate(new QItemDelegate(this));
    // layerTab_->setViewMode(QListView::IconMode);

    layerTab_->setFlow(QListView::TopToBottom);
    tabWidget->addTab(layerTab_, tr("Layers"));

    // Styles tab
    styleTab_ = new StyleTab;
    tabWidget->addTab(styleTab_, tr("Styles"));
    splitter->addWidget(tabWidget);
    splitter->addWidget(mapWidget_);
    QList<int> list;
    list.push_back(200);
    list.push_back(600);
    splitter->setSizes(list);

    mapWidget_->setFocusPolicy(Qt::StrongFocus);
    mapWidget_->setFocus();

    // setCentralWidget(mapWidget_);
    setCentralWidget(splitter);
    createActions();
    createMenus();
    createToolBars();
    createContextMenu();

    setWindowTitle(tr("Mapnik Viewer"));
    status = new QStatusBar(this);
    status->showMessage(tr(""));
    setStatusBar(status);
    resize(800, 600);

    // connect mapview to layerlist
    connect(mapWidget_, SIGNAL(mapViewChanged()), layerTab_, SLOT(update()));
    // slider
    connect(slider_, SIGNAL(valueChanged(int)), mapWidget_, SLOT(zoomToLevel(int)));
    // renderer selector
    connect(renderer_selector_, SIGNAL(currentIndexChanged(int)), mapWidget_, SLOT(updateRenderer(int)));

    // scale factor
    connect(scale_factor_, SIGNAL(valueChanged(double)), mapWidget_, SLOT(updateScaleFactor(double)));
    //
    connect(layerTab_, SIGNAL(update_mapwidget()), mapWidget_, SLOT(updateMap()));
    connect(layerTab_, SIGNAL(layerSelected(int)), mapWidget_, SLOT(layerSelected(int)));
}

MainWindow::~MainWindow()
{
    delete mapWidget_;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    event->accept();
}

void MainWindow::createContextMenu()
{
    layerTab_->setContextMenuPolicy(Qt::ActionsContextMenu);
    layerTab_->addAction(openAct);
    layerTab_->addAction(layerInfo);
}

void MainWindow::open(QString const& path)
{
    if (path.isNull())
    {
        filename_ = QFileDialog::getOpenFileName(this, tr("Open Mapnik file"), currentPath, "*.xml");
    }
    else
    {
        filename_ = path;
    }

    if (!filename_.isEmpty())
    {
        load_map_file(filename_);
        setWindowTitle(tr("%1 - Mapnik Viewer").arg(filename_));
    }
}

void MainWindow::reload()
{
    if (!filename_.isEmpty())
    {
        mapnik::box2d<double> bbox = mapWidget_->getMap()->get_current_extent();
        load_map_file(filename_);
        mapWidget_->zoomToBox(bbox);
        setWindowTitle(tr("%1 - *Reloaded*").arg(filename_));
    }
}

void MainWindow::save()
{
    QString initialPath = QDir::currentPath() + "/untitled.xml";
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save"),
                                                    initialPath,
                                                    tr("%1 Files (*.xml)").arg(QString("Mapnik definition")));
    if (!filename.isEmpty())
    {
        std::cout << "saving " << filename.toStdString() << std::endl;
        mapnik::save_map(*mapWidget_->getMap(), filename.toStdString());
    }
}

void MainWindow::load_map_file(QString const& filename)
{
    std::cout << "loading " << filename.toStdString() << std::endl;
    unsigned width = mapWidget_->width();
    unsigned height = mapWidget_->height();
    std::shared_ptr<mapnik::Map> map(new mapnik::Map(width, height));
    mapWidget_->setMap(map);
    try
    {
        mapnik::auto_cpu_timer t(std::clog, "loading map took: ");
        mapnik::load_map(*map, filename.toStdString());
    }
    catch (std::exception const& ex)
    {
        std::cout << ex.what() << "\n";
    }
    catch (...)
    {
        std::cerr << "Exception caught in load_map\n";
    }
    layerTab_->setModel(new LayerListModel(map, this));
    styleTab_->setModel(new StyleModel(map, this));
    zoom_all();
}

void MainWindow::zoom_to_box()
{
    mapWidget_->setTool(MapWidget::ZoomToBox);
}

void MainWindow::pan()
{
    mapWidget_->setTool(MapWidget::Pan);
}

void MainWindow::info()
{
    mapWidget_->setTool(MapWidget::Info);
}

void MainWindow::pan_left()
{
    mapWidget_->panLeft();
}

void MainWindow::pan_right()
{
    mapWidget_->panRight();
}

void MainWindow::pan_up()
{
    mapWidget_->panUp();
}

void MainWindow::pan_down()
{
    mapWidget_->panDown();
}

void MainWindow::about()
{
    about_dialog dlg;
    dlg.exec();
}

void MainWindow::export_as()
{
    QAction* action = qobject_cast<QAction*>(sender());
    QByteArray fileFormat = action->data().toByteArray();
    QString initialPath = QDir::currentPath() + "/map." + fileFormat;

    QString fileName = QFileDialog::getSaveFileName(
      this,
      tr("Export As"),
      initialPath,
      tr("%1 Files (*.%2);;All Files (*)").arg(QString(fileFormat.toUpper())).arg(QString(fileFormat)));
    if (!fileName.isEmpty())
    {
        QPixmap const& pix = mapWidget_->pixmap();
        pix.save(fileName);
    }
}

void MainWindow::print()
{
    // Q_ASSERT(mapWidget_->pixmap());
    // QPrintDialog dialog(&printer, this);
    // if (dialog.exec()) {
    //    QPainter painter(&printer);
    //    QRect rect = painter.viewport();
    //    QSize size = mapWidget_->pixmap()->size();
    //    size.scale(rect.size(), Qt::KeepAspectRatio);
    //    painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
    //    painter.setWindow(mapWidget_->pixmap()->rect());
    //    painter.drawPixmap(0, 0, *mapWidget_->pixmap());
    // }
}

void MainWindow::createActions()
{
    // exportAct = new QAction(tr("&Export as ..."),this);
    // exportAct->setShortcut(tr("Ctrl+E"));
    // connect(exportAct, SIGNAL(triggered()), this, SLOT(export_as()));
    zoomAllAct = new QAction(QIcon(":/images/home.png"), tr("Zoom All"), this);
    connect(zoomAllAct, SIGNAL(triggered()), this, SLOT(zoom_all()));

    zoomBoxAct = new QAction(QIcon(":/images/zoombox.png"), tr("Zoom To Box"), this);
    zoomBoxAct->setCheckable(true);
    connect(zoomBoxAct, SIGNAL(triggered()), this, SLOT(zoom_to_box()));

    panAct = new QAction(QIcon(":/images/pan.png"), tr("Pan"), this);
    panAct->setCheckable(true);
    connect(panAct, SIGNAL(triggered()), this, SLOT(pan()));

    infoAct = new QAction(QIcon(":/images/info.png"), tr("Info"), this);
    infoAct->setCheckable(true);
    connect(infoAct, SIGNAL(triggered()), this, SLOT(info()));

    toolsGroup = new QActionGroup(this);
    toolsGroup->addAction(zoomBoxAct);
    toolsGroup->addAction(panAct);
    toolsGroup->addAction(infoAct);
    zoomBoxAct->setChecked(true);

    openAct = new QAction(tr("Open Map definition"), this);
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
    saveAct = new QAction(tr("Save Map definition"), this);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    panLeftAct = new QAction(QIcon(":/images/left.png"), tr("&Pan Left"), this);
    connect(panLeftAct, SIGNAL(triggered()), this, SLOT(pan_left()));
    panRightAct = new QAction(QIcon(":/images/right.png"), tr("&Pan Right"), this);
    connect(panRightAct, SIGNAL(triggered()), this, SLOT(pan_right()));
    panUpAct = new QAction(QIcon(":/images/up.png"), tr("&Pan Up"), this);
    connect(panUpAct, SIGNAL(triggered()), this, SLOT(pan_up()));
    panDownAct = new QAction(QIcon(":/images/down.png"), tr("&Pan Down"), this);
    connect(panDownAct, SIGNAL(triggered()), this, SLOT(pan_down()));

    reloadAct = new QAction(QIcon(":/images/reload.png"), tr("Reload"), this);
    connect(reloadAct, SIGNAL(triggered()), this, SLOT(reload()));

    layerInfo = new QAction(QIcon(":/images/info.png"), tr("&Layer info"), layerTab_);
    connect(layerInfo, SIGNAL(triggered()), layerTab_, SLOT(layerInfo()));
    connect(layerTab_, SIGNAL(doubleClicked(QModelIndex const&)), layerTab_, SLOT(layerInfo2(QModelIndex const&)));
    foreach (QByteArray format, QImageWriter::supportedImageFormats())
    {
        QString text = tr("%1...").arg(QString(format).toUpper());

        QAction* action = new QAction(text, this);
        action->setData(format);
        connect(action, SIGNAL(triggered()), this, SLOT(export_as()));
        exportAsActs.append(action);
    }

    printAct = new QAction(QIcon(":/images/print.png"), tr("&Print ..."), this);
    printAct->setShortcut(tr("Ctrl+E"));
    connect(printAct, SIGNAL(triggered()), this, SLOT(print()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    aboutAct = new QAction(QIcon(":/images/about.png"), tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
}

void MainWindow::createMenus()
{
    exportMenu = new QMenu(tr("&Export As"), this);
    foreach (QAction* action, exportAsActs)
        exportMenu->addAction(action);

    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addMenu(exportMenu);
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);
    menuBar()->addMenu(fileMenu);

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(aboutAct);
    menuBar()->addMenu(helpMenu);
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("Actions"));
    fileToolBar->addAction(zoomAllAct);
    fileToolBar->addAction(zoomBoxAct);
    fileToolBar->addAction(panAct);
    fileToolBar->addAction(panLeftAct);
    fileToolBar->addAction(panRightAct);
    fileToolBar->addAction(panUpAct);
    fileToolBar->addAction(panDownAct);
    fileToolBar->addAction(infoAct);
    fileToolBar->addAction(reloadAct);
    fileToolBar->addAction(printAct);

    renderer_selector_ = new QComboBox(fileToolBar);
    renderer_selector_->setFocusPolicy(Qt::NoFocus);
    renderer_selector_->addItem("AGG");
#ifdef HAVE_CAIRO
    renderer_selector_->addItem("Cairo");
#endif
    renderer_selector_->addItem("Grid");
    fileToolBar->addWidget(renderer_selector_);

    scale_factor_ = new QDoubleSpinBox(fileToolBar);
    scale_factor_->setMinimum(0.1);
    scale_factor_->setMaximum(10.0);
    scale_factor_->setSingleStep(0.1);
    scale_factor_->setValue(1.0);

    fileToolBar->addWidget(scale_factor_);
    slider_ = new QSlider(Qt::Horizontal, fileToolBar);
    slider_->setRange(1, 18);
    slider_->setTickPosition(QSlider::TicksBelow);
    slider_->setTickInterval(1);
    slider_->setTracking(false);
    fileToolBar->addWidget(slider_);
    fileToolBar->addAction(aboutAct);
}

void MainWindow::set_default_extent(double x0, double y0, double x1, double y1)
{
    try
    {
        std::shared_ptr<mapnik::Map> map_ptr = mapWidget_->getMap();
        if (map_ptr)
        {
            mapnik::projection prj(map_ptr->srs());
            prj.forward(x0, y0);
            prj.forward(x1, y1);
            default_extent_ = mapnik::box2d<double>(x0, y0, x1, y1);
            mapWidget_->zoomToBox(default_extent_);
            std::cout << "SET DEFAULT EXT:" << default_extent_ << std::endl;
        }
    }
    catch (...)
    {}
}

void MainWindow::set_scaling_factor(double scaling_factor)
{
    mapWidget_->set_scaling_factor(scaling_factor);
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
