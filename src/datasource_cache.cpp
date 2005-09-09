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

//$Id: datasource_cache.cpp 23 2005-03-22 22:16:34Z pavlenko $

#include "datasource_cache.hpp"

#include <algorithm>
#include <stdexcept>
#include <boost/filesystem/operations.hpp>

namespace mapnik
{
    using namespace std;
    using namespace boost;
     
    datasource_cache::datasource_cache()
    {
        if (lt_dlinit()) throw;
    }

    datasource_cache::~datasource_cache()
    {
        lt_dlexit();
    }

    std::map<string,ref_ptr<PluginInfo> > datasource_cache::plugins_;
    bool datasource_cache::registered_=false;
        
    datasource_p datasource_cache::create(const Parameters& params)
    {
        datasource *ds=0;
        try
        {
            std::string type=params.get("type");	    
            map<string,ref_ptr<PluginInfo> >::iterator itr=plugins_.find(type);
            if (itr!=plugins_.end())
            {
                if (itr->second->handle())
                {
                    create_ds* create_datasource = (create_ds*) lt_dlsym(itr->second->handle(), "create");
                    if (!create_datasource)
                    {
                        std::cerr << "Cannot load symbols: " << lt_dlerror() << std::endl;
                    }
                    else
                    {
                        ds=create_datasource(params);
                    }
                }
                else
                {
                    std::cerr << "Cannot load library: " << "  "<< lt_dlerror() << std::endl;
                }
            }
            std::cout<<"datasource="<<ds<<" type="<<type<<std::endl;
        }
	catch (datasource_exception& ex)
	{
	    std::cerr<<ex.what()<<std::endl;
	}
        catch (...)
        {
            std::cerr<<"exception caught "<<std::endl;
        }
        return ref_ptr<datasource,datasource_delete>(ds);
    }

    bool datasource_cache::insert(const std::string& type,const lt_dlhandle module)
    {	      
	return plugins_.insert(make_pair(type,ref_ptr<PluginInfo>(new PluginInfo(type,module)))).second;     
    }

    void datasource_cache::register_datasources(const std::string& str)
    {	
	Lock lock(&mutex_);
	filesystem::path path(str);
	filesystem::directory_iterator end_itr;
	if (exists(path))
        {
	    for (filesystem::directory_iterator itr(path);itr!=end_itr;++itr )
	    {
		if (!is_directory( *itr ) && itr->leaf()[0]!='.')
                {
		    lt_dlhandle module=lt_dlopenext(itr->string().c_str());
		    if (module)
		    {
			datasource_name* ds_name = (datasource_name*) lt_dlsym(module, "datasource_name");
			if (ds_name && insert(ds_name(),module))
			{                           
			    std::cout<<"registered datasource : "<<ds_name()<<std::endl;
			    registered_=true;
			}
		    }
		    else
		    {
			std::cerr<<lt_dlerror()<<std::endl;
		    }
                }
            }   
        }	
    }
}
