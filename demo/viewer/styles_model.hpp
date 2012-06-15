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

//$Id$

#ifndef STYLE_MODEL_HPP
#define STYLE_MODEL_HPP

#include <QAbstractItemModel>

#ifndef Q_MOC_RUN
#include <mapnik/map.hpp>
#endif

#include <boost/scoped_ptr.hpp>

class node;
class StyleModel : public QAbstractItemModel
{
      Q_OBJECT
  public:
      StyleModel(boost::shared_ptr<mapnik::Map> map, QObject * parent=0);
      ~StyleModel();
      // interface
      QModelIndex index  (int row, int col, QModelIndex const& parent = QModelIndex()) const;
      QModelIndex parent (QModelIndex const& child) const;
      int rowCount( QModelIndex const& parent = QModelIndex()) const;
      int columnCount( QModelIndex const& parent = QModelIndex()) const;
      QVariant  data(const QModelIndex & index, int role = Qt::DisplayRole) const;
   private:
      //boost::shared_ptr<mapnik::Map> map_;
      boost::scoped_ptr<node> root_;
};

#endif // STYLE_MODEL_HPP
