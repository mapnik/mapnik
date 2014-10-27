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

#include <mapnik/config.hpp>

// boost
#include "boost_std_shared_shim.hpp"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/noncopyable.hpp>
#pragma GCC diagnostic pop

#include <mapnik/text/text_properties.hpp>
#include <mapnik/text/placements/simple.hpp>
#include <mapnik/text/placements/list.hpp>
#include <mapnik/text/formatting/text.hpp>
#include <mapnik/text/formatting/list.hpp>
#include <mapnik/text/formatting/format.hpp>
#include <mapnik/text/formatting/layout.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/symbolizer.hpp>

#include "mapnik_enumeration.hpp"
#include "mapnik_threads.hpp"

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

// This class works around a feature in boost python.
// See http://osdir.com/ml/python.c++/2003-11/msg00158.html

template <typename T,
          typename X1 = boost::python::detail::not_specified,
          typename X2 = boost::python::detail::not_specified,
          typename X3 = boost::python::detail::not_specified>
class class_with_converter : public boost::python::class_<T, X1, X2, X3>
{
public:
    using self = class_with_converter<T,X1,X2,X3>;
    // Construct with the class name, with or without docstring, and default __init__() function
    class_with_converter(char const* name, char const* doc = 0) : boost::python::class_<T, X1, X2, X3>(name, doc)  { }

    // Construct with class name, no docstring, and an uncallable __init__ function
    class_with_converter(char const* name, boost::python::no_init_t y) : boost::python::class_<T, X1, X2, X3>(name, y) { }

    // Construct with class name, docstring, and an uncallable __init__ function
    class_with_converter(char const* name, char const* doc, boost::python::no_init_t y) : boost::python::class_<T, X1, X2, X3>(name, doc, y) { }

    // Construct with class name and init<> function
    template <class DerivedT> class_with_converter(char const* name, boost::python::init_base<DerivedT> const& i)
        : boost::python::class_<T, X1, X2, X3>(name, i) { }

    // Construct with class name, docstring and init<> function
    template <class DerivedT>
    inline class_with_converter(char const* name, char const* doc, boost::python::init_base<DerivedT> const& i)
        : boost::python::class_<T, X1, X2, X3>(name, doc, i) { }

    template <class D>
    self& def_readwrite_convert(char const* name, D const& d, char const* /*doc*/=0)
    {
        this->add_property(name,
                           boost::python::make_getter(d, boost::python::return_value_policy<boost::python::return_by_value>()),
                           boost::python::make_setter(d, boost::python::default_call_policies()));
        return *this;
    }
};

