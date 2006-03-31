/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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
//$Id: shapeindex.cc 27 2005-03-30 21:45:40Z pavlenko $

#include "shape.hpp"
#include "quadtree.hpp"

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <vector>
#include <string>

const int MAXDEPTH = 64;
const int DEFAULT_DEPTH = 8;
const double MINRATIO=0.5;
const double MAXRATIO=0.8;
const double DEFAULT_RATIO=0.55;

int main (int argc,char** argv) 
{
    using namespace mapnik;
    namespace po = boost::program_options;
    using std::string;
    using std::vector;
    
    bool verbose=false;
    unsigned int depth=DEFAULT_DEPTH;
    double ratio=DEFAULT_RATIO;
    vector<string> shape_files;
    
    try
    {
	po::options_description desc("shapeindex utility");
	desc.add_options()
	    ("help,h", "produce usage message")
	    ("version,V","print version string")
	    ("verbose,v","verbose output")
	    ("depth,d", po::value<unsigned int>(), "max tree depth\n(default 8)")   
	    ("ratio,r",po::value<double>(),"split ratio (default 0.55)")
	    ("shape_files",po::value<vector<string> >(),"shape files to index file1 file2 ...fileN")
	    ;
	
	po::positional_options_description p;
	p.add("shape_files",-1);
	po::variables_map vm;        
	po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
	po::notify(vm);
	
	if (vm.count("version"))
	{
	    clog<<"version 0.2.0a" <<endl;
	    return 1;
	}

	if (vm.count("help")) 
	{
	    clog << desc << "\n";
	    return 1;
	}
	if (vm.count("depth"))
	{
	    depth = vm["depth"].as<unsigned int>();
	}
	if (vm.count("ratio"))
	{
	    ratio = vm["ratio"].as<double>();
	}
	
        if (vm.count("shape_files"))
	{
	    shape_files=vm["shape_files"].as< vector<string> >();
	}
    }
    catch (...)
    {
	clog << "Exception of unknown type!"<<endl;
	return -1;
    }
    
    std::clog<<"max tree depth:"<<depth<<std::endl;
    std::clog<<"split ratio:"<<ratio<<std::endl;
  
    vector<string>::const_iterator itr=shape_files.begin();
    if (itr==shape_files.end())
    {
	std::clog << "no shape files to index"<<std::endl;
	return 0;
    }
    while (itr != shape_files.end())
    {
	std::clog<<"processing "<<*itr << std::endl;
	shape_file shp;
	std::string shapename(*itr++);
	if (!shp.open(shapename+".shp")) {
	    std::clog<<"error : cannot open "<< (shapename+".shp") <<"\n";
	    continue;
	}

	shp.read_xdr_integer(); //file_code == 9994
	shp.skip(5*4); 
	
	int file_length=shp.read_xdr_integer();
	int version=shp.read_ndr_integer();
	int shape_type=shp.read_ndr_integer();
	Envelope<double> extent;
	shp.read_envelope(extent);
	
	
	std::clog<<"length="<<file_length<<std::endl;
	std::clog<<"version="<<version<<std::endl;
	std::clog<<"type="<<shape_type<<std::endl;
	std::clog<<"extent:"<<extent<<std::endl;
	  
	int pos=50;
	shp.seek(pos*2);  
	quadtree<int> tree(extent,depth,ratio);
	int count=0;
	while (true) {
	    
	    int offset=shp.pos();
	    int record_number=shp.read_xdr_integer();
	    int content_length=shp.read_xdr_integer();
	    
	    shp.skip(4);
	    std::cout << "offset= "<< offset << std::endl;
	    
	    Envelope<double> item_ext;
	    if (shape_type==shape_io::shape_point)
	    {
		double x=shp.read_double();
		double y=shp.read_double();
		item_ext=Envelope<double>(x,y,x,y);
	
	    }
	    if (shape_type==shape_io::shape_pointm)
	    {
		double x=shp.read_double();
		double y=shp.read_double();
		shp.read_double();
		item_ext=Envelope<double>(x,y,x,y);
	
	    }
	    else if (shape_type==shape_io::shape_pointz)
	    {
		double x=shp.read_double();
		double y=shp.read_double();
		shp.read_double();
		shp.read_double();
		item_ext=Envelope<double>(x,y,x,y);
	    }
	
	    else 
	    {
      
		shp.read_envelope(item_ext);
		shp.skip(2*content_length-4*8-4);

	    }

	    tree.insert(offset,item_ext);
	    if (verbose) {
		std::clog<<"record number "<<record_number<<" box="<<item_ext<<std::endl;
	    }

	    pos+=4+content_length;
	    ++count;

	    if (pos>=file_length) {
		break;
	    }  
	} 
	shp.close();
  
	std::clog<<" number shapes="<<count<<std::endl;  
    
	std::fstream file((shapename+".index").c_str(),
			  std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
	if (!file) {
	    std::clog << "cannot open index file for writing file \""
		      <<(shapename+".index")<<"\""<<std::endl;
	} else {
	    tree.trim();
	    std::clog<<" number nodes="<<tree.count()<<std::endl;
	    tree.write(file);
	    file.close();
	}
    }
    std::clog<<"done!"<<std::endl;
    return 0;
}
