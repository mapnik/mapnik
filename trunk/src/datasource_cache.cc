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

//$Id: dsfactory.cc 67 2004-11-23 10:04:32Z artem $

#include "datasource_cache.hh"
#include "config.hh"
#include <unistd.h>
#include <dirent.h>
#include <algorithm>
#include <stdexcept>

#ifndef _DATASOURCE_PLUGINS_DIR
#error "_DATASOURCE_PLUGINS_DIR must be defined to compile this file"
#endif

namespace mapnik
{
    using namespace std;

    datasource_cache::datasource_cache()
    {
        if (lt_dlinit()) throw;
    }

    datasource_cache::~datasource_cache()
    {
        lt_dlexit();
    }

    std::map<string,ref_ptr<PluginInfo> > datasource_cache::plugins_;
    

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

    void datasource_cache::insert(const std::string& type,const lt_dlhandle module)
    {
	
        std::map<string,ref_ptr<PluginInfo> >::iterator itr=plugins_.find(type);
        if (itr==plugins_.end())
        {
            plugins_.insert(make_pair(type,ref_ptr<PluginInfo>(new PluginInfo(type,module))));
        }
        else
        {
            std::cerr << "plugin "<< type << " already registered\n";
        }
    }

    void datasource_cache::register_datasources(const std::string& path)
    {	
	Lock lock(&mutex_);
	int errors=0;
        DIR *pdir=opendir (path.c_str());
        if (pdir)
        {
            errors=lt_dlsetsearchpath(path.c_str());
            if (!errors)
            {
                dirent* pfile;
                string::iterator pos;
                while((pfile=readdir(pdir)))
                {
                    std::string file_name(pfile->d_name);

		    if (file_name=="." || file_name=="..")
			continue;
                    std::string::size_type len=file_name.size();
		    
                    if (len>3 && 
			file_name[len-1]=='o' && 
			file_name[len-2]=='s')
                    {
                        lt_dlhandle module=lt_dlopenext(file_name.c_str());
                        if (module)
                        {
                            datasource_name* ds_name = (datasource_name*) lt_dlsym(module, "datasource_name");
                            if (ds_name)
                            {
                                std::cout<<"found datasource plugin : "<<ds_name()<<std::endl;
                                insert(ds_name(),module);
                            }
                        }
                        else
                        {
                            std::cerr<<lt_dlerror()<<std::endl;
                        }
                    }
                }
                closedir(pdir);
            }
        }
    }

    
    namespace 
    {
    
	bool load_datasources()
	{
	    datasource_cache::instance()->register_datasources(_DATASOURCE_PLUGINS_DIR);
	    return true;
	}
	const bool ok=load_datasources();
    }
    
}
