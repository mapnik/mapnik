/* This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#include <QtGui>
#include "layerdelegate.hpp"

LayerDelegate::LayerDelegate(QObject* parent)
    : QAbstractItemDelegate(parent)
{}

void LayerDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(QColor(255, 0, 0), 1));

    if (option.state & QStyle::State_Selected)
        painter->setBrush(QBrush(QColor(0, 0, 255, 64)));
    else
        painter->setBrush(QBrush(QColor(255, 0, 0, 64)));

    painter->drawRoundedRect(option.rect, 4, 4);

    if (option.state & QStyle::State_Selected)
        painter->setBrush(option.palette.highlightedText());
    else
        painter->setBrush(QBrush(QColor(255, 120, 0, 127)));
}

QSize LayerDelegate::sizeHint(const QStyleOptionViewItem& /* option */, const QModelIndex& /* index */) const
{
    return QSize(120, 24);
}
