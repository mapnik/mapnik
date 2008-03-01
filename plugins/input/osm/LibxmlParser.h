#ifndef XMLPARSE_H
#define XMLPARSE_H

#include <libxml/parser.h>
#include <map>
#include "osm.h"

void startElement (void *user_data, const xmlChar *name, const xmlChar **attrs);
void endElement (void *user_data, const xmlChar *name);
void characters (void *user_data, const xmlChar *ch, int len);

struct read_state
{
	bool in_node,
		in_way;
	osm_item *cur_osm_item;
	osm_dataset *osm_items;
	std::map<int,osm_node*> tmp_node_store;
} ;

#endif
