/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#ifndef QUADTREE_HPP
#define QUADTREE_HPP
// stl
#include <cstring>
#include <vector>
#include <fstream>
#include <iostream>
#include <memory>
// mapnik
#include <mapnik/box2d.hpp>

using mapnik::box2d;
using mapnik::coord2d;

template <typename T>
struct quadtree_node
{
    using value_type = T;
    box2d<double> ext_;
    std::vector<value_type> data_;
    quadtree_node<value_type>* children_[4];
    quadtree_node(box2d<double> const& ext)
        : ext_(ext),data_()
    {
        std::memset(children_, 0, sizeof(quadtree_node<value_type>*)*4);
    }

    ~quadtree_node()
    {
        for (int i=0;i<4;++i)
        {
            if (children_[i])
            {
                delete children_[i],children_[i]=0;
            }
        }
    }

    int num_subnodes() const
    {
        int count=0;
        for (int i=0;i<4;++i)
        {
            if (children_[i])
            {
                ++count;
            }
        }
        return count;
    }
};

template <typename T>
class quadtree
{
    using value_type = T;
private:
    quadtree_node<value_type>* root_;
    const int maxdepth_;
    const double ratio_;
public:

    quadtree(box2d<double> const& extent, int maxdepth, double ratio)
        : root_(new quadtree_node<value_type>(extent)),
          maxdepth_(maxdepth),
          ratio_(ratio) {}

    ~quadtree()
    {
        if (root_) delete root_;
    }

    void insert(value_type const& data,box2d<double> const& item_ext)
    {
        insert(data,item_ext,root_,maxdepth_);
    }

    int count() const
    {
        return count_nodes(root_);
    }

    int count_items() const
    {
        int count=0;
        count_items(root_,count);
        return count;
    }

    void print() const
    {
        print(root_);
    }

    void trim()
    {
        trim_tree(root_);
    }

    void write(std::ostream& out)
    {
        char header[16];
        std::memset(header,0,16);
        header[0]='m';
        header[1]='a';
        header[2]='p';
        header[3]='n';
        header[4]='i';
        header[5]='k';
        out.write(header,16);
        write_node(out,root_);
    }

private:

    void trim_tree(quadtree_node<value_type>*&  node)
    {
        if (node)
        {
            for (int i=0;i<4;++i)
            {
                trim_tree(node->children_[i]);
            }

            if (node->num_subnodes()==1 && node->data_.size()==0)
            {
                for (int i=0;i<4;++i)
                {
                    if (node->children_[i])
                    {
                        node=node->children_[i];
                        break;
                    }
                }
            }
        }
    }

    int count_nodes(quadtree_node<value_type> const*  node) const
    {
        if (!node)
        {
            return 0;
        }
        else
        {
            int count = 1;
            for (int i=0;i<4;++i)
            {
                count += count_nodes(node->children_[i]);
            }
            return count;
        }
    }

    void count_items(quadtree_node<value_type> const* node,int& count) const
    {
        if (node)
        {
            count += node->data_.size();
            for (int i=0;i<4;++i)
            {
                count_items(node->children_[i],count);
            }
        }
    }

    int subnode_offset(quadtree_node<value_type> const* node) const
    {
        int offset=0;
        for (int i = 0; i < 4; i++)
        {
            if (node->children_[i])
            {
                offset +=sizeof(box2d<double>)+(node->children_[i]->data_.size()*sizeof(value_type))+3*sizeof(int);
                offset +=subnode_offset(node->children_[i]);
            }
        }
        return offset;
    }

    void write_node(std::ostream& out, quadtree_node<value_type> const* node) const
    {
        if (node)
        {
            int offset=subnode_offset(node);
            int shape_count=node->data_.size();
            int recsize=sizeof(box2d<double>) + 3 * sizeof(int) + shape_count * sizeof(value_type);
            std::unique_ptr<char[]> node_record(new char[recsize]);
            std::memset(node_record.get(), 0, recsize);
            std::memcpy(node_record.get(), &offset, 4);
            std::memcpy(node_record.get() + 4, &node->ext_, sizeof(box2d<double>));
            std::memcpy(node_record.get() + 36, &shape_count, 4);
            for (int i=0; i < shape_count; ++i)
            {
                memcpy(node_record.get() + 40 + i * sizeof(value_type), &(node->data_[i]),sizeof(value_type));
            }
            int num_subnodes=0;
            for (int i = 0; i < 4; ++i)
            {
                if (node->children_[i])
                {
                    ++num_subnodes;
                }
            }
            std::memcpy(node_record.get() + 40 + shape_count * sizeof(value_type),&num_subnodes,4);
            out.write(node_record.get(),recsize);
            for (int i = 0;i<4;++i)
            {
                write_node(out,node->children_[i]);
            }
        }
    }

    void print(quadtree_node<value_type> const* node,int level=0) const
    {
        if (node)
        {
            typename std::vector<value_type>::const_iterator itr=node->data_.begin();
            std::string pad;
            for (int i = 0; i < level; ++i)
            {
                pad+=" ";
            }
            std::clog<<pad<<"node "<<node<<" extent:"<<node->ext_<<std::endl;
            std::clog<<pad;
            while(itr!=node->data_.end())
            {
                std::clog<<*itr<<" ";
                ++itr;
            }
            std::clog<<std::endl;
            for (int i = 0; i < 4; ++i)
            {
                print(node->children_[i],level+4);
            }
        }
    }

    void insert(value_type const& data,const box2d<double>& item_ext, quadtree_node<value_type> *  node,int maxdepth)
    {
        if (node && node->ext_.contains(item_ext))
        {
            double width=node->ext_.width();
            double height=node->ext_.height();

            double lox=node->ext_.minx();
            double loy=node->ext_.miny();
            double hix=node->ext_.maxx();
            double hiy=node->ext_.maxy();

            box2d<double> ext[4];
            ext[0]=box2d<double>(lox,loy,lox + width * ratio_,loy + height * ratio_);
            ext[1]=box2d<double>(hix - width * ratio_,loy,hix,loy + height * ratio_);
            ext[2]=box2d<double>(lox,hiy - height*ratio_,lox + width * ratio_,hiy);
            ext[3]=box2d<double>(hix - width * ratio_,hiy - height*ratio_,hix,hiy);

            if (maxdepth > 1)
            {
                for (int i = 0; i < 4; ++i)
                {
                    if (ext[i].contains(item_ext))
                    {
                        if (!node->children_[i])
                        {
                            node->children_[i]=new quadtree_node<value_type>(ext[i]);
                        }
                        insert(data,item_ext,node->children_[i],maxdepth-1);
                        return;
                    }
                }
            }
            node->data_.push_back(data);
        }
    }
};
#endif //QUADTREE_HPP
