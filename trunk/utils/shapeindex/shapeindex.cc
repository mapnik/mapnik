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

#include <iostream>
#include <argp.h>
#include "config.hh"
#include "mapnik.hh"
#include "shape.hh"
#include "quadtree.hh"


const int MAXDEPTH = 64;
const int DEFAULT_DEPTH = 16;
const double MINRATIO=0.5;
const double MAXRATIO=0.8;
const double DEFAULT_RATIO=0.55;

const char* argp_program_version = "shapeindex utility version 0.1";
const char* argp_program_bug_address="bugs@shapeindex";

static char doc[] = "shapeindex utility to create spatial index";
 
static char args_doc[]="shape_file";

static struct argp_option options[] = {
    {"verbose",'v',0,  0,"Produce verbose output"},
    {"quite",  'q',0,  0,"Don't produce any output"},
    {"silent", 's',0,  OPTION_ALIAS},
    {0,0,0,0, "shape index options:" },
    {"depth",  'd',"VALUE",0,"maximum depth of the tree (default=16)"},
    {"ratio",  'r',"VALUE",0,"split ratio between 0.5-0.9 (default 0.55)"},
    { 0 }
};

struct arguments {
    char *args[2]; 
    int silent;
    int verbose;
    int depth;
    double ratio;
};

static error_t 
parse_opt ( int key,char *arg, struct argp_state *state) {
    struct arguments* arguments = (struct arguments*)state->input;
    switch (key)  
    {
    case 'q':
    case 's':
	arguments->silent = 1;
	break;
    case 'v':
	arguments->verbose = 1;
	break;
    case 'd':
	arguments->depth = arg ? atoi(arg) : DEFAULT_DEPTH;
	    break;
    case 'r':
	arguments->ratio = arg ? atof(arg) : DEFAULT_RATIO;
	    break; 
    case ARGP_KEY_NO_ARGS:
	argp_usage (state);
    case ARGP_KEY_ARG:
	if (state->arg_num>=1)
	    //too many arguments.
	    argp_usage (state);
	arguments->args[state->arg_num] = arg;
	break;
    case ARGP_KEY_END:
	if (state->arg_num < 1)
	    //not enough arguments
	    argp_usage (state);
	break;
    default:
	return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

using namespace mapnik;

int main (int argc,char** argv) {
  
    struct arguments arguments;
  
    arguments.silent = 0;
    arguments.verbose = 0;
    arguments.silent = 0;
    arguments.depth = DEFAULT_DEPTH;
    arguments.ratio = DEFAULT_RATIO;
    argp_parse(&argp, argc, argv, 0, 0, &arguments);
  
    std::cout<<"processing "<<arguments.args[0]<<std::endl;

    std::cout<<"max tree depth:"<<arguments.depth<<std::endl;
    std::cout<<"split ratio:"<<arguments.ratio<<std::endl;
  
    shape_file shp;
    std::string shapename(arguments.args[0]);
    if (!shp.open(shapename+".shp")) {
	std::cerr<<"error : cannot open "<< (shapename+".shp") <<"\n";
	return -1;
    }

    int file_code=shp.read_xdr_integer();
    assert(file_code==9994);
    shp.skip(5*4);
  
    int file_length=shp.read_xdr_integer();
    int version=shp.read_ndr_integer();
    int shape_type=shp.read_ndr_integer();
    Envelope<double> extent;
    shp.read_envelope(extent);

    if (!arguments.silent) {
	//std::cout<<"length="<<file_length<<std::endl;
	//std::cout<<"version="<<version<<std::endl;
	std::cout<<"type="<<shape_type<<std::endl;
	std::cout<<"extent:"<<extent<<std::endl;
    }
  
    int pos=50;
    shp.seek(pos*2);  
    quadtree<int> tree(extent,arguments.depth,arguments.ratio);
    int count=0;
    while (true) {

	int offset=shp.pos();
	int record_number=shp.read_xdr_integer();
	int content_length=shp.read_xdr_integer();
    
	shp.skip(4);
	Envelope<double> item_ext;
	if (shape_type==shape_io::shape_point) {
    
	    coord2d c(0,0);
	    shp.read_coord(c);
	    shp.skip(2*content_length-2*8-4);
	    item_ext=Envelope<double>(c.x,c.y,c.x,c.y);
	
	} else {
      
	    shp.read_envelope(item_ext);
	    shp.skip(2*content_length-4*8-4);

	}

	tree.insert(offset,item_ext);
	if (arguments.verbose) {
	    std::cout<<"record number "<<record_number<<" box="<<item_ext<<std::endl;
	}

	pos+=4+content_length;
	++count;

	if (pos>=file_length) {
	    break;
	}  
    } 
    shp.close();
  
    std::cout<<" number shapes="<<count<<std::endl;  
    
    std::fstream file((shapename+".index").c_str(),
		      std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
    if (!file) {
	std::cerr << "cannot open index file for writing file \""
		  <<(shapename+".index")<<"\""<<std::endl;
    } else {
	tree.trim();
	
	std::cout<<" number nodes="<<tree.count()<<std::endl;
	

	tree.write(file);
	file.close();
    }
    std::cout<<"done!"<<std::endl;
    return 0;
}
