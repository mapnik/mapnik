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
#include <QLabel>
#include <QDockWidget>
#include <QDebug>
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

#include <iostream>
#include <fstream>
#include <map>
#include "rapidjson/document.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <QMessageBox>
#include <QCoreApplication>
#include <cstdlib>
#include "groupinfo.hpp"

using mapnik::layer;

MainWindow::MainWindow()
{
    mapWidget_ = new MapWidget(this);

    mapWidget_->setFocusPolicy(Qt::StrongFocus);
    mapWidget_->setFocus();

    // 创建自定义部件
    m_completeRoadsWidget = new CompleteRoadsWidget(this);
    // 创建 QDockWidget 并设置特性
    m_dockWidget =  new QDockWidget(this);
    m_dockWidget->setWidget(m_completeRoadsWidget);
    m_dockWidget->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    // 将 QDockWidget 添加到 QMainWindow
    this->addDockWidget(Qt::LeftDockWidgetArea, m_dockWidget);

    // 设置停靠区域
    m_dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_dockWidget->setVisible(false);

    // setCentralWidget(mapWidget_);
    setCentralWidget(mapWidget_);
    createActions();
    createToolBars();
    setWindowTitle(tr("飞渡-路网融合助手"));
    resize(800, 600);
}

MainWindow::~MainWindow()
{
    // if(mapWidget_)
    // {
    //     delete mapWidget_;
    //     mapWidget_ = NULL;
    // }
    delete mapWidget_;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    event->accept();
}

void MainWindow::zoomIn_to_box()
{
    if (mapWidget_)
    {
        mapWidget_->setTool(MapWidget::ZoomIn);
    }
}
void MainWindow::zoomOut_to_box()
{
    if (mapWidget_)
    {
        mapWidget_->setTool(MapWidget::ZoomOut);
    }
}

void MainWindow::pan()
{
    mapWidget_->setTool(MapWidget::Pan);
}

void MainWindow::save()
{
    std::vector<long> result;
    mapWidget_->roadMerger->getMergeResult(result);
    std::cout<<"size of result:"<<result.size();
    // 将result转换成mwmId的json数组并保存文件
    // 创建一个rapidjson的Document对象，类型为kArrayType
    rapidjson::Document doc;
    doc.SetArray();

    // 获取Document对象的分配器
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    for(int i=0; i<result.size(); ++i)
    {
      long key = result[i];
      if(m_osmid2featureid.find(key)!=m_osmid2featureid.end())
      {
        //jsonArray.append(QJsonValue(qint64(m_osmid2featureid[key])));
        // 将元素转换为rapidjson的Value对象，类型为kNumberType
        rapidjson::Value value(m_osmid2featureid[key]);

        // 将Value对象推入Document对象的数组中
        doc.PushBack(value, allocator);
      }
    }
    // 创建一个StringBuffer对象，用于存储生成的json字符串
    rapidjson::StringBuffer buffer;

    // 创建一个Writer对象，用于将Document对象转换为json字符串
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    // 将Document对象接受Writer对象的访问，生成json字符串
    doc.Accept(writer);

    // 获取生成的json字符串
    const char* json = buffer.GetString();

    // 假设要写入的json文件的路径是"data.json"
    std::ofstream file(m_midLinePath.toStdString());
    if (file.is_open()) {
      // 将json字符串写入文件中
      file << json;
      file.close();
      // 弹出一个对话框，显示文件生成完成的消息
      QMessageBox::information(nullptr, "完成", "成功生成midLine.json");
     }
     else
    {
      QMessageBox::critical(nullptr, "错误", "无法打开文件");
    }

    emit afterSave_signal();
//    QCoreApplication::quit();
}

void MainWindow::afterSave()
{
    m_completeRoadsAct->setCheckable(true);
    m_completeRoadsAct->setEnabled(true);

    m_saveAct->setCheckable(false);
    m_saveAct->setEnabled(false);
}

void MainWindow::OnItemCheckBoxChanged(const QString& id, int status)
{
    mapWidget_->roadMerger->OnItemCheckBoxChanged(id, status);
}

