/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2006 Artem Pavlenko
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

#if !defined QUAD_TREE_HPP
#define QUAD_TREE_HPP

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/noncopyable.hpp>
#include <vector>
#include <mapnik/envelope.hpp>


namespace mapnik
{
    template <typename T>
    class quad_tree : boost::noncopyable
    {
	struct node
	{
	    typedef T value_t;
	    typedef std::vector<T> cont_t;
	    typedef typename cont_t::iterator iterator;
	    typedef typename cont_t::const_iterator const_iterator;
	    Envelope<double> extent_;
	    cont_t cont_;
	    node * children_[4];

	    explicit node(Envelope<double> const& ext)
		: extent_(ext)
	    {
		std::memset(children_,0,4*sizeof(node*));
	    }
   
	    Envelope<double> const& extent() const
	    {
		return extent_;
	    }
	    
	    iterator begin() 
	    {
		return cont_.begin();
	    }
	    
	    const_iterator begin() const 
	    {
		return cont_.begin();
	    }
	    
	    iterator end() 
	    {
		return cont_.end();
	    }
	    
	    const_iterator end() const 
	    {
		return cont_.end();
	    }
	    ~node () {}
	};
	
	typedef boost::ptr_vector<node> nodes_t;	
	typedef typename node::cont_t cont_t;
	typedef typename cont_t::iterator node_data_iterator;
	
	nodes_t nodes_;
	node * root_;	
	const double ratio_; 
	
    public:
	typedef typename nodes_t::iterator iterator;
	typedef typename nodes_t::const_iterator const_iterator;
	typedef typename boost::ptr_vector<T,boost::view_clone_allocator> result_t;	
	typedef typename result_t::iterator query_iterator;
   
	result_t query_result_;
	
	explicit quad_tree(Envelope<double> const& ext,double ratio=0.55) 
	    : ratio_(ratio)
	{
	    nodes_.push_back(new node(ext));
	    root_ = &nodes_[0];
	}
		
	void insert(T data, Envelope<double> const& box)
	{
	    do_insert_data(data,box,root_);
	}
        
	query_iterator query_in_box(Envelope<double> const& box)
	{
	    query_result_.clear();
	    query_node(box,query_result_,root_);
	    return query_result_.begin();
	}
	
	query_iterator query_end()
	{
	    return query_result_.end();
	}

	iterator begin()
	{
	    return nodes_.begin();
	}
	
	const_iterator begin() const
	{
	    return nodes_.begin();
	}

	iterator end()
	{
	    return  nodes_.end();
	}
	
	const_iterator end() const
	{
	    return  nodes_.end();
	}
	
    private:
        
	void query_node(Envelope<double> const& box, result_t & result, node * node_) const
	{
	    if (node_)
	    {
		Envelope<double> const& node_extent = node_->extent();
		if (box.intersects(node_extent))
		{
		    node_data_iterator i=node_->begin();
		    node_data_iterator end=node_->end();
		    while ( i!=end)
		    {
			result.push_back(&(*i));
			++i;
		    }
		    for (int k = 0; k < 4; ++k)
		    {
		        query_node(box,result,node_->children_[k]);
		    }
		}
	    }
	}
	
	void do_insert_data(T data, Envelope<double> const& box, node * n)
	{
	    if (n)
	    {
		Envelope<double> const& node_extent = n->extent();
		Envelope<double> ext[4];
		split_box(node_extent,ext);		
		for (int i=0;i<4;++i)
		{
		    if (ext[i].contains(box))
		    {
			if (!n->children_[i])
			{
			    nodes_.push_back(new node(ext[i]));
			    n->children_[i]=&nodes_.back();
			}
			do_insert_data(data,box,n->children_[i]);
			return;
		    }
		}
		n->cont_.push_back(data);
	    }
	}
	
	void split_box(Envelope<double> const& node_extent,Envelope<double> * ext)
	{
	    coord2d c=node_extent.center();

	    double width=node_extent.width();
	    double height=node_extent.height();
	    
	    double lox=node_extent.minx();
	    double loy=node_extent.miny();
	    double hix=node_extent.maxx();
	    double hiy=node_extent.maxy();
	    
	    ext[0]=Envelope<double>(lox,loy,lox + width * ratio_,loy + height * ratio_);
	    ext[1]=Envelope<double>(hix - width * ratio_,loy,hix,loy + height * ratio_);
	    ext[2]=Envelope<double>(lox,hiy - height*ratio_,lox + width * ratio_,hiy);
	    ext[3]=Envelope<double>(hix - width * ratio_,hiy - height*ratio_,hix,hiy);
	}
    };    
} 

#endif
