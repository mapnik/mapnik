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

#include <iostream>
#include <fstream>
#include <map>
#include "rapidjson/document.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <QMessageBox>
#include <QCoreApplication>

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
    if(mapWidget_)
    {
        delete mapWidget_;
        mapWidget_ = NULL;
    }
    
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
    QCoreApplication::quit();
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
        std::cout<<"doc.HasParseError()"<<std::endl;
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

void MainWindow::setMidLineJsonPath(const QString& midLineJsonPath)
{
  m_midLinePath = midLineJsonPath;
}

void MainWindow::createActions()
{
    // create a smart pointer to an action group
    m_toolsGroup = QSharedPointer<QActionGroup>::create(this);

    // create some smart pointers to actions
    m_zoomAllAct = QSharedPointer<QAction>::create(QIcon(":/images/zoomall.png"), tr("全部"), this);
    m_zoomIn = QSharedPointer<QAction>::create(QIcon(":/images/zoomin.png"), tr("框选放大"), this);
    m_zoomOut = QSharedPointer<QAction>::create(QIcon(":/images/zoomout.png"), tr("框选缩小"), this);
    m_panAct = QSharedPointer<QAction>::create(QIcon(":/images/pan.png"), tr("平移"), this);
    m_saveAct = QSharedPointer<QAction>::create(QIcon(":/images/save.png"), tr("&保存"), this);

    // connect the actions to the slots
    connect(m_zoomAllAct.data(), SIGNAL(triggered()), this, SLOT(zoom_all()));
    connect(m_zoomIn.data(), SIGNAL(triggered()), this, SLOT(zoomIn_to_box()));
    connect(m_zoomOut.data(), SIGNAL(triggered()), this, SLOT(zoomOut_to_box()));
    connect(m_panAct.data(), SIGNAL(triggered()), this, SLOT(pan()));
    connect(m_saveAct.data(), SIGNAL(triggered()), this, SLOT(save()));

    // set some actions as checkable
    m_zoomIn->setCheckable(true);
    m_zoomOut->setCheckable(true);
    m_panAct->setCheckable(true);

    // add the actions to the action group
    m_toolsGroup->addAction(m_zoomAllAct.data());
    m_toolsGroup->addAction(m_zoomIn.data());
    m_toolsGroup->addAction(m_zoomOut.data());
    m_toolsGroup->addAction(m_panAct.data());
    m_toolsGroup->addAction(m_saveAct.data());

    // set the default checked action
    m_panAct->setChecked(true);
}

void MainWindow::createToolBars()
{
    // create a smart pointer to a tool bar
    m_fileToolBar = QSharedPointer<QToolBar>::create(tr("Actions"));

    // add the actions to the tool bar
    m_fileToolBar->addAction(m_zoomAllAct.data());
    m_fileToolBar->addAction(m_zoomIn.data());
    m_fileToolBar->addAction(m_zoomOut.data());
    m_fileToolBar->addAction(m_panAct.data());
    m_fileToolBar->addAction(m_saveAct.data());

    // add the tool bar to the widget
    addToolBar(m_fileToolBar.data());
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
