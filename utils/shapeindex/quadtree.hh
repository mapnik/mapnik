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

#ifndef QUADTREE_HH
#define QUADTREE_HH

#include "mapnik.hh"
#include <vector>
#include <fstream>

using namespace mapnik;

template <typename T>
struct quadtree_node
{
    std::vector<T> data_;
    Envelope<double> ext_;
    quadtree_node<T>* children_[4];
    quadtree_node(const Envelope<double>& ext)
        :ext_(ext)
    {
        memset(children_,0,sizeof(quadtree_node<T>*)*4);
    }
    ~quadtree_node()
    {
    }
};

template <typename T>
class quadtree
{
    private:
        quadtree_node<T>* root_;
        const int maxdepth_;
        const double ratio_;
    public:
        quadtree(const Envelope<double>& extent,int maxdepth,double ratio)
            : root_(new quadtree_node<T>(extent)),
            maxdepth_(maxdepth),
            ratio_(ratio){}
        ~quadtree()
        {
            destroy_node(root_);
        }
        void insert(const T& data,const Envelope<double>& item_ext)
        {
            insert(data,item_ext,root_,maxdepth_);
        }
        int count() const
        {
            return count_nodes(root_);
        }

        void print() const
        {
            print(root_);
        }

        void write(std::ostream& out)
        {
            char header[16];
            memset(header,0,16);
            header[0]='m';
            header[1]='a';
            header[2]='p';
            header[4]='n';
	    header[5]='i';
	    header[6]='k';
            out.write(header,16);
            //trim_tree(root_);//TODO trim empty nodes
            write_node(out,root_);
        }

    private:

        void destroy_node(quadtree_node<T>* node)
        {
            if (node)
            {
                for (int i=0;i<4;++i)
                {
                    destroy_node(node->children_[i]);
                }
                delete node,node=0;
            }
        }

        bool trim_tree(quadtree_node<T>* node)
        {

            for (int i=0;i<4;++i)
            {
                if (node->children_[i] && trim_tree(node->children_[i]))
                {
                    std::cout << "destroy"<<std::endl;
                    destroy_node(node->children_[i]);
                }
            }

            int num_shapes=node->data_.size();
            int num_subnodes=0;
            if (num_shapes==0)
            {
                quadtree_node<T>* subnode=0;
                for (int i=0;i<4;++i)
                {
                    if (node->children_[i])
                    {
                        subnode=node->children_[i];
                        ++num_subnodes;
                    }
                }
                if (num_subnodes==1)
                {
                    node=subnode;                 // memory leak!
                }
            }
            return (num_shapes == 0 && num_subnodes == 0);
        }

        int count_nodes(const quadtree_node<T>* node) const
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

        int subnode_offset(const quadtree_node<T>* node) const
        {
            int offset=0;
            for (int i=0;i<4;i++)
            {
                if (node->children_[i])
                {
                    offset +=sizeof(Envelope<double>)+(node->children_[i]->data_.size()*sizeof(T))+3*sizeof(int);
                    offset +=subnode_offset(node->children_[i]);
                }
            }
            return offset;
        }

        void write_node(std::ostream& out,const quadtree_node<T>* node) const
        {
            if (node)
            {
                int offset=subnode_offset(node);
                int shape_count=node->data_.size();
                int recsize=sizeof(Envelope<double>) + 3 * sizeof(int) + shape_count * sizeof(T);
                char* node_record=new char[recsize];
                memset(node_record,0,recsize);
                memcpy(node_record,&offset,4);
                memcpy(node_record+4,&node->ext_,sizeof(Envelope<double>));
                memcpy(node_record+36,&shape_count,4);
                for (int i=0;i<shape_count;++i)
                {
                    memcpy(node_record + 40 + i * sizeof(T),&(node->data_[i]),sizeof(T));
                }
                int num_subnodes=0;
                for (int i=0;i<4;++i)
                {
                    if (node->children_[i])
                        ++num_subnodes;
                }
                memcpy(node_record + 40 + shape_count * sizeof(T),&num_subnodes,4);
                out.write(node_record,recsize);
                delete [] node_record;

                for (int i=0;i<4;++i)
                {
                    write_node(out,node->children_[i]);
                }
            }
        }

        void print(const quadtree_node<T>* node) const
        {
            if (node)
            {
                typename std::vector<T>::const_iterator itr=node->data_.begin();
                std::cout<<"node extent:"<<node->ext_<<std::endl;
                while(itr!=node->data_.end())
                {
                    std::cout<<*itr<<" ";
                    ++itr;
                }
                std::cout<<std::endl;
                for (int i=0;i<4;++i)
                {
                    print(node->children_[i]);
                }
            }
        }

        void insert(const T& data,const Envelope<double>& item_ext,quadtree_node<T>* node,int maxdepth)
        {
            if (node && node->ext_.contains(item_ext))
            {
                coord2d c=node->ext_.center();

                double width=node->ext_.width();
                double height=node->ext_.height();

                double lox=node->ext_.minx();
                double loy=node->ext_.miny();
                double hix=node->ext_.maxx();
                double hiy=node->ext_.maxy();

                Envelope<double> ext[4];
                ext[0]=Envelope<double>(lox,loy,lox + width * ratio_,loy + height * ratio_);
                ext[1]=Envelope<double>(hix - width * ratio_,loy,hix,loy + height * ratio_);
                ext[2]=Envelope<double>(lox,hiy - height*ratio_,lox + width * ratio_,hiy);
                ext[3]=Envelope<double>(hix - width * ratio_,hiy - height*ratio_,hix,hiy);

                if (maxdepth > 1)
                {
                    for (int i=0;i<4;++i)
                    {
                        if (ext[i].contains(item_ext))
                        {
                            if (!node->children_[i])
                            {
                                node->children_[i]=new quadtree_node<T>(ext[i]);
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
#endif                                            //QUADTREE_HH