void MainWindow::finishCompleteRoads(const QString& groupid, const QString& version)
{
    // std::vector<cehuidataInfo> result;
    // mapWidget_->roadMerger->getCompleteRoadsResult(result);
    mapWidget_->roadMerger->exportCompleteRoads(m_completeRoadsFile, groupid, version);
    emit completeRoads_quit_signal();
    // QCoreApplication::quit();
    // // 正常退出
    // exit(EXIT_SUCCESS);
}

void MainWindow::startCompleteRoads()
{
    mapWidget_->roadMerger->clearLayers();
    mapWidget_->roadMerger->clipedCehuiData();
    mapWidget_->roadMerger->showClipedCehuiOnMap();
    std::vector<cehuidataInfo> result;
    mapWidget_->roadMerger->getCompleteRoadsResult(result);
    qDebug() << "startCompleteRoads:result size" << result.size();
    emit updateCheckedItems_signal(result);
    m_dockWidget->setVisible(true);

    m_completeRoadsAct->setCheckable(false);
    m_completeRoadsAct->setEnabled(false);
}

void MainWindow::loadCehuiTableFields(const QString& cehuiTableIniFilePath)
{
    if (mapWidget_!=NULL && mapWidget_->roadMerger!=NULL)
    {
        mapWidget_->roadMerger->loadCehuiTableFields(cehuiTableIniFilePath);
    }
}

bool MainWindow::loadFeatureid2osmid(const QString& jsonPath)
{
   std::ifstream file(jsonPath.toStdString());
   if (file.is_open()) {
    // 读取文件内容到一个字符串中
    std::string content((std::istreambuf_iterator<char>(file)),
                        (std::istreambuf_iterator<char>()));
    file.close();

    // 使用rapidjson的Document类解析字符串
    rapidjson::Document doc;
    doc.Parse(content.c_str());

    // 检查是否是有效的json文档
    if (doc.HasParseError()) {
        // 处理错误情况
        return false;
    }

    // 检查是否是json数组
    if (doc.IsArray()) {
        // 获取json数组
        rapidjson::Value& array = doc;

        // 遍历json数组中的每个元素
        for (rapidjson::SizeType i = 0; i < array.Size(); i++) {
            // 检查是否是json数组
            if (array[i].IsArray()) {
                // 获取json数组
                rapidjson::Value& subarray = array[i];

                // 检查是否包含两个元素
                if (subarray.Size() == 2) {
                    // 获取第一个元素，转换为long类型
                    long key = subarray[1].GetInt64();

                    // 获取第二个元素，转换为long类型
                    long value = subarray[0].GetInt64();

                    // 将键值对插入到map中
                    m_osmid2featureid.insert(std::make_pair(key, value));
                }
            }
        }

        return true;
    }
 }
 return false;
}

bool MainWindow::updateGroupidComboBox(const QString& groupidsFilePath)
{
    std::vector<GroupInfo> groupInfoList;
    std::ifstream file(groupidsFilePath.toStdString());
    if (file.is_open()) {
        // 读取文件内容到一个字符串中
        std::string content((std::istreambuf_iterator<char>(file)),
                            (std::istreambuf_iterator<char>()));
        file.close();

        // 使用 rapidjson::Document 解析 JSON 字符串
        rapidjson::Document doc;
        doc.Parse(content.c_str());

        // 检查是否有解析错误
        if (doc.HasParseError()) {
            std::cerr << "JSON 解析错误：" << doc.GetParseError() << std::endl;
            return false;
        }

        // 确保解析得到的是一个数组
        if (!doc.IsArray()) {
            std::cerr << "JSON 不是一个数组" << std::endl;
            return false;
        }

        // 遍历数组
        for (rapidjson::SizeType i = 0; i < doc.Size(); i++) {
            // 获取数组中的对象
            const rapidjson::Value& obj = doc[i];

            // 确保对象包含 "name"、"id" 和 "version" 成员
            if (obj.IsObject() && obj.HasMember("groupName") && obj.HasMember("groupId") && obj.HasMember("groupVersion")) {
                GroupInfo info;
                // 输出 "name" 和 "id"
                info.name = obj["groupName"].GetString();
                info.id = obj["groupId"].GetString();
                std::cout << "groupName: " << obj["groupName"].GetString() << ", groupId: " << obj["groupId"].GetString() << std::endl;

                // 输出 "version" 数组
                const rapidjson::Value& version = obj["groupVersion"];
                if (version.IsArray()) {
                    std::cout << "groupVersion: ";
                    for (rapidjson::SizeType j = 0; j < version.Size(); j++) {
                        // 输出版本号
                        std::cout << version[j].GetInt() << (j != version.Size() - 1 ? ", " : "");
                        info.versions.push_back(version[j].GetInt());
                    }
                    std::cout << std::endl;
                }
                groupInfoList.push_back(info);
            }
        }


        if (m_completeRoadsWidget)
        {
            m_completeRoadsWidget->updateGroupidComboBox(groupInfoList);
        }
        return true;
    }
    return false;
}

