/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko, Jean-Francois Doyon
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
#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>

#include <mapnik/text_properties.hpp>
#include <mapnik/text_placements/simple.hpp>
#include <mapnik/text_placements/list.hpp>
#include <mapnik/formatting/text.hpp>
#include <mapnik/formatting/list.hpp>
#include <mapnik/formatting/format.hpp>
#include <mapnik/formatting/expression_format.hpp>
#include <mapnik/processed_text.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/text_symbolizer.hpp>

#include "mapnik_enumeration.hpp"
#include "mapnik_threads.hpp"
#include "python_optional.hpp"

using namespace mapnik;

/* Notes:
   Overriding functions in inherited classes:
   boost.python documentation doesn't really tell you how to do it.
   But this helps:
   http://www.gamedev.net/topic/446225-inheritance-in-boostpython/

   register_ptr_to_python is required for wrapped classes, but not for unwrapped.

   Functions don't have to be members of the class, but can also be
   normal functions taking a ref to the class as first parameter.
*/

namespace {
using namespace boost::python;

boost::python::tuple get_displacement(text_symbolizer_properties const& t)
{
    return boost::python::make_tuple(t.displacement.first, t.displacement.second);
}

void set_displacement(text_symbolizer_properties &t, boost::python::tuple arg)
{
    if (len(arg) != 2)
    {
        PyErr_SetObject(PyExc_ValueError,
                        ("expected 2-item tuple in call to set_displacement; got %s"
                         % arg).ptr()
            );
        throw_error_already_set();
    }

    double x = extract<double>(arg[0]);
    double y = extract<double>(arg[1]);
    t.displacement = std::make_pair(x, y);
}


struct NodeWrap: formatting::node, wrapper<formatting::node>
{
    NodeWrap() : formatting::node(), wrapper<formatting::node>()
    {

    }

    void apply(char_properties const& p, Feature const& feature, processed_text &output) const
    {
        python_block_auto_unblock b;
        this->get_override("apply")(ptr(&p), ptr(&feature), ptr(&output));
    }

    virtual void add_expressions(expression_set &output) const
    {
        override o = this->get_override("add_expressions");
        if (o)
        {
            python_block_auto_unblock b;
            o(ptr(&output));
        } else
        {
            formatting::node::add_expressions(output);
        }
    }

    void default_add_expressions(expression_set &output) const
    {
        formatting::node::add_expressions(output);
    }
};


struct TextNodeWrap: formatting::text_node, wrapper<formatting::text_node>
{
    TextNodeWrap(expression_ptr expr) : formatting::text_node(expr), wrapper<formatting::text_node>()
    {

    }

    TextNodeWrap(std::string expr_text) : formatting::text_node(expr_text), wrapper<formatting::text_node>()
    {

    }

    virtual void apply(char_properties const& p, Feature const& feature, processed_text &output) const
    {
        if(override o = this->get_override("apply"))
        {
            python_block_auto_unblock b;
            o(ptr(&p), ptr(&feature), ptr(&output));
        }
        else
        {
            formatting::text_node::apply(p, feature, output);
        }
    }

    void default_apply(char_properties const& p, Feature const& feature, processed_text &output) const
    {
        formatting::text_node::apply(p, feature, output);
    }
};

struct FormatNodeWrap: formatting::format_node, wrapper<formatting::format_node>
{
    virtual void apply(char_properties const& p, Feature const& feature, processed_text &output) const
    {
        if(override o = this->get_override("apply"))
        {
            python_block_auto_unblock b;
            o(ptr(&p), ptr(&feature), ptr(&output));
        }
        else
        {
            formatting::format_node::apply(p, feature, output);
        }
    }

