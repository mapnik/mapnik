#include <libxml/xmlreader.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include "osm.h"
#include <map>


class osmparser
{
private:
    static osm_item *cur_item;
    static long curID;
    static bool in_node, in_way; 
    static osm_dataset* components;
    static std::string error;
    static std::map<long,osm_node*> tmp_node_store;

	static int do_parse(xmlTextReaderPtr);

public:
    static void processNode(xmlTextReaderPtr reader);
    static void startElement(xmlTextReaderPtr reader, const xmlChar *name);
    static void endElement(const xmlChar* name);
    static bool parse(osm_dataset *ds, const char* filename);
    static bool parse(osm_dataset *ds, char* data,int nbytes);
};

