// much of this is based on osm2pgsql

#include "osmparser.h"
#include "osm.h"
#include <string>
#include <cassert>
#include <mapnik/util/conversions.hpp>

osm_item* osmparser::cur_item=nullptr;
mapnik::value_integer osmparser::curID=0;
bool osmparser::in_node=false, osmparser::in_way=false;
osm_dataset* osmparser::components=nullptr;
std::string osmparser::error="";
std::map<mapnik::value_integer,osm_node*> osmparser::tmp_node_store=std::map<mapnik::value_integer,osm_node*>();

void osmparser::processNode(xmlTextReaderPtr reader)
{
    xmlChar *name = xmlTextReaderName(reader);
    if(name==nullptr)
        name=xmlStrdup(BAD_CAST "--");

    switch(xmlTextReaderNodeType(reader))
    {
    case XML_READER_TYPE_ELEMENT:
        startElement(reader,name);
        break;

    case XML_READER_TYPE_END_ELEMENT:
        endElement(name);
    }
    xmlFree(name);
}

void osmparser::startElement(xmlTextReaderPtr reader, const xmlChar *name)
{
    std::string tags;
    xmlChar *xid, *xlat, *xlon, *xk, *xv;

    if(xmlStrEqual(name,BAD_CAST "node"))
    {
        curID = 0;
        in_node = true;
        osm_node *node=new osm_node;
        xlat=xmlTextReaderGetAttribute(reader,BAD_CAST "lat");
        xlon=xmlTextReaderGetAttribute(reader,BAD_CAST "lon");
        xid=xmlTextReaderGetAttribute(reader,BAD_CAST "id");
        assert(xlat);
        assert(xlon);
        assert(xid);
        node->lat=atof((char*)xlat);
        node->lon=atof((char*)xlon);
        mapnik::util::string2int((char *)xid, node->id);
        cur_item = node;
        tmp_node_store[node->id] = node;
        xmlFree(xid);
        xmlFree(xlon);
        xmlFree(xlat);
    }
    else if (xmlStrEqual(name,BAD_CAST "way"))
    {
        curID=0;
        in_way = true;
        osm_way *way=new osm_way;
        xid=xmlTextReaderGetAttribute(reader,BAD_CAST "id");
        assert(xid);
        mapnik::util::string2int((char *)xid, way->id);
        cur_item  =  way;
        xmlFree(xid);
    }
    else if (xmlStrEqual(name,BAD_CAST "nd"))
    {
        xid=xmlTextReaderGetAttribute(reader,BAD_CAST "ref");
        assert(xid);
        mapnik::value_integer ndid;
        mapnik::util::string2int((char *)xid, ndid);
        if(tmp_node_store.find(ndid)!=tmp_node_store.end())
        {
            (static_cast<osm_way*>(cur_item))->nodes.push_back
                (tmp_node_store[ndid]);
        }
        xmlFree(xid);
    }
    else if (xmlStrEqual(name,BAD_CAST "tag"))
    {
        std::string key="", value="";
        xk = xmlTextReaderGetAttribute(reader,BAD_CAST "k");
        xv = xmlTextReaderGetAttribute(reader,BAD_CAST "v");
        assert(xk);
        assert(xv);
        cur_item->keyvals[(char*)xk] = (char*)xv;
        xmlFree(xk);
        xmlFree(xv);
    }
    if (xmlTextReaderIsEmptyElement(reader))
    {
        // Fake endElement for empty nodes
        endElement(name);
    }
}

void osmparser::endElement(const xmlChar* name)
{
    if(xmlStrEqual(name,BAD_CAST "node"))
    {
        in_node = false;
        components->add_node(static_cast<osm_node*>(cur_item));
    }
    else if(xmlStrEqual(name,BAD_CAST "way"))
    {
        in_way = false;
        components->add_way(static_cast<osm_way*>(cur_item));
    }
}

bool osmparser::parse(osm_dataset *ds, const char* filename)
{
    components=ds;
    xmlTextReaderPtr reader = xmlNewTextReaderFilename(filename);
    int ret=do_parse(reader);
    xmlFreeTextReader(reader);
    return (ret==0) ?  true:false;
}

bool osmparser::parse(osm_dataset *ds,char* data, int nbytes)
{
    // from cocoasamurai.blogspot.com/2008/10/getting-some-xml-love-with-
    // libxml2.html, converted from Objective-C to straight C

    components=ds;
    xmlTextReaderPtr reader = xmlReaderForMemory(data,nbytes,nullptr,nullptr,0);
    int ret=do_parse(reader);
    xmlFreeTextReader(reader);
    return (ret==0) ? true:false;
}


int osmparser::do_parse(xmlTextReaderPtr reader)
{
    int ret=-1;
    if(reader!=nullptr)
    {
        ret = xmlTextReaderRead(reader);
        while(ret==1)
        {
            processNode(reader);
            ret=xmlTextReaderRead(reader);
        }
    }
    return ret;
}
