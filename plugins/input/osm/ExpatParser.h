#include <expat.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include "osm.h"
#include <map>


class ExpatParser
{
private:
	static osm_item *cur_item;
	static int curID;
	static bool in_node, in_way; 
	static osm_dataset* components;
	static std::string error;
	static std::map<int,osm_node*> tmp_node_store;

	static void startElement(void *d,const XML_Char* name,
		const XML_Char** attrs);
	static void endElement(void *d,const XML_Char* name);
	static void characters(void*, const XML_Char* txt,int txtlen);
public:
	static bool parse(osm_dataset*,std::istream&);
	static std::string getError() { return error; }
};

