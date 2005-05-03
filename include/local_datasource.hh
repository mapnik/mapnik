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

#ifndef LOCAL_DATASOURCE_HH
#define LOCAL_DATASOURCE_HH

#include "mapnik.hh"
#include <vector>

namespace mapnik
{
    /*
    class local_datasource : public datasource 
    {
    public:
	local_datasource(Parameters const& params);
	int type() const;
	static std::string name();
	featureset_ptr features(query const& q) const;
	const Envelope<double>& envelope() const;
	virtual ~local_datasource();
    private:
	static std::string name_;
	Envelope<double> extent_;
	std::vector<Feature*>  
    };
    */
}

#endif //LOCAL_DATASOURCE_HH
