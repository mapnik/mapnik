/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#ifndef DATASET_DELIVERER_H
#define DATASET_DELIVERER_H

#include "osm.h"
#include <string>

using  namespace std;

class dataset_deliverer 
{
private:
    static osm_dataset* dataset; 
    static std::string last_bbox;
    static std::string last_filename;

public:
    static osm_dataset *load_from_file(const string&, const string&);

    static void release()
    {
        delete dataset;
    }
};

#endif // DATASET_DELIVERER_H
