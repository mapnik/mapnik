/* This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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



#if !defined ABOUT_DIALOG_HPP
#define ABOUT_DIALOG_HPP

#include "ui_about.h"
#include <QDialog>

class about_dialog : public QDialog
{
  Q_OBJECT
public:
  about_dialog(QWidget * parent = 0);
private:
  Ui::Dialog ui;
};


#endif //ABOUT_DIALOG_HPP
