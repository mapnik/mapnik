#ifndef TEST_CATCH_EXT_HPP
#define TEST_CATCH_EXT_HPP

#include "catch.hpp"

#define TRY_CHECK(expr)                                                                                                \
    try                                                                                                                \
    {                                                                                                                  \
        CHECK(expr);                                                                                                   \
    }                                                                                                                  \
    catch (Catch::TestFailureException&)                                                                               \
    {                                                                                                                  \
        /* thrown by CHECK after it catches and reports */                                                             \
        /* an exception from expr => swallow this       */                                                             \
    }

#define TRY_CHECK_FALSE(expr)                                                                                          \
    try                                                                                                                \
    {                                                                                                                  \
        CHECK_FALSE(expr);                                                                                             \
    }                                                                                                                  \
    catch (Catch::TestFailureException&)                                                                               \
    {}

#endif // TEST_CATCH_EXT_HPP
