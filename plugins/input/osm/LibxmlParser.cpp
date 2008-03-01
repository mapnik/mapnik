#include <libxml/parser.h>
#include <cstring>
#include <map>
#include "LibxmlParser.h"

#include <iostream>
using namespace std;

void startElement (void *user_state, const xmlChar *name, const xmlChar **attrs)
{
	   cerr<<"Start element"<<endl;
	int count=0;
	read_state *state = static_cast<read_state*>(user_state);
	std::string cur_key, cur_value;
	
	if(!xmlStrcmp(name,(const xmlChar*)("node")))
	{
	   cerr<<"Found a node"<<endl;
		state->in_node=true;
		osm_node *node = new osm_node;
		if(attrs!=NULL)
		{
			while(attrs[count]!=NULL)
			{
				if(!xmlStrcmp(attrs[count],(const xmlChar*)"lat"))
					node->lat = atof((const char*)attrs[count+1]);
				else if(!xmlStrcmp(attrs[count],
								(const xmlChar*)"lon"))
				{
					node->lon = atof((const char*)attrs[count+1]);
				}
				else if(!xmlStrcmp
								(attrs[count],(const xmlChar*)"id"))
				{
					node->id = atoi((const char*)attrs[count+1]);
				}
				count+=2;
			}
		}
		state->cur_osm_item = node;
		state->tmp_node_store[node->id] = node;
	}
	else if (!xmlStrcmp(name,(const xmlChar*)"tag") &&
			(state->in_node==true || state->in_way==true) )
	{
		while(attrs[count]!=NULL)
		{
			if(!xmlStrcmp(attrs[count],(const xmlChar*)"k"))
				cur_key = (const char*) attrs[count+1];
			else if(!xmlStrcmp(attrs[count],(const xmlChar*)"v"))
				cur_value = (const char*) attrs[count+1];
			count+=2;
		}
		state->cur_osm_item->keyvals[cur_key] = cur_value;
	}
	else if(!xmlStrcmp(name,(const xmlChar*)"way"))
	{
	   cerr<<"Found a way"<<endl;
		state->in_way=true;
		osm_way *way = new osm_way;
		if(attrs!=NULL)
		{
			while(attrs[count]!=NULL)
			{
				if(!xmlStrcmp(attrs[count],(const xmlChar*)"id"))
					way->id = atoi((const char*)attrs[count+1]);
				count+=2;
			}
		}
		state->cur_osm_item = way;
	}
	else if(!xmlStrcmp(name,(const xmlChar*)"nd") &&
			state->in_way==true)
	{
		while(attrs[count]!=NULL)
		{
			if(!xmlStrcmp(attrs[count],(const xmlChar*)"ref"))
			{
				int id = atoi((const char*)attrs[count+1]);
				if(state->tmp_node_store.find(id)!=state->tmp_node_store.end())
				{
					(static_cast<osm_way*>(state->cur_osm_item))->nodes.
									push_back(state->tmp_node_store[id]);
				}
			}
			count+=2;
		}
	}
}

void endElement (void *user_state, const xmlChar *name)
{
	read_state *state = static_cast<read_state*>(user_state);
	if(!xmlStrcmp(name,(const xmlChar*)"node"))
	{
		state->in_node=false;
		state->osm_items->add_node(static_cast<osm_node*>
							(state->cur_osm_item));
	}
	if(!xmlStrcmp(name,(const xmlChar*)"way"))
	{
		state->in_way=false;
		state->osm_items->add_way(static_cast<osm_way*>
								(state->cur_osm_item));
	}
}

void characters (void *user_state, const xmlChar *ch, int len)
{
}	
