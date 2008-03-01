#include "ExpatParser.h"
#include "osm.h"
#include <string>

using std::cerr;
using std::endl;


 osm_item* ExpatParser::cur_item=NULL;
 int ExpatParser::curID=0;
 bool ExpatParser::in_node=false, ExpatParser::in_way=false;
 osm_dataset* ExpatParser::components=NULL;
 std::string ExpatParser::error="";
 std::map<int,osm_node*> ExpatParser::tmp_node_store = std::map<int,osm_node*>();

void ExpatParser::startElement(void *d,const XML_Char* element,
		const XML_Char** attrs)
{
	double lat, lon;
	int from, to;
	std::string tags; 

		if(!strcmp(element,"node"))
		{
			curID = 0;
			in_node = true;
			int count=0;
			osm_node *node=new osm_node;
			while(attrs[count])
			{
				if(!strcmp(attrs[count],"lat"))
					node->lat = atof(attrs[count+1]);
				if(!strcmp(attrs[count],"lon"))
					node->lon = atof(attrs[count+1]);
				if(!strcmp(attrs[count],"id"))
					node->id = atoi(attrs[count+1]);
				count+=2;
			}

			cur_item = node;	
			tmp_node_store[node->id] = node;

		}
		else if (!strcmp(element,"way"))
		{
			curID=0;
			in_way = true;
			osm_way *way=new osm_way;
			for(int count=0; attrs[count]; count+=2)
			{
				if(!strcmp(attrs[count],"id"))
					way->id = atoi(attrs[count+1]);
			}
			cur_item  =  way; 
		}
		else if (!strcmp(element,"nd") && (in_way))
		{
			int ndid;

			for(int count=0; attrs[count]; count+=2)
			{
				if(!strcmp(attrs[count],"ref"))
				{
					ndid=atoi(attrs[count+1]);
					if(tmp_node_store.find(ndid)!=tmp_node_store.end())
					{
						(static_cast<osm_way*>(cur_item))->nodes.push_back
						(tmp_node_store[ndid]);
					}
				}
			}

		}
		else if (!strcmp(element,"tag"))
		{
			std::string key="", value="";

			for(int count=0; attrs[count]; count+=2)
			{
				if(!strcmp(attrs[count],"k"))
					key = attrs[count+1];
				if(!strcmp(attrs[count],"v"))
				 	value = attrs[count+1];
			}
			cur_item->keyvals[key] = value;
		}
}

void ExpatParser::endElement(void *d,const XML_Char* name)
{
	if(!strcmp(name,"node"))
	{
		in_node = false;
		components->add_node(static_cast<osm_node*>(cur_item));
	}
	else if(!strcmp(name,"way"))
	{
		in_way = false;
		components->add_way(static_cast<osm_way*>(cur_item));
	}


}

void ExpatParser::characters(void*, const XML_Char* txt,int txtlen)
{
}

bool ExpatParser::parse(osm_dataset *ds, std::istream &in)
{
	int done, count=0, n;
	char buf[4096];

	XML_Parser p = XML_ParserCreate(NULL);
	if(!p)
	{
		error = "Error creating parser";
		return false; 
	}

	XML_SetElementHandler(p,ExpatParser::startElement,ExpatParser::endElement);
	XML_SetCharacterDataHandler(p,ExpatParser::characters);
	components = ds ;

	// straight from example
	do
	{
		in.read(buf,4096);
		n = in.gcount();
		done = (n!=4096);
		if(XML_Parse(p,buf,n,done) == XML_STATUS_ERROR)
		{
			error = "xml parsing error";
			return false;
		}
		count += n;
	} while (!done);
	XML_ParserFree(p);
	return true;
}

