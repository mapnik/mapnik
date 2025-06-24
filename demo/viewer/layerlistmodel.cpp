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

#include "layerlistmodel.hpp"
#include <QIcon>
#include <QBrush>
#include <mapnik/layer.hpp>

using mapnik::Map;

LayerListModel::LayerListModel(std::shared_ptr<Map> map, QObject* parent)
    : QAbstractListModel(parent)
    , map_(map)
{}

int LayerListModel::rowCount(QModelIndex const&) const
{
    if (map_)
        return map_->layers().size();
    return 0;
}

QVariant LayerListModel::data(QModelIndex const& index, int role) const
{
    if (!index.isValid() || !map_)
        return QVariant();
    if (index.row() < 0 || index.row() >= int(map_->layers().size()))
        return QVariant();
    if (role == Qt::DisplayRole)
        return QString(map_->layers().at(index.row()).name().c_str());
    else if (role == Qt::DecorationRole)
    {
        double scale = map_->scale();
        if (map_->layers().at(index.row()).visible(scale))
        {
            return QIcon(":/images/globe.png");
        }
        else
        {
            return QIcon(":/images/globe_bw.png");
        }
    }
    else if (role == Qt::CheckStateRole)
    {
        if (map_->layers().at(index.row()).active())
            return QVariant(Qt::Checked);
        else
            return QVariant(Qt::Unchecked);
    }
    else if (role == Qt::ForegroundRole)
    {
        if (map_->layers().at(index.row()).active())
            return QBrush(QColor("black"));
        else
            return QBrush(QColor("lightgrey"));
    }
    else
    {
        return QVariant();
    }
}

QVariant LayerListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return QString("TODO Column %1").arg(section);
    else
        return QString("TODO Row %1").arg(section);
}

bool LayerListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!map_)
        return false;

    if (index.isValid() && role == Qt::CheckStateRole)
    {
        int status = value.toInt();
        std::vector<mapnik::layer>& layers = const_cast<std::vector<mapnik::layer>&>(map_->layers());
        layers.at(index.row()).set_active(status);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags LayerListModel::flags(QModelIndex const& index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (index.isValid())
        flags |= Qt::ItemIsUserCheckable;
    return flags;
}

boost::optional<mapnik::layer&> LayerListModel::map_layer(int i)
{
    if (map_)
    {
        std::vector<mapnik::layer>& layers = const_cast<std::vector<mapnik::layer>&>(map_->layers());
        if (i < int(layers.size()))
            return boost::optional<mapnik::layer&>(layers[i]);
    }
    return boost::optional<mapnik::layer&>();
}
