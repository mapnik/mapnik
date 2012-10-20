#ifndef PYTHON_UTILS_HPP
#define PYTHON_UTILS_HPP

#include <boost/python.hpp>

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
