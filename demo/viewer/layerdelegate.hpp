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

#ifndef LAYER_DELEGATE_HPP
#define LAYER_DELEGATE_HPP

#include <QAbstractItemDelegate>
#include <QFontMetrics>
#include <QModelIndex>
#include <QSize>

class QAbstractItemModel;
class QObject;
class QPainter;

class LayerDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

  public:
    LayerDelegate(QObject* parent = 0);
    void paint(QPainter* painter, QStyleOptionViewItem const& option, QModelIndex const& index) const;
    QSize sizeHint(QStyleOptionViewItem const& option, QModelIndex const& index) const;
};

#endif // LAYER_DELEGATE_HPP