    void default_apply(char_properties const& p, Feature const& feature, processed_text &output) const
    {
        formatting::format_node::apply(p, feature, output);
    }
};

struct ExprFormatWrap: formatting::expression_format, wrapper<formatting::expression_format>
{
    virtual void apply(char_properties const& p, Feature const& feature, processed_text &output) const
    {
        if(override o = this->get_override("apply"))
        {
            python_block_auto_unblock b;
            o(ptr(&p), ptr(&feature), ptr(&output));
        }
        else
        {
            formatting::expression_format::apply(p, feature, output);
        }
    }

    void default_apply(char_properties const& p, Feature const& feature, processed_text &output) const
    {
        formatting::expression_format::apply(p, feature, output);
    }
};

struct ListNodeWrap: formatting::list_node, wrapper<formatting::list_node>
{
    //Default constructor
    ListNodeWrap() : formatting::list_node(), wrapper<formatting::list_node>()
    {
    }

    //Special constructor: Takes a python sequence as its argument
    ListNodeWrap(object l) : formatting::list_node(), wrapper<formatting::list_node>()
    {
        stl_input_iterator<formatting::node_ptr> begin(l), end;
        children_.insert(children_.end(), begin, end);
    }

    /* TODO: Add constructor taking variable number of arguments.
       http://wiki.python.org/moin/boost.python/HowTo#A.22Raw.22_function */


    virtual void apply(char_properties const& p, Feature const& feature, processed_text &output) const
    {
        if(override o = this->get_override("apply"))
        {
            python_block_auto_unblock b;
            o(ptr(&p), ptr(&feature), ptr(&output));
        }
        else
        {
            formatting::list_node::apply(p, feature, output);
        }
    }

    void default_apply(char_properties const& p, Feature const& feature, processed_text &output) const
    {
        formatting::list_node::apply(p, feature, output);
    }

    inline void IndexError(){
        PyErr_SetString(PyExc_IndexError, "Index out of range");
        throw_error_already_set();
    }

    unsigned get_length()
    {
        return children_.size();
    }

    formatting::node_ptr get_item(int i)
    {
        if (i < 0) i+= children_.size();
        if (i < static_cast<int>(children_.size())) return children_[i];
        IndexError();
        return formatting::node_ptr(); //Avoid compiler warning
    }

    void set_item(int i, formatting::node_ptr ptr)
    {
        if (i < 0) i+= children_.size();
        if (i < static_cast<int>(children_.size())) children_[i] = ptr;
        IndexError();
    }

    void append(formatting::node_ptr ptr)
    {
        children_.push_back(ptr);
    }
};

struct TextPlacementsWrap: text_placements, wrapper<text_placements>
{
    text_placement_info_ptr get_placement_info(double scale_factor_) const
    {
        python_block_auto_unblock b;
        return this->get_override("get_placement_info")();
    }
};

struct TextPlacementInfoWrap: text_placement_info, wrapper<text_placement_info>
{
    TextPlacementInfoWrap(text_placements const* parent,
                          double scale_factor_)
        : text_placement_info(parent, scale_factor_)
    {

    }

    bool next()
    {
        python_block_auto_unblock b;
        return this->get_override("next")();
    }
};

void insert_expression(expression_set *set, expression_ptr p)
{
    set->insert(p);
}

char_properties & get_format(text_symbolizer const& sym)
{
    return sym.get_placement_options()->defaults.format;
}

void set_format(text_symbolizer const& sym, char_properties & format)
{
    sym.get_placement_options()->defaults.format = format;
}

text_symbolizer_properties & get_properties(text_symbolizer const& sym)
{
    return sym.get_placement_options()->defaults;
}

void set_properties(text_symbolizer const& sym, text_symbolizer_properties & defaults)
{
    sym.get_placement_options()->defaults = defaults;
}

}

