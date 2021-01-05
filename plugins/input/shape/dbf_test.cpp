/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#include <iostream>
#include <iomanip>

#include "dbfile.hpp"

using namespace std;

int main(int argc,char** argv)
{
    if (argc!=2)
    {
        cout<<"usage: dbfdump  <file_name>"<<endl;
        return 0;
    }
    cout << argv[1]<<endl;
    dbf_file dbf(argv[1]);

    cout<<endl;
    for (int i=0;i<dbf.num_records();++i)
    {
        dbf.move_to(i+1);

        for (int j=0;j<dbf.num_fields();++j)
        {
            string name=dbf.descriptor(j).name_;
            int width=dbf.descriptor(j).length_;
            int dec = dbf.descriptor(j).dec_;
            string val=dbf.string_value(j);
            if (dec > 0)
            {
                cout << name << "(" << width <<"," << dec << "):" << val << endl;
            }
            else
            {
                cout << name << "(" << width << "):" << val << endl;
            }
        }
        cout<<endl;
    }
    cout<<"done!"<<endl;
    return 0;
}
