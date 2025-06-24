/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef SVG_PATH_ITERATOR_HPP
#define SVG_PATH_ITERATOR_HPP

// mapnik

#include <mapnik/view_transform.hpp>
#include <mapnik/transform_path_adapter.hpp>
// boost
#include <boost/iterator/iterator_adaptor.hpp>
#include <memory>

namespace mapnik {
namespace svg {

using namespace mapnik;

/*
 * @brief Iterator class used to iterate over geometry vertexes.
 * Since mapnik::geometry provides access to the components of
 * a vertex only through variables passed by reference,
 * geometry_iterator retrieves these components (command, x coord,
 * and y coord) and makes them available inside tuples.
 *
 * This iterator exposes the behavior of a forward iterator and
 * subclasses boost::iterator_adaptor, which already implements
 * several iterator operations, such as dereferencing.
 *
 * @tparam Value the type of sequence element dereferenced.
 * @tparam Container the sequence over which it iterates.
 */
template<typename Value, typename Container>
class path_iterator
    : public boost::
        iterator_adaptor<path_iterator<Value, Container>, Value*, boost::use_default, boost::forward_traversal_tag>
{
  public:
    using value_type = Value;
    using container_type = Container;
    // using value_component_type = typename Container::value_type;

    /*!
     * @brief Constructor that initializes the reference to the current element to null.
     * This constructor is suitable to mark the end of the iterator (analogous to
     * calling end_iterator() in an STL container).
     *
     * @param geometry the geometry that handles the vector of vertexes.
     */
    path_iterator(Container const& path)
        : path_iterator::iterator_adaptor_(0)
        , path_(path)
        , first_value_(std::make_shared<Value>(0, 0, 0))
    {}

    /*!
     * This constructor receives the first element of the sequence as a pointer.
     * Since the container type will likely be a mapnik::geometry, this
     * first element would need to be obtained in a similar way as the increment
     * method below. For this reason, most of the time this constructor will
     * be called with a null pointer. The body of the constructor makes a call
     * to increment() in order to obtain this first element from the container.
     *
     * @param p pointer to the first element of the sequence.
     * @param geometry the geometry that handles the vector of vertexes.
     */
    explicit path_iterator(Value* first_element, Container const& path)
        : path_iterator::iterator_adaptor_(first_element)
        , path_(path)
        , first_value_(std::make_shared<Value>(0, 0, 0))
    {
        this->increment();
    }

    struct enabler
    {};

    /*!
     * @brief Constructor that enables operation between const and non-const iterators.
     * @sa http://www.boost.org/doc/libs/1_45_0/libs/iterator/doc/iterator_facade.html#interoperability
     */
    template<typename OtherValue>
    path_iterator(path_iterator<OtherValue, Container> const& other,
                  typename boost::enable_if<boost::is_convertible<OtherValue*, Value*>, enabler>::type = enabler())
        : path_iterator::iterator_adaptor_(other.base())
    {}

  private:

    // grant access to iterator_adaptor to handle iteration properly.
    friend class boost::iterator_core_access;

    /*!
     * @brief Advance the iterator.
     */
    void increment()
    {
        // variables used to extract vertex components.
        geometry_type::coordinate_type x;
        geometry_type::coordinate_type y;

        // extract next vertex components.
        unsigned cmd = path_.vertex(&x, &y);

        if (cmd == SEG_END)
        {
            // if the end of the sequence is reached, set the reference
            // to the current element to null, so it matches the value
            // that marks the end of the sequence as defined in the
            // "end_iterator" constructor.
            this->base_reference() = 0;
        }
        else if (this->base_reference() == 0)
        {
            // the first element of the container is stored in the
            // member variable 'first_value_' and later assigned
            // to the reference that boost::iterator_adaptor stores
            // to track the current element.
            //
            // 'first_value_' is used as intermediate storage
            // because the compiler prohibits the assignment of the
            // address of a temporary object (&Value(...)).
            *first_value_ = Value(cmd, x, y);
            this->base_reference() = first_value_.get();
        }
        else
        {
            // point the reference to the current element to the next.
            *(this->base_reference()) = Value(cmd, x, y);
        }
    }

    /*!
     * @brief Test whether the current element is equal to 'other'.
     * @tparam OtherValue the value type of the other iterator (it may be const or non-const).
     * @param other iterator to compare to current element.
     */
    template<typename OtherValue>
    bool equal(path_iterator<OtherValue, Container> const& other) const
    {
        return this->base_reference() == other.base();
    }

    Container const& path_;
    std::shared_ptr<Value> first_value_;
};

// Specialization of geometry_iterator, as needed by mapnik::svg::svg_path_data_grammar.
// The Value type is a std::tuple that holds 5 elements, the command and the x and y coordinate.
// Each coordinate is stored twice to match the needs of the grammar.
// using path_iterator_type = path_iterator<std::tuple<unsigned, geometry_type::coordinate_type,
// geometry_type::value_type>,
//                      transform_path_adapter<view_transform, geometry_type> >;

} // namespace svg
} // namespace mapnik

#endif // SVG_PATH_ITERATOR_HPP
