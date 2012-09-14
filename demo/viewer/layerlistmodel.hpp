/* This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
      LayerListModel(boost::shared_ptr<mapnik::Map> map, QObject * parent = 0);
      int rowCount(const QModelIndex &parent = QModelIndex()) const;
      QVariant data(const QModelIndex &index, int role) const;
      QVariant headerData(int section, Qt::Orientation orientation,
                          int role = Qt::DisplayRole) const;
      bool setData(const QModelIndex &index, const QVariant &value,
                   int role = Qt::EditRole);
      Qt::ItemFlags flags(QModelIndex const& index) const;
      boost::optional<mapnik::layer&> map_layer(int i);

   private:
      boost::shared_ptr<mapnik::Map> map_;
};

#endif //LAYER_LIST_MODEL_HPP
