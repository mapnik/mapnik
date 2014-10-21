#ifndef PYTHON_UTILS_HPP
#define PYTHON_UTILS_HPP

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <boost/python.hpp>
#pragma GCC diagnostic pop

// Use RAII to acquire and release the GIL as needed.
class ensure_gil
{
    public:
        ensure_gil() : gil_state_(PyGILState_Ensure()) {}
        ~ensure_gil() { PyGILState_Release( gil_state_ ); }
    protected:
        PyGILState_STATE gil_state_;
};

std::string extractException();

#endif // PYTHON_UTILS_HPP
