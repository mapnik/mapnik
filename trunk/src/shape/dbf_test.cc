/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
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

#include <iostream>
#include <iomanip>
#include "dbffile.hh"

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
        if (!(i%10))
        {
            for (int j=0;j<dbf.num_fields();++j)
            {
                int width=dbf.descriptor(j).length_;
                string name=dbf.descriptor(j).name_;
                cout<<setw(width)<<name<<"|";
            }
            cout<<endl;
        }
        for (int j=0;j<dbf.num_fields();++j)
        {
            int width=dbf.descriptor(j).length_;
            string val=dbf.string_value(j);
            cout <<setw(width)<<val<<"|";
        }
        cout<<endl;
    }
    cout<<"done!"<<endl;
    return 0;
}
