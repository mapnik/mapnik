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

#ifndef INFO_DIALOG_HPP
#define INFO_DIALOG_HPP

#include "ui_info.h"
#include <QDialog>

class info_dialog : public QDialog
{
    Q_OBJECT
  public:
    info_dialog(QVector<QPair<QString, QString>> const& info, QWidget* parent = 0);

  private:
    Ui::InfoDialog ui;
};

#endif // INFO_DIALOG_HPP
