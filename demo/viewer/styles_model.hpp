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

#ifndef STYLE_MODEL_HPP
#define STYLE_MODEL_HPP

#include <QAbstractItemModel>

#ifndef Q_MOC_RUN
#include <mapnik/map.hpp>
#endif

class node;
class StyleModel : public QAbstractItemModel
{
    Q_OBJECT
  public:
    StyleModel(std::shared_ptr<mapnik::Map> map, QObject* parent = 0);
    ~StyleModel();
    // interface
    QModelIndex index(int row, int col, QModelIndex const& parent = QModelIndex()) const;
    QModelIndex parent(QModelIndex const& child) const;
    int rowCount(QModelIndex const& parent = QModelIndex()) const;
    int columnCount(QModelIndex const& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  private:
    // std::shared_ptr<mapnik::Map> map_;
    const std::unique_ptr<node> root_;
};

#endif // STYLE_MODEL_HPP