/*
boost::python::tuple get_displacement(text_layout_properties const& t)
{
    return boost::python::make_tuple(0.0,0.0);// FIXME t.displacement.x, t.displacement.y);
}

void set_displacement(text_layout_properties &t, boost::python::tuple arg)
{
    if (len(arg) != 2)
    {
        PyErr_SetObject(PyExc_ValueError,
                        ("expected 2-item tuple in call to set_displacement; got %s"
                         % arg).ptr()
            );
        throw_error_already_set();
    }

    //double x = extract<double>(arg[0]);
    //double y = extract<double>(arg[1]);
    //t.displacement.set(x, y); FIXME
}


struct NodeWrap
    : formatting::node, wrapper<formatting::node>
{
    NodeWrap()
        : formatting::node(), wrapper<formatting::node>() {}

    void apply(evaluated_format_properties_ptr p, feature_impl const& feature, attributes const& vars, text_layout &output) const
    {
        python_block_auto_unblock b;
        this->get_override("apply")(ptr(&p), ptr(&feature), ptr(&vars), ptr(&output));
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
*/
/*
struct TextNodeWrap
    : formatting::text_node, wrapper<formatting::text_node>
{
    TextNodeWrap(expression_ptr expr)
        : formatting::text_node(expr), wrapper<formatting::text_node>() {}

    TextNodeWrap(std::string expr_text)
        : formatting::text_node(expr_text), wrapper<formatting::text_node>() {}

    virtual void apply(evaluated_format_properties_ptr p, feature_impl const& feature, attributes const& vars, text_layout &output) const
    {
        if(override o = this->get_override("apply"))
        {
            python_block_auto_unblock b;
            o(ptr(&p), ptr(&feature), ptr(&vars), ptr(&output));
        }
        else
        {
            formatting::text_node::apply(p, feature, vars, output);
        }
    }

    void default_apply(evaluated_format_properties_ptr p, feature_impl const& feature, attributes const& vars, text_layout &output) const
    {
        formatting::text_node::apply(p, feature, vars, output);
    }
};
*/
/*
struct FormatNodeWrap
    : formatting::format_node, wrapper<formatting::format_node>
{
    virtual void apply(evaluated_format_properties_ptr p, feature_impl const& feature, attributes const& vars, text_layout &output) const
    {
        if(override o = this->get_override("apply"))
        {
            python_block_auto_unblock b;
            o(ptr(&p), ptr(&feature), ptr(&vars), ptr(&output));
        }
        else
        {
            formatting::format_node::apply(p, feature, vars ,output);
        }
    }

    void default_apply(evaluated_format_properties_ptr p, feature_impl const& feature, attributes const& vars, text_layout &output) const
    {
        formatting::format_node::apply(p, feature, vars, output);
    }
};

struct ExprFormatWrap: formatting::expression_format, wrapper<formatting::expression_format>
{
    virtual void apply(evaluated_format_properties_ptr p, feature_impl const& feature, attributes const& vars, text_layout &output) const
    {
        if(override o = this->get_override("apply"))
        {
            python_block_auto_unblock b;
            o(ptr(&p), ptr(&feature), ptr(&vars), ptr(&output));
        }
        else
        {
            formatting::expression_format::apply(p, feature, vars, output);
        }
    }

    void default_apply(evaluated_format_properties_ptr p, feature_impl const& feature, attributes const& vars, text_layout &output) const
    {
        formatting::expression_format::apply(p, feature, vars, output);
    }
};

struct LayoutNodeWrap: formatting::layout_node, wrapper<formatting::layout_node>
{
    virtual void apply(evaluated_format_properties_ptr p, feature_impl const& feature, attributes const& vars, text_layout &output) const
    {
        if(override o = this->get_override("apply"))
        {
            python_block_auto_unblock b;
            o(ptr(&p), ptr(&feature), ptr(&vars), ptr(&output));
        }
        else
        {
            formatting::layout_node::apply(p, feature, vars, output);
        }
    }

    void default_apply(evaluated_format_properties_ptr p, feature_impl const& feature, attributes const& vars, text_layout &output) const
    {
        formatting::layout_node::apply(p, feature, vars, output);
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
        while (begin != end)
        {
           children_.push_back(*begin);
           ++begin;
        }
    }

    // TODO: Add constructor taking variable number of arguments.
       http://wiki.python.org/moin/boost.python/HowTo#A.22Raw.22_function

    virtual void apply(evaluated_format_properties_ptr p, feature_impl const& feature, attributes const& vars, text_layout &output) const
    {
        if(override o = this->get_override("apply"))
        {
            python_block_auto_unblock b;
            o(ptr(&p), ptr(&feature), ptr(&vars), ptr(&output));
        }
        else
        {
            formatting::list_node::apply(p, feature, vars, output);
        }
    }

    void default_apply(evaluated_format_properties_ptr p, feature_impl const& feature, attributes const& vars, text_layout &output) const
    {
        formatting::list_node::apply(p, feature, vars, output);
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
*/
/*
struct TextPlacementsWrap: text_placements, wrapper<text_placements>
{
    text_placement_info_ptr get_placement_info(double scale_factor_) const
    {
        python_block_auto_unblock b;
        //return this->get_override("get_placement_info")();
        return text_placement_info_ptr();
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


evaluated_format_properties_ptr get_format(text_symbolizer const& sym)
{
    return sym.get_placement_options()->defaults.format;
}

void set_format(text_symbolizer const& sym, evaluated_format_properties_ptr format)
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
*/
}

