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

#ifndef LAYER_LIST_MODEL_HPP
#define LAYER_LIST_MODEL_HPP

#include <QAbstractListModel>
#include <QModelIndex>
#include <QVariant>
#ifndef Q_MOC_RUN
#include <mapnik/map.hpp>
#endif
#include <boost/optional/optional.hpp>

class LayerListModel : public QAbstractListModel
{
    Q_OBJECT
  public:
    LayerListModel(std::shared_ptr<mapnik::Map> map, QObject* parent = 0);
    int rowCount(QModelIndex const& parent = QModelIndex()) const;
    QVariant data(QModelIndex const& index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    bool setData(QModelIndex const& index, QVariant const& value, int role = Qt::EditRole);
    Qt::ItemFlags flags(QModelIndex const& index) const;
    boost::optional<mapnik::layer&> map_layer(int i);

  private:
    std::shared_ptr<mapnik::Map> map_;
};

#endif // LAYER_LIST_MODEL_HPP
