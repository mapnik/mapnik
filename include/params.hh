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

//$Id$

#ifndef PARAMS_HH
#define PARAMS_HH

#include <map>

namespace mapnik
{

    typedef std::pair<std::string,std::string> Parameter;

    class Parameters
    {
        typedef std::map<std::string,std::string> ParamMap;
    private:
	ParamMap data_;
    public:
	typedef ParamMap::const_iterator const_iterator;
	Parameters() {}
	const std::string get(const std::string& name) const;
	void add(const Parameter& param);
	void add(const std::string& name,const std::string& value);
	const_iterator begin() const;
	const_iterator end() const;
	virtual ~Parameters();
    };
}
#endif                                            //PARAMS_HH