void export_text_placement()
{
    /*
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

    enumeration_<halo_rasterizer_e>("halo_rasterizer")
        .value("FULL",HALO_RASTERIZER_FULL)
        .value("FAST",HALO_RASTERIZER_FAST)
        ;
    */
    class_<text_symbolizer>("TextSymbolizer",
                            init<>())
        ;
    /*

    class_with_converter<text_symbolizer_properties>
        ("TextSymbolizerProperties")
        .def_readwrite_convert("label_placement", &text_symbolizer_properties::label_placement)
        .def_readwrite_convert("upright", &text_symbolizer_properties::upright)
        .def_readwrite("label_spacing", &text_symbolizer_properties::label_spacing)
        .def_readwrite("label_position_tolerance", &text_symbolizer_properties::label_position_tolerance)
        .def_readwrite("avoid_edges", &text_symbolizer_properties::avoid_edges)
        .def_readwrite("margin", &text_symbolizer_properties::margin)
        .def_readwrite("repeat_distance", &text_symbolizer_properties::repeat_distance)
        .def_readwrite("minimum_distance", &text_symbolizer_properties::minimum_distance)
        .def_readwrite("minimum_padding", &text_symbolizer_properties::minimum_padding)
        .def_readwrite("minimum_path_length", &text_symbolizer_properties::minimum_path_length)
        .def_readwrite("maximum_angle_char_delta", &text_symbolizer_properties::max_char_angle_delta)
        .def_readwrite("allow_overlap", &text_symbolizer_properties::allow_overlap)
        .def_readwrite("largest_bbox_only", &text_symbolizer_properties::largest_bbox_only)
        .def_readwrite("layout_defaults", &text_symbolizer_properties::layout_defaults)
        //.def_readwrite("format", &text_symbolizer_properties::format)
        .add_property ("format_tree",
                       &text_symbolizer_properties::format_tree,
                       &text_symbolizer_properties::set_format_tree);
    //from_xml, to_xml operate on mapnik's internal XML tree and don't make sense in python.
     //  add_expressions isn't useful in python either. The result is only needed by
     //  attribute_collector (which isn't exposed in python) and
     //  it just calls add_expressions of the associated formatting tree.
     //  set_old_style expression is just a compatibility wrapper and doesn't need to be exposed in python.
    ;

    class_with_converter<text_layout_properties>
        ("TextLayoutProperties")
        .def_readwrite_convert("horizontal_alignment", &text_layout_properties::halign)
        .def_readwrite_convert("justify_alignment", &text_layout_properties::jalign)
        .def_readwrite_convert("vertical_alignment", &text_layout_properties::valign)
        .def_readwrite("text_ratio", &text_layout_properties::text_ratio)
        .def_readwrite("wrap_width", &text_layout_properties::wrap_width)
        .def_readwrite("wrap_before", &text_layout_properties::wrap_before)
        .def_readwrite("orientation", &text_layout_properties::orientation)
        .def_readwrite("rotate_displacement", &text_layout_properties::rotate_displacement)
        .add_property("displacement", &get_displacement, &set_displacement);

    class_with_converter<detail::evaluated_format_properties>
        ("CharProperties")
        .def_readwrite_convert("text_transform", &detail::evaluated_format_properties::text_transform)
        .def_readwrite_convert("fontset", &detail::evaluated_format_properties::fontset)
        .def(init<detail::evaluated_format_properties const&>()) //Copy constructor
        .def_readwrite("face_name", &detail::evaluated_format_properties::face_name)
        .def_readwrite("text_size", &detail::evaluated_format_properties::text_size)
        .def_readwrite("character_spacing", &detail::evaluated_format_properties::character_spacing)
        .def_readwrite("line_spacing", &detail::evaluated_format_properties::line_spacing)
        .def_readwrite("text_opacity", &detail::evaluated_format_properties::text_opacity)
        .def_readwrite("fill", &detail::evaluated_format_properties::fill)
        .def_readwrite("halo_fill", &detail::evaluated_format_properties::halo_fill)
        .def_readwrite("halo_radius", &evaluated_format_properties::halo_radius)
        //from_xml, to_xml operate on mapnik's internal XML tree and don't make sense in python.
        ;
    class_<TextPlacementsWrap,
        std::shared_ptr<TextPlacementsWrap>,
        boost::noncopyable>
        ("TextPlacements")
        .def_readwrite("defaults", &text_placements::defaults)
        //.def("get_placement_info", pure_virtual(&text_placements::get_placement_info))
        // TODO: add_expressions()
        ;
    register_ptr_to_python<std::shared_ptr<text_placements> >();

    class_<TextPlacementInfoWrap,
        std::shared_ptr<TextPlacementInfoWrap>,
        boost::noncopyable>
        ("TextPlacementInfo",
         init<text_placements const*, double>())
        .def("next", pure_virtual(&text_placement_info::next))
        .def_readwrite("properties", &text_placement_info::properties)
        .def_readwrite("scale_factor", &text_placement_info::scale_factor)
        ;
    register_ptr_to_python<std::shared_ptr<text_placement_info> >();


    class_<expression_set,std::shared_ptr<expression_set>,
           boost::noncopyable>("ExpressionSet")
        .def("insert", &insert_expression);
    ;

    class_<formatting::node,std::shared_ptr<formatting::node>,
           boost::noncopyable>("FormattingNode")
        .def("apply", pure_virtual(&formatting::node::apply))
        .def("add_expressions", pure_virtual(&formatting::node::add_expressions))
        .def("to_xml", pure_virtual(&formatting::node::to_xml))
        ;

    register_ptr_to_python<std::shared_ptr<formatting::node> >();

    class_<formatting::text_node,
           std::shared_ptr<formatting::text_node>,
           bases<formatting::node>,boost::noncopyable>("FormattingText", init<expression_ptr>())
        .def(init<std::string>())
        .def("apply", &formatting::text_node::apply)//, &TextNodeWrap::default_apply)
        .add_property("text",&formatting::text_node::get_text, &formatting::text_node::set_text)
        ;

    register_ptr_to_python<std::shared_ptr<formatting::text_node> >();

    class_with_converter<FormatNodeWrap,
        std::shared_ptr<FormatNodeWrap>,
        bases<formatting::node>,
        boost::noncopyable>
        ("FormattingFormat")
        .def_readwrite_convert("text_size", &formatting::format_node::text_size)
        .def_readwrite_convert("face_name", &formatting::format_node::face_name)
        .def_readwrite_convert("character_spacing", &formatting::format_node::character_spacing)
        .def_readwrite_convert("line_spacing", &formatting::format_node::line_spacing)
        .def_readwrite_convert("text_opacity", &formatting::format_node::text_opacity)
        .def_readwrite_convert("text_transform", &formatting::format_node::text_transform)
        .def_readwrite_convert("fill", &formatting::format_node::fill)
        .def_readwrite_convert("halo_fill", &formatting::format_node::halo_fill)
        .def_readwrite_convert("halo_radius", &formatting::format_node::halo_radius)
        .def("apply", &formatting::format_node::apply, &FormatNodeWrap::default_apply)
        .add_property("child",
                      &formatting::format_node::get_child,
                      &formatting::format_node::set_child)
        ;
    register_ptr_to_python<std::shared_ptr<formatting::format_node> >();

    class_<ListNodeWrap,
        std::shared_ptr<ListNodeWrap>,
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

    register_ptr_to_python<std::shared_ptr<formatting::list_node> >();

    class_<ExprFormatWrap,
        std::shared_ptr<ExprFormatWrap>,
        bases<formatting::node>,
        boost::noncopyable>
        ("FormattingExpressionFormat")
        .def_readwrite("text_size", &formatting::expression_format::text_size)
        .def_readwrite("face_name", &formatting::expression_format::face_name)
        .def_readwrite("character_spacing", &formatting::expression_format::character_spacing)
        .def_readwrite("line_spacing", &formatting::expression_format::line_spacing)
        .def_readwrite("text_opacity", &formatting::expression_format::text_opacity)
        .def_readwrite("fill", &formatting::expression_format::fill)
        .def_readwrite("halo_fill", &formatting::expression_format::halo_fill)
        .def_readwrite("halo_radius", &formatting::expression_format::halo_radius)
        .def("apply", &formatting::expression_format::apply, &ExprFormatWrap::default_apply)
        .add_property("child",
                      &formatting::expression_format::get_child,
                      &formatting::expression_format::set_child)
        ;
    register_ptr_to_python<std::shared_ptr<formatting::expression_format> >();
*/
    //TODO: registry
}