void export_text_placement()
{
    using namespace boost::python;

    enumeration_<label_placement_e>("label_placement")
        .value("LINE_PLACEMENT",LINE_PLACEMENT)
        .value("POINT_PLACEMENT",POINT_PLACEMENT)
        .value("VERTEX_PLACEMENT",VERTEX_PLACEMENT)
        .value("INTERIOR_PLACEMENT",INTERIOR_PLACEMENT)
        ;
    enumeration_<vertical_alignment_e>("vertical_alignment")
        .value("TOP",V_TOP)
        .value("MIDDLE",V_MIDDLE)
        .value("BOTTOM",V_BOTTOM)
        .value("AUTO",V_AUTO)
        ;

    enumeration_<horizontal_alignment_e>("horizontal_alignment")
        .value("LEFT",H_LEFT)
        .value("MIDDLE",H_MIDDLE)
        .value("RIGHT",H_RIGHT)
        .value("AUTO",H_AUTO)
        ;

    enumeration_<justify_alignment_e>("justify_alignment")
        .value("LEFT",J_LEFT)
        .value("MIDDLE",J_MIDDLE)
        .value("RIGHT",J_RIGHT)
        .value("AUTO", J_AUTO)
        ;

    enumeration_<text_transform_e>("text_transform")
        .value("NONE",NONE)
        .value("UPPERCASE",UPPERCASE)
        .value("LOWERCASE",LOWERCASE)
        .value("CAPITALIZE",CAPITALIZE)
        ;

    class_<text_symbolizer>("TextSymbolizer",
                            init<>())
        .def(init<expression_ptr, std::string const&, unsigned, color const&>())
        .add_property("placements",
                      &text_symbolizer::get_placement_options,
                      &text_symbolizer::set_placement_options)
        //TODO: Check return policy, is there a better way to do this?
        .add_property("format",
                      make_function(&get_format, return_value_policy<reference_existing_object>()),
                      &set_format,
                      "Shortcut for placements.defaults.default_format")
        .add_property("properties",
                      make_function(&get_properties, return_value_policy<reference_existing_object>()),
                      &set_properties,
                      "Shortcut for placements.defaults")
        .add_property("comp_op",
                      &text_symbolizer::comp_op,
                      &text_symbolizer::set_comp_op,
                      "Set/get the comp-op")
        .add_property("clip",
                      &text_symbolizer::clip,
                      &text_symbolizer::set_clip,
                      "Set/get the text geometry's clipping status")
        ;


    class_with_converter<text_symbolizer_properties>
        ("TextSymbolizerProperties")
        .def_readwrite_convert("label_placement", &text_symbolizer_properties::label_placement)
        .def_readwrite_convert("horizontal_alignment", &text_symbolizer_properties::halign)
        .def_readwrite_convert("justify_alignment", &text_symbolizer_properties::jalign)
        .def_readwrite_convert("vertical_alignment", &text_symbolizer_properties::valign)
        .def_readwrite("orientation", &text_symbolizer_properties::orientation)
        .add_property("displacement",
                      &get_displacement,
                      &set_displacement)
        .def_readwrite("label_spacing", &text_symbolizer_properties::label_spacing)
        .def_readwrite("label_position_tolerance", &text_symbolizer_properties::label_position_tolerance)
        .def_readwrite("avoid_edges", &text_symbolizer_properties::avoid_edges)
        .def_readwrite("minimum_distance", &text_symbolizer_properties::minimum_distance)
        .def_readwrite("minimum_padding", &text_symbolizer_properties::minimum_padding)
        .def_readwrite("minimum_path_length", &text_symbolizer_properties::minimum_path_length)
        .def_readwrite("maximum_angle_char_delta", &text_symbolizer_properties::max_char_angle_delta)
        .def_readwrite("force_odd_labels", &text_symbolizer_properties::force_odd_labels)
        .def_readwrite("allow_overlap", &text_symbolizer_properties::allow_overlap)
        .def_readwrite("largest_bbox_only", &text_symbolizer_properties::largest_bbox_only)
        .def_readwrite("text_ratio", &text_symbolizer_properties::text_ratio)
        .def_readwrite("wrap_width", &text_symbolizer_properties::wrap_width)
        .def_readwrite("format", &text_symbolizer_properties::format)
        .add_property ("format_tree",
                       &text_symbolizer_properties::format_tree,
                       &text_symbolizer_properties::set_format_tree);
    /* from_xml, to_xml operate on mapnik's internal XML tree and don't make sense in python.
       add_expressions isn't useful in python either. The result is only needed by
       attribute_collector (which isn't exposed in python) and
       it just calls add_expressions of the associated formatting tree.
       set_old_style expression is just a compatibility wrapper and doesn't need to be exposed in python. */
    ;


    class_with_converter<char_properties>
        ("CharProperties")
        .def_readwrite_convert("text_transform", &char_properties::text_transform)
        .def_readwrite_convert("fontset", &char_properties::fontset)
        .def(init<char_properties const&>()) //Copy constructor
        .def_readwrite("face_name", &char_properties::face_name)
        .def_readwrite("text_size", &char_properties::text_size)
        .def_readwrite("character_spacing", &char_properties::character_spacing)
        .def_readwrite("line_spacing", &char_properties::line_spacing)
        .def_readwrite("text_opacity", &char_properties::text_opacity)
        .def_readwrite("wrap_char", &char_properties::wrap_char)
        .def_readwrite("wrap_character", &char_properties::wrap_char)
        .def_readwrite("wrap_before", &char_properties::wrap_before)
        .def_readwrite("fill", &char_properties::fill)
        .def_readwrite("halo_fill", &char_properties::halo_fill)
        .def_readwrite("halo_radius", &char_properties::halo_radius)
        /* from_xml, to_xml operate on mapnik's internal XML tree and don't make sense in python.*/
        ;

    class_<TextPlacementsWrap,
        boost::shared_ptr<TextPlacementsWrap>,
        boost::noncopyable>
        ("TextPlacements")
        .def_readwrite("defaults", &text_placements::defaults)
        .def("get_placement_info", pure_virtual(&text_placements::get_placement_info))
        /* TODO: add_expressions() */
        ;
    register_ptr_to_python<boost::shared_ptr<text_placements> >();

    class_<TextPlacementInfoWrap,
        boost::shared_ptr<TextPlacementInfoWrap>,
        boost::noncopyable>
        ("TextPlacementInfo",
         init<text_placements const*, double>())
        .def("next", pure_virtual(&text_placement_info::next))
        .def("get_actual_label_spacing", &text_placement_info::get_actual_label_spacing)
        .def("get_actual_minimum_distance", &text_placement_info::get_actual_minimum_distance)
        .def("get_actual_minimum_padding", &text_placement_info::get_actual_minimum_padding)
        .def_readwrite("properties", &text_placement_info::properties)
        .def_readwrite("scale_factor", &text_placement_info::scale_factor)
        ;
    register_ptr_to_python<boost::shared_ptr<text_placement_info> >();


    class_<processed_text,
        boost::shared_ptr<processed_text>,
        boost::noncopyable>
        ("ProcessedText", no_init)
        .def("push_back", &processed_text::push_back)
        .def("clear", &processed_text::clear)
        ;


    class_<expression_set,
        boost::shared_ptr<expression_set>,
        boost::noncopyable>
        ("ExpressionSet")
        .def("insert", &insert_expression);
    ;


    //TODO: Python namespace
    class_<NodeWrap,
        boost::shared_ptr<NodeWrap>,
        boost::noncopyable>
        ("FormattingNode")
        .def("apply", pure_virtual(&formatting::node::apply))
        .def("add_expressions",
             &formatting::node::add_expressions,
             &NodeWrap::default_add_expressions)
        ;
    register_ptr_to_python<boost::shared_ptr<formatting::node> >();


    class_<TextNodeWrap,
        boost::shared_ptr<TextNodeWrap>,
        bases<formatting::node>,
        boost::noncopyable>
        ("FormattingText", init<expression_ptr>())
        .def(init<std::string>())
        .def("apply", &formatting::text_node::apply, &TextNodeWrap::default_apply)
        .add_property("text",
                      &formatting::text_node::get_text,
                      &formatting::text_node::set_text)
        ;
    register_ptr_to_python<boost::shared_ptr<formatting::text_node> >();


    class_with_converter<FormatNodeWrap,
        boost::shared_ptr<FormatNodeWrap>,
        bases<formatting::node>,
        boost::noncopyable>
        ("FormattingFormat")
        .def_readwrite_convert("text_size", &formatting::format_node::text_size)
        .def_readwrite_convert("face_name", &formatting::format_node::face_name)
        .def_readwrite_convert("character_spacing", &formatting::format_node::character_spacing)
        .def_readwrite_convert("line_spacing", &formatting::format_node::line_spacing)
        .def_readwrite_convert("text_opacity", &formatting::format_node::text_opacity)
        .def_readwrite_convert("wrap_char", &formatting::format_node::wrap_char)
        .def_readwrite_convert("wrap_character", &formatting::format_node::wrap_char)
        .def_readwrite_convert("wrap_before", &formatting::format_node::wrap_before)
        .def_readwrite_convert("text_transform", &formatting::format_node::text_transform)
        .def_readwrite_convert("fill", &formatting::format_node::fill)
        .def_readwrite_convert("halo_fill", &formatting::format_node::halo_fill)
        .def_readwrite_convert("halo_radius", &formatting::format_node::halo_radius)
        .def("apply", &formatting::format_node::apply, &FormatNodeWrap::default_apply)
        .add_property("child",
                      &formatting::format_node::get_child,
                      &formatting::format_node::set_child)
        ;
    register_ptr_to_python<boost::shared_ptr<formatting::format_node> >();

    class_<ListNodeWrap,
        boost::shared_ptr<ListNodeWrap>,
        bases<formatting::node>,
        boost::noncopyable>
        ("FormattingList", init<>())
        .def(init<list>())
        .def("append", &formatting::list_node::push_back)
        .def("apply", &formatting::list_node::apply, &ListNodeWrap::default_apply)
        .def("__len__", &ListNodeWrap::get_length)
        .def("__getitem__", &ListNodeWrap::get_item)
        .def("__setitem__", &ListNodeWrap::set_item)
        .def("append", &ListNodeWrap::append)
        ;

    register_ptr_to_python<boost::shared_ptr<formatting::list_node> >();

    class_<ExprFormatWrap,
        boost::shared_ptr<ExprFormatWrap>,
        bases<formatting::node>,
        boost::noncopyable>
        ("FormattingExpressionFormat")
        .def_readwrite("text_size", &formatting::expression_format::text_size)
        .def_readwrite("face_name", &formatting::expression_format::face_name)
        .def_readwrite("character_spacing", &formatting::expression_format::character_spacing)
        .def_readwrite("line_spacing", &formatting::expression_format::line_spacing)
        .def_readwrite("text_opacity", &formatting::expression_format::text_opacity)
        .def_readwrite("wrap_char", &formatting::expression_format::wrap_char)
        .def_readwrite("wrap_character", &formatting::expression_format::wrap_char)
        .def_readwrite("wrap_before", &formatting::expression_format::wrap_before)
        .def_readwrite("fill", &formatting::expression_format::fill)
        .def_readwrite("halo_fill", &formatting::expression_format::halo_fill)
        .def_readwrite("halo_radius", &formatting::expression_format::halo_radius)
        .def("apply", &formatting::expression_format::apply, &ExprFormatWrap::default_apply)
        .add_property("child",
                      &formatting::expression_format::get_child,
                      &formatting::expression_format::set_child)
        ;
    register_ptr_to_python<boost::shared_ptr<formatting::expression_format> >();

    //TODO: registry
}
