#include "python_featureset.hpp"
#include "python_utils.hpp"

// boost
#include <boost/python.hpp>

python_featureset::python_featureset(boost::python::object iterator)
{
    ensure_gil lock;
    begin_ = boost::python::stl_input_iterator<mapnik::feature_ptr>(iterator);
}

python_featureset::~python_featureset()
{
    ensure_gil lock;
    begin_ = end_;
}

mapnik::feature_ptr python_featureset::next()
{
    // checking to see if we've reached the end does not require the GIL.
    if(begin_ == end_)
        return mapnik::feature_ptr();

    // getting the next feature might call into the interpreter and so the GIL must be held.
    ensure_gil lock;

    return *(begin_++);
}

