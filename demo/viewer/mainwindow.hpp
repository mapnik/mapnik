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
#include <QSharedPointer>
#include "completeRoadsWidget.hpp"


// using namespace mapnik;

class MainWindow : public QMainWindow
{
    Q_OBJECT
  public:
    MainWindow();
    virtual ~MainWindow();

  public:
    std::shared_ptr<mapnik::Map> get_map();
    bool loadFeatureid2osmid(const QString& jsonPath);
    void loadCehuiTableFields(const QString& cehuiTableIniFilePath);
    void setMidLineJsonPath(const QString& midLineJsonPath);
    void setCompleteRoadsFile(const QString& completeRoadsFilePath);
    bool updateGroupidComboBox(const QString& groupidsFilePath);

  protected:
    void closeEvent(QCloseEvent* event);

  signals:
     void afterSave_signal();
     void updateCheckedItems_signal(const std::vector<cehuidataInfo>& cehuidataInfoList);
     void quit_signal();

  public slots:
    void zoom_all();
    void zoomIn_to_box();
    void zoomOut_to_box();
    void pan();
    void save();
    void afterSave();
    void startCompleteRoads();
    void finishCompleteRoads(const QString& groupid);

    void OnItemCheckBoxChanged(const QString& id, int status);

    MapWidget* mapWidget(){
        return mapWidget_;
    }

  private:
    void createActions();
    void createToolBars();

    MapWidget* mapWidget_;
    // actions
    QActionGroup* m_toolsGroup;
    QAction* m_zoomAllAct;
    QAction* m_zoomIn;
    QAction* m_zoomOut;
    QAction* m_panAct;
    QAction* m_saveAct;
    QAction* m_completeRoadsAct;

    // toolbars
    QToolBar* m_fileToolBar;
    std::map<long,long> m_osmid2featureid;
    QString m_midLinePath;
    QString m_completeRoadsFile;

    CompleteRoadsWidget* m_completeRoadsWidget;
    QDockWidget* m_dockWidget;

};

#endif // MAINWINDOW_HPP