void MainWindow::setMidLineJsonPath(const QString& midLineJsonPath)
{
  m_midLinePath = midLineJsonPath;
}

void MainWindow::setCompleteRoadsFile(const QString& completeRoadsFile)
{
    m_completeRoadsFile = completeRoadsFile;
}

void MainWindow::createActions()
{
    m_toolsGroup = new QActionGroup(this);

    // create some smart pointers to actions
    m_zoomAllAct = new QAction(QIcon(":/images/zoomall.png"), tr("全部"), this);
    m_zoomIn = new QAction(QIcon(":/images/zoomin.png"), tr("框选放大"), this);
    m_zoomOut = new QAction(QIcon(":/images/zoomout.png"), tr("框选缩小"), this);
    m_panAct = new QAction(QIcon(":/images/pan.png"), tr("平移"), this);
    m_saveAct = new QAction(QIcon(":/images/save.png"), tr("&保存"), this);
    m_completeRoadsAct = new QAction(QIcon(":/images/home.png"), tr("&补全道路"), this);

    // connect the actions to the slots
    connect(m_zoomAllAct, SIGNAL(triggered()), this, SLOT(zoom_all()));
    connect(m_zoomIn, SIGNAL(triggered()), this, SLOT(zoomIn_to_box()));
    connect(m_zoomOut, SIGNAL(triggered()), this, SLOT(zoomOut_to_box()));
    connect(m_panAct, SIGNAL(triggered()), this, SLOT(pan()));
    connect(m_saveAct, SIGNAL(triggered()), this, SLOT(save()));
    connect(this, SIGNAL(afterSave_signal()), this, SLOT(afterSave()));
    connect(m_completeRoadsAct, SIGNAL(triggered()), this, SLOT(startCompleteRoads()));

    connect(this, SIGNAL(updateCheckedItems_signal(const std::vector<cehuidataInfo>&)), m_completeRoadsWidget, SLOT(updateCheckedItems(const std::vector<cehuidataInfo>&)));
    connect(m_completeRoadsWidget, SIGNAL(itemCheckBoxChanged_signal(const QString&, int)), this, SLOT(OnItemCheckBoxChanged(const QString&, int)));
    connect(m_completeRoadsWidget, SIGNAL(exportCompleteRoads_signal(const QString&, const QString&)), this, SLOT(finishCompleteRoads(const QString&, const QString&)));


    // set some actions as checkable
    m_zoomIn->setCheckable(true);
    m_zoomOut->setCheckable(true);
    m_panAct->setCheckable(true);
    m_saveAct->setCheckable(true);
    m_completeRoadsAct->setCheckable(false);
    m_completeRoadsAct->setEnabled(false);

    // add the actions to the action group
    m_toolsGroup->addAction(m_zoomAllAct);
    m_toolsGroup->addAction(m_zoomIn);
    m_toolsGroup->addAction(m_zoomOut);
    m_toolsGroup->addAction(m_panAct);
    m_toolsGroup->addAction(m_saveAct);
    m_toolsGroup->addAction(m_completeRoadsAct);

}

void MainWindow::createToolBars()
{
    m_fileToolBar = addToolBar(tr("Actions"));
    // add the actions to the tool bar
    m_fileToolBar->addAction(m_zoomAllAct);
    m_fileToolBar->addAction(m_zoomIn);
    m_fileToolBar->addAction(m_zoomOut);
    m_fileToolBar->addAction(m_panAct);
    m_fileToolBar->addAction(m_saveAct);
    m_fileToolBar->addAction(m_completeRoadsAct);
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

void MainWindow::merge(const QString& base, const QString& cehui)
{
    if(mapWidget_ && mapWidget_->roadMerger)
    {
        mapWidget_->roadMerger->merge(base, cehui);
    }
}
