#ifndef OSM_H
#define OSM_H

#include <vector>
#include <string>
#include <map>
#include <set>
#include <utility>

struct bounds
{
    double w,s,e,n;
    bounds() { w=-180; s=-90; e=180; n=90; }
    bounds(double w, double s, double e, double n )
    { 
        this->w = w;
        this->s = s;
        this->e = e;
        this->n = n;
    }
};

class polygon_types
{
public:
    std::vector<std::pair<std::string,std::string> > ptypes;

    polygon_types()
    {
        ptypes.push_back(std::pair<std::string,std::string>("natural","wood"));
        ptypes.push_back(std::pair<std::string,std::string>("natural","water"));
        ptypes.push_back(std::pair<std::string,std::string>("natural","heath"));
        ptypes.push_back(std::pair<std::string,std::string>("natural","marsh"));
        ptypes.push_back(std::pair<std::string,std::string>("military",
				"danger_area"));
        ptypes.push_back(std::pair<std::string,std::string>
                ("landuse","forest"));
        ptypes.push_back(std::pair<std::string,std::string>
                ("landuse","industrial"));
    }
};

struct osm_item
{
    long id;
    std::map<std::string,std::string> keyvals;    
    virtual std::string to_string();
	virtual ~osm_item() { }
};


struct osm_node: public osm_item
{
    double lat, lon;
    std::string to_string();
};

struct osm_way: public osm_item
{
    std::vector<osm_node*> nodes; 
    std::string to_string();
    bounds get_bounds();
    bool is_polygon();
    static polygon_types ptypes;
};

class osm_dataset
{
private:
    int next_item_mode;
    enum {Node, Way };
    std::vector<osm_node*>::iterator node_i;
    std::vector<osm_way*>::iterator way_i;
    std::vector<osm_node*> nodes;
    std::vector<osm_way*> ways; 

public:
    osm_dataset() { node_i=nodes.begin(); way_i=ways.begin();
                    next_item_mode=Node; }
    osm_dataset(const char* name) 
        { node_i=nodes.begin(); way_i=ways.begin();
         next_item_mode=Node; load(name); }
    bool load(const char* name,const std::string& parser="libxml2");
    bool load_from_url(const std::string&,const std::string&,
                const std::string& parser="libxml2");
    ~osm_dataset();
    void clear();
    void add_node(osm_node* n) { nodes.push_back(n); }
    void add_way(osm_way* w) { ways.push_back(w); }
    std::string to_string();
    bounds get_bounds();
    std::set<std::string> get_keys();
    void rewind_nodes() { node_i=nodes.begin(); }
    void rewind_ways() { way_i=ways.begin(); }
    void rewind() { rewind_nodes(); rewind_ways(); next_item_mode=Node; }
    osm_node * next_node();
    osm_way * next_way();
    osm_item * next_item();
    bool current_item_is_node() { return next_item_mode==Node; }
    bool current_item_is_way() { return next_item_mode==Way; }
};

#endif // OSM_H
